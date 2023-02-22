#pragma once

#include "FFmpegHwDevice-new.h"
#include "common/Frame.h"
#include "common/ErrorCode.h"

struct AVFormatContext;
struct AVStream;
struct AVCodecContext;
struct AVCodec;
struct AVDictionary;
struct AVFrame;
struct SwsContext;
struct AVPacket;

namespace sophon_stream {
namespace multimedia {
namespace process {
namespace decode {
namespace gpu {

struct DecoderParam {
    int resizeW; // 需要缩放的图像宽
    int resizeH; // 需要缩放的图像高
    unsigned int skipCount = 0; // frames count to skip
    FFmpegHwDevice *pHwDev;
    bool reopen = true;
    bool testMode = false;
    unsigned int interval = 40000;
    int openTimeout = 1000000;//us
    int reopenCount = -1;//-1 is infinite
};

/**
 * 用ffmpeg进行解码(包括软解和硬解)
 */
class FFmpegDecoder {
  public:
    /**
     * 默认构造函数
     */
    FFmpegDecoder();

    /**
     * 析构函数
     */
    ~FFmpegDecoder();

    /**
     * 初始化解码资源
     * @param dp[IN]  Docoder Configuration Parameter
     *
     * @return  0为成功 其他见ivs.h中result_code定义
     */
    common::ErrorCode init(const DecoderParam &dp, const int w, const int h, const std::string& side, const int sideNo, int codecId, std::string &strError);

    /**
     * decodePacket: decode a packet
     * @param:
     *   got_picture[OUT]: the count of images which are decoded and returned.
     * @return: SUCCESS - the packet is decoded successfully.
     *          Otherwise errors happens with the decoder.
     */
    common::ErrorCode decodePacket(const int streamIndex, AVPacket *pkt, int &got_picture, std::string &strError, double *timeStamp = nullptr);

    /**
     * 获取一帧原始图像，在解码函数之后调用
     */
    std::shared_ptr<common::Frame> getOriginImage();

    /**
     * 获取一帧缩放图像，在解码函数之后调用
     */
    //std::shared_ptr<Image> getResizeImage();

    /**
     * bindCudaCtx函数
     * 将cuda资源和本线程绑定
     */
    void bindCudaCtx();

    /**
     * 销毁解码资源
     * @return  0为成功 其他见ivs.h中result_code定义
     */
    common::ErrorCode destroy();

    void setStream(AVStream *stream) {
        mVideo = stream;
    }

  private:
    int mVideoStream = -1;                              // input流的index
    //AVFormatContext *mInputCtx = nullptr;               // input流的上下文句柄
    AVStream *mVideo = nullptr;                         // 视频流
    AVCodecContext *mDecoderCtx = nullptr;              // 解码器上下文句柄
    AVCodec *mDecoder = nullptr;                        // ffmpeg解码器
    AVFrame *mSrcFrame = nullptr;                       // 解码后的nv12图像
    AVFrame *mDstFrame = nullptr;                       // CPU颜色空间转换后的bgr图像
    AVFrame *mDstFrameOrigin = nullptr;                 // CPU颜色空间转换后的bgr图像原图
    SwsContext *mSwsCtx = nullptr;                      // ffmpeg颜色空间转换上下文句柄(仅cpu用)
    SwsContext *mSwsCtxOrigin = nullptr;                      // ffmpeg颜色空间转换上下文句柄(仅cpu用)
    //std::shared_ptr<Image> mImageOrigin = nullptr;            // 最终原始图像指针，如果设备设置为cpu就指向cpu内存，设置为gpu就指向gpu内存
    //std::shared_ptr<Image> mImageResize = nullptr;      // 最终缩放图像指针，如果设备设置为cpu就指向cpu内存，设置为gpu就指向gpu内存
    std::shared_ptr<common::Frame> mImageOrigin = nullptr;            // 最终原始图像指针，如果设备设置为cpu就指向cpu内存，设置为gpu就指向gpu内存
    std::shared_ptr<common::Frame> mImageResize = nullptr;      // 最终缩放图像指针，如果设备设置为cpu就指向cpu内存，设置为gpu就指向gpu内存
    unsigned char *mResizeData = nullptr;               // 仅在gpu缩放情况下使用,存放gpu下yuv图像
    int mResizeTotal = -1;                              // mResizeData整体大小
    int mResizeY = -1;                                  // nv12 y分量大小
    int mResizeUV = -1;                                 // nv12 uv分量大小

    int mWidth = 1920;
    int mHeight = 1080;
    int mResizeW = -1;
    int mResizeH = -1;

    std::string mSide = "";
#ifdef GPU_VERSION
    CUcontext mCUcontext = nullptr;
#endif
  private:
    common::ErrorCode makeImage();
};

}//namespace gpu
}//namespace decode
}//namespace process
}//namespace multimedia
}//namespace sophon_stream
