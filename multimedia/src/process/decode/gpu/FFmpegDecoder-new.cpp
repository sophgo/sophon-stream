#include "FFmpegDecoder-new.h"
#include "common/Logger.h"
#include "config.h"
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/pixdesc.h>
#include <libavutil/opt.h>
#ifdef GPU_VERSION
#include <libavutil/hwcontext.h>
#include <libavutil/hwcontext_cuda.h>
#endif
#include <libavutil/avassert.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}

#define CUVID_NAME_LEN 40
//Gpu颜色空间转换
extern int cvtColor(unsigned char *d_req, unsigned char *d_res, int resolution,
                    int height, int width,
                    int linesize);
extern void ResizeNv12(unsigned char *dpDstNv12, int nDstPitch, int nDstWidth,
                       int nDstHeight, unsigned char *dpSrcNv12, int nSrcPitch,
                       int nSrcWidth, int nSrcHeight, unsigned char *dpDstNv12UV);



//FIXME: This is not multi-GPU safe.

/**Solution:

// the max number of GPU devices supported by single ivs-engine
#define MAX_GPU_DEVICES 8

static AVBufferRef *gHwDeviceCtx[MAX_GPU_DEVICES];
static enum AVPixelFormat gHwPixFmt[MAX_GPU_DEVICES];
**/

static enum AVPixelFormat gHwPixFmt;


//回调函数 用于获取硬件format 设置回调必须在hwDecoderInit之前
static enum AVPixelFormat getHwFormat(AVCodecContext *ctx,
                                      const enum AVPixelFormat *pix_fmts) {
    const enum AVPixelFormat *p;

    for (p = pix_fmts; *p != -1; p++) {
        if (*p == gHwPixFmt)
            return *p;
    }
    IVS_ERROR("Failed to get HW surface format.");
    return AV_PIX_FMT_NONE;
}

namespace lynxi {
namespace ivs {
namespace multimedia {
namespace process {
namespace decode {
namespace gpu {

FFmpegDecoder::FFmpegDecoder() {

}

FFmpegDecoder::~FFmpegDecoder() {
    destroy();
}

//初始化函数
common::ErrorCode FFmpegDecoder::init(const DecoderParam &dp, const int w, const int h,  const std::string& side, const int sideNo, int codecId, std::string &strError) {
    mWidth = w;
    mHeight = h;
    mResizeW = dp.resizeW;
    mResizeH = dp.resizeH;
    mSide = side;

    int ret = (int)common::ErrorCode::SUCCESS;

    /*********************decode*********************/

#ifdef GPU_VERSION

    if (AV_CODEC_ID_HEVC == codecId)
        mDecoder = avcodec_find_decoder_by_name("hevc_cuvid");
    else if (AV_CODEC_ID_H264 == codecId)
        mDecoder = avcodec_find_decoder_by_name("h264_cuvid");
    else mDecoder = avcodec_find_decoder(AVCodecID(codecId));
#else
    mDecoder = avcodec_find_decoder(AVCodecID(codecId));
#endif

    if (nullptr == mDecoder) {
        strError = "Failed to find hw decoder ";
        return common::ErrorCode::ERR_FFMPEG_NONE_HW_DEC;
    }

    //根据decoder获取decoder上下文句柄
    if (!(mDecoderCtx = avcodec_alloc_context3(mDecoder))) {
        strError = "Could not allocate video decoder codec context";
        return common::ErrorCode::ERR_FFMPEG_AVCODEC_CTX_ALLOC;
    }

    if (avcodec_parameters_to_context(mDecoderCtx, mVideo->codecpar) < 0) {
        strError = "Failed to copy parameters";
        return common::ErrorCode::ERR_FFMPEG_PARAM_COPY;
    }

    AVDictionary *dictDec = nullptr;
#ifdef GPU_VERSION //此处必须在CodecContext申请成功后执行
    if (nullptr == dp.pHwDev) {
        strError = "HwDevice is not inited.";
        return common::ErrorCode::ERR_FFMPEG_CREATE_HW_DEV;
    }
    enum AVHWDeviceType type = AVHWDeviceType(dp.pHwDev->getHWDeviceType());
    AVBufferRef *hwDevCtx = dp.pHwDev->getHwDeviceCtx();
    if(hwDevCtx==nullptr){
        strError = "HwDevice create failed!";
        return common::ErrorCode::ERR_FFMPEG_CREATE_HW_DEV;
    }
    for (int i = 0;; i++) {
        const AVCodecHWConfig *config = avcodec_get_hw_config(mDecoder, i);
        if (!config) {
            try {
                strError = "Decoder " + std::string(mDecoder->name) + " does not support device type " +
                           std::string(av_hwdevice_get_type_name(type));
            } catch (std::exception &e) {
                IVS_ERROR("{0}", e.what());
            }

            return common::ErrorCode::ERR_FFMPEG_DEC_NOTSUP_HWDEV;
        }
        if (config->methods & AV_CODEC_HW_CONFIG_METHOD_HW_DEVICE_CTX &&
                config->device_type == type) {
            gHwPixFmt = config->pix_fmt;
            break;
        }
    }

    //设置getHwFormat回调
    mDecoderCtx->get_format  = getHwFormat;
    mDecoderCtx->hw_device_ctx = av_buffer_ref(hwDevCtx);

    //此处从硬件句柄中获取cuda句柄，并绑定到当前线程(必做)
    AVHWDeviceContext *device_ctx = reinterpret_cast<AVHWDeviceContext *>(hwDevCtx->data);
    AVCUDADeviceContext *device_hwctx = reinterpret_cast<AVCUDADeviceContext *>(device_ctx->hwctx);
    mCUcontext = device_hwctx->cuda_ctx;
    cuCtxPushCurrent(mCUcontext);

    if ((mWidth != mResizeW) && (mHeight != mResizeH)) { //如果需要缩放则申请缩放的nv12空间
        mResizeY = mResizeW * mResizeH;
        mResizeUV = mResizeW * mResizeH / 2;
        mResizeTotal = mResizeY + mResizeUV;
        int retCuda = cuMemAlloc(reinterpret_cast<CUdeviceptr *>(&mResizeData), static_cast<size_t>(mResizeTotal));
        if (retCuda != CUDA_SUCCESS) {
            strError = "cuMemAlloc failed,return value : " + std::to_string(retCuda);
            return common::ErrorCode::ERR_FFMPEG_ALLOC_FRAME_OUTOF_MEM;
        }
    }

    //设置gpu index
    try {
        av_dict_set(&dictDec, "gpu", std::to_string(sideNo).c_str(), 0);
    } catch (std::exception &e) {
        IVS_ERROR("{0}", e.what());
    }
#endif
    //解码得到的视频帧空间的申请，对于硬解，颜色空间转换在GPU上做，结果直接放到mImage里
    if (!(mSrcFrame = av_frame_alloc())) {
        strError = "Can not alloc source frame.";
        return common::ErrorCode::ERR_FFMPEG_ALLOC_FRAME_OUTOF_MEM;
    }

#ifndef GPU_VERSION //此处是cpu的颜色空间转换的一些准备工作
    //颜色空间转换后视频帧空间的申请
    if (!(mDstFrame = av_frame_alloc())) {
        strError = "Can not alloc dest frame.";
        return common::ErrorCode::ERR_FFMPEG_ALLOC_FRAME_OUTOF_MEM;
    }
    ret = av_image_alloc(mDstFrame->data, mDstFrame->linesize, mResizeW, mResizeH, AV_PIX_FMT_BGR24, 1);
    if (ret < 0) {
        strError = "alloc image data for dst frame fail, errorInfo: " + getFFmpegErro(ret);
        return common::ErrorCode::ERR_FFMPEG_ALLOC_FRAME_OUTOF_MEM;
    }

    if (!(mDstFrameOrigin = av_frame_alloc())) {
        strError = "Can not alloc dest frame.";
        return common::ErrorCode::ERR_FFMPEG_ALLOC_FRAME_OUTOF_MEM;
    }
    ret = av_image_alloc(mDstFrameOrigin->data, mDstFrameOrigin->linesize, mWidth, mHeight, AV_PIX_FMT_BGR24, 1);
    if (ret < 0) {
        strError = "alloc image data for dst frame fail, errorInfo: " + getFFmpegErro(ret);
        return common::ErrorCode::ERR_FFMPEG_ALLOC_FRAME_OUTOF_MEM;
    }

    // 初始化swscontext，在这个模块中需要进行两个转换，一个是对颜色空间进行转换，yuv420->bgr24。另一个是缩放到640*360。
    mSwsCtx = sws_getContext(mWidth, mHeight, AV_PIX_FMT_YUV420P,
                             mResizeW, mResizeH, AV_PIX_FMT_BGR24,
                             SWS_BICUBIC, nullptr, nullptr, nullptr);
    if (nullptr == mSwsCtx) {
        strError = "get sws context fail";
        return common::ErrorCode::ERR_FFMPEG_SWS_CONTEXT_GET ;
    }

    mSwsCtxOrigin = sws_getContext(mWidth, mHeight, AV_PIX_FMT_YUV420P,
                                   mWidth, mHeight, AV_PIX_FMT_BGR24,
                                   SWS_BICUBIC, nullptr, nullptr, nullptr);
    if (nullptr == mSwsCtxOrigin) {
        strError = "get sws context fail";
        return common::ErrorCode::ERR_FFMPEG_SWS_CONTEXT_GET ;
    }
#endif

    //给视频结果申请空间
    //mImage = std::make_shared<Image>(resizeH, resizeW, 3, mCodecDevice);

    //Initialize the AVCodecContext to use the given AVCodec
    if ((ret = avcodec_open2(mDecoderCtx, mDecoder, &dictDec)) < 0) {
        try {
            strError = "Decoder failed to open codec for stream stream index:" + std::to_string(mVideoStream) +
                       " , avcodec_open2 return: " + getFFmpegErro(ret);
        } catch (std::exception &e) {
            IVS_ERROR("{0}", e.what());
        }

        return common::ErrorCode::ERR_FFMPEG_OPEN_CODEC;
    }

    return common::ErrorCode::SUCCESS;
}

std::shared_ptr<common::Frame> FFmpegDecoder::getOriginImage() {
    return mImageOrigin;
}

//std::shared_ptr<Image> FFmpegDecoder::getResizeImage() {
//    return mImageResize;
//}

common::ErrorCode FFmpegDecoder::destroy() {
    if (nullptr != mResizeData) {
#ifdef GPU_VERSION
        cuMemFree(reinterpret_cast<CUdeviceptr>(mResizeData));
        mResizeData = nullptr;
#endif
    }
    if (nullptr != mSrcFrame) {
        av_frame_free(&mSrcFrame);
        mSrcFrame = nullptr;
    }
    if (nullptr != mDstFrame) {
        if (nullptr != mDstFrame->data[0])
            av_freep(&mDstFrame->data[0]);
        av_frame_free(&mDstFrame);
        mDstFrame = nullptr;
    }
    if (nullptr != mDstFrameOrigin) {
        if (nullptr != mDstFrameOrigin->data[0])
            av_freep(&mDstFrameOrigin->data[0]);
        av_frame_free(&mDstFrameOrigin);
        mDstFrameOrigin = nullptr;
    }

    if (nullptr != mSwsCtx) {
        sws_freeContext(mSwsCtx);
        mSwsCtx = nullptr;
    }
    if (nullptr != mSwsCtxOrigin) {
        sws_freeContext(mSwsCtxOrigin);
        mSwsCtxOrigin = nullptr;
    }
    if (nullptr != mDecoderCtx) {
        avcodec_free_context(&mDecoderCtx);
        mDecoderCtx = nullptr;
        IVS_INFO("FFmpegDecoder mDecoderCtx destroyed!");
    }
    return common::ErrorCode::SUCCESS;
}

void doCopy(void *src,void * dst,int w,int h,int c) {
    int size = w * h * c;

#ifdef GPU_VERSION
    CUDA_MEMCPY2D m = { 0 };
    m.WidthInBytes = w;
    m.Height = h * c;
    m.srcMemoryType = CU_MEMORYTYPE_DEVICE;
    m.srcDevice = CUdeviceptr(src);
    m.srcPitch = m.WidthInBytes;
    m.dstMemoryType = CU_MEMORYTYPE_DEVICE;
    m.dstDevice = CUdeviceptr(dst);
    m.dstPitch = m.WidthInBytes;
    cuMemcpy2D(&m);
#else
    memcpy(dst, src, size);
#endif

}

common::ErrorCode FFmpegDecoder::makeImage(){
        std::shared_ptr<unsigned char> imageOrigin = nullptr;
        std::shared_ptr<unsigned char> imageResize = nullptr;
#ifdef GPU_VERSION //gpu下用cuda做缩放和颜色空间转换
        unsigned char* gpuData = nullptr;
        CUresult cuRet = cuMemAlloc(reinterpret_cast<CUdeviceptr *>(&gpuData), static_cast<size_t>(mHeight*mWidth*3*sizeof(unsigned char)));
        if (cuRet != CUDA_SUCCESS) {
            IVS_ERROR("cuMemAlloc failed,return value : {0}, width:{1}, height:{2}", cuRet, mWidth, mHeight);
            return common::ErrorCode::ERR_CUDA_MEMORY_ALLOCATION;
        }
        imageOrigin.reset(gpuData,[](void* p) {
            cuMemFree(reinterpret_cast<CUdeviceptr>(p));
            p = nullptr;
        });
        if (imageOrigin != nullptr) {
            if ((mWidth != mResizeW) && (mHeight != mResizeH)) {
                unsigned char* gpuDataResize = nullptr;
                cuRet = cuMemAlloc(reinterpret_cast<CUdeviceptr *>(&gpuDataResize),static_cast<size_t>(mResizeH*mResizeW*3*sizeof(unsigned char)));
                if (cuRet != CUDA_SUCCESS) {
                    IVS_ERROR("cuMemAlloc failed,return value : {0}", cuRet);
                    return common::ErrorCode::ERR_CUDA_MEMORY_ALLOCATION;
                }
                imageResize.reset(gpuDataResize,[](void* p) {
                    cuMemFree(reinterpret_cast<CUdeviceptr>(p));
                    p = nullptr;
                });

                if (imageResize != nullptr) {
                    ResizeNv12(mResizeData, mResizeW, mResizeW, mResizeH, mSrcFrame->data[0]
                               , mSrcFrame->linesize[0], mSrcFrame->width, mSrcFrame->height,
                               mResizeData + mResizeY);
                    cvtColor(mResizeData, static_cast<uint8_t *>(imageResize.get()),
                             mResizeW * mResizeH, mResizeH, mResizeW, mResizeW);
                    cvtColor(mSrcFrame->data[0], static_cast<uint8_t *>(imageOrigin.get()),
                             mSrcFrame->height * mSrcFrame->width, mSrcFrame->height,
                             mSrcFrame->width, mSrcFrame->linesize[0]);
                }

            } else
                cvtColor(mSrcFrame->data[0], static_cast<uint8_t *>(imageOrigin.get()),
                         mSrcFrame->height * mSrcFrame->width, mSrcFrame->height,
                         mSrcFrame->width, mSrcFrame->linesize[0]);
        }


#else //cpu下ffmpeg颜色空间转换中自带缩放功能
        imageResize.reset(new unsigned char[mResizeH*mResizeW*3*sizeof (unsigned char)],[](unsigned char* p) {
            delete[] p;
            p=nullptr;
        });
        ret = sws_scale(mSwsCtx, mSrcFrame->data, mSrcFrame->linesize, 0,
                        mHeight, mDstFrame->data, mDstFrame->linesize);

        doCopy(mDstFrame->data[0],imageResize.get(),mResizeW,mResizeH,3);

        imageOrigin.reset(new unsigned char[mWidth*mHeight*3*sizeof (unsigned char)],[](unsigned char* p) {
            delete[] p;
            p=nullptr;
        });

        ret = sws_scale(mSwsCtxOrigin, mSrcFrame->data, mSrcFrame->linesize, 0,
                        mHeight, mDstFrameOrigin->data, mDstFrameOrigin->linesize);
        doCopy(mDstFrameOrigin->data[0],imageOrigin.get(),mWidth,mHeight,3);
#endif
        std::shared_ptr<common::Frame> origin = std::make_shared<common::Frame>();
        origin->mChannel = 3;
        origin->mWidth = mWidth;
        origin->mHeight = mHeight;

        origin->mFormatType = common::FormatType::BGR_PACKET;
        origin->mDataType = common::DataType::INTEGER;

        origin->mData = std::static_pointer_cast<void>(imageOrigin);
        origin->mDataSize = origin->mHeight*origin->mWidth*origin->mChannel;

        origin->mSide = mSide;
        origin->mWidthStep = sizeof(char)*origin->mChannel;
        origin->mHeightStep = origin->mWidthStep*origin->mWidth;
        origin->mChannelStep = origin->mHeightStep*origin->mHeight;
        mImageOrigin = origin;
        return common::ErrorCode::SUCCESS;
        //mImageResize = imageResize;
}

/**
 * decodePacket: decode a packet
 * @param:
 *   got_picture[OUT]: the count of images which are decoded and returned.
 * @return: SUCCESS - the packet is decoded successfully.
 *          Otherwise errors happens with the decoder.
 */
common::ErrorCode FFmpegDecoder::decodePacket(const int streamIndex, AVPacket *pkt, int &got_picture, std::string &strError, double *timeStamp) {

    if (pkt != nullptr && streamIndex != pkt->stream_index) return common::ErrorCode::NOT_VIDEO_CHANNEL;


//    av_packet_rescale_ts(pkt,
//                         mVideo->time_base,
//                         mDecoderCtx->time_base);
//    av_rescale_q(1,mVideo->time_base,mDecoderCtx->time_base);

    int ret = avcodec_send_packet(mDecoderCtx, pkt);
    if (ret < 0) {
        try {
            strError = "Error during decoding, avcodec_send_packet ret:" + std::to_string(ret) + ",info:" + getFFmpegErro(ret);

        } catch (std::exception &e) {
            IVS_ERROR("{0}", e.what());
        }
        return common::ErrorCode::ERR_FFMPEG_DURING_DECODING;
    }

    got_picture = 0;
    while (true) {
        av_frame_unref(mSrcFrame);

        ret = avcodec_receive_frame(mDecoderCtx, mSrcFrame);
        if (ret == AVERROR(EAGAIN)) {//try again表示成功且未结束
            return common::ErrorCode::SUCCESS;
        } else if (ret == AVERROR_EOF) { //EOF表示末尾
//            if(mImageOrigin==nullptr){
//                makeImage();
//            }
            return common::ErrorCode::STREAM_END;
        } else if (ret < 0) {
            try {
                strError = "Error while decoding, avcodec_receive_frame ret:" + std::to_string(ret) + ",info:" + getFFmpegErro(ret);

            } catch (std::exception &e) {
                IVS_ERROR("{0}", e.what());
            }
            return common::ErrorCode::ERR_FFMPEG_WHILE_DECODING;
        }

        *timeStamp = mSrcFrame->pts * av_q2d(mVideo->time_base);

//        static double lastpKtStamp = 0;

//        double pktStamp = pkt->pts*av_q2d(mVideo->time_base);
//        double cha = pktStamp-lastpKtStamp;

//        IVS_TRACE("pkt->pts:{0}--pkt->dts:{1}-----frame->pts:{2}",
//                  pkt->pts,pkt->dts,mSrcFrame->pts);

//        IVS_TRACE("avpacket->pts:{0}--lastpKtStamp:{1}--cha{2}",pktStamp,lastpKtStamp,cha);

//        lastpKtStamp = pktStamp;

//        IVS_TRACE("pkt->dts:{0}",pkt->dts);

//        *timeStamp = mSrcFrame->pts * av_q2d(mDecoderCtx->time_base);

//        IVS_TRACE("mSrcFrame->pts:{0}------mVideo->time_base:den{1}-num{2}----timeStamp:{3}----duration",
//                  mSrcFrame->pts,mDecoderCtx->time_base.den,
//                  mDecoderCtx->time_base.num,*timeStamp);

        common::ErrorCode errorCode = makeImage();
        if(errorCode!=common::ErrorCode::SUCCESS){
            return errorCode;
        }
        got_picture++;
    }

    return common::ErrorCode::SUCCESS;
}

void FFmpegDecoder::bindCudaCtx() {
#ifdef GPU_VERSION
    if (mCUcontext != nullptr)
        cuCtxPushCurrent(mCUcontext);
#endif
}

}//namespace gpu
}//namespace decode
}//namespace process
}//namespace multimedia
}//namespace ivs
}//namespace lynxi
