#include "FFmpegFormatInput-new.h"
#include "FFmpegHwDevice-new.h"
//#include "common/ivs.h"
#include "common/Logger.h"

extern "C" {
#include <libavformat/avformat.h>
}

namespace lynxi {
namespace ivs {
namespace multimedia {
namespace process {
namespace decode {
namespace gpu {

FFmpegFormatInput::FFmpegFormatInput() {

}

FFmpegFormatInput::~FFmpegFormatInput() {
    destroy();
}


int FFmpegFormatInput::interrupt_cb(void *ctx) {
    FFmpegFormatInput *pObj = static_cast<FFmpegFormatInput *>(ctx);
    IVS_DEBUG("interrupt_cb");
    return 0;
}

common::ErrorCode FFmpegFormatInput::init(const std::string &url, const int timeout, std::string &strError) {
    mRtspUrl = url;

    int ret = (int)common::ErrorCode::SUCCESS;
    AVDictionary *dict = nullptr;
    //默认为udp传输，会出现花屏现象，因此设置为tcp传输，解决花屏问题。
    av_dict_set(&dict, "rtsp_transport", "tcp", 0);
    av_dict_set(&dict, "stimeout", std::to_string(timeout).c_str(), 0);

    mInputCtx = avformat_alloc_context();
    if (nullptr == mInputCtx) {
        strError = "input context alloced failed.";
        return common::ErrorCode::ERR_FFMPEG_INPUT_CTX_ALLOC;
    }
    mInputCtx->flags |= AVFMT_FLAG_NONBLOCK;
//    mInputCtx->interrupt_callback.opaque = this;
//    mInputCtx->interrupt_callback.callback = interrupt_cb;
    //根据src地址得到input上下文
    ret = avformat_open_input(&mInputCtx, mRtspUrl.c_str(), nullptr, &dict);
    if (ret != 0) {
        strError = "Cannot open input file:" + mRtspUrl + ", errorInfo: " + getFFmpegErro(ret);
        return common::ErrorCode::ERR_FFMPEG_INPUT_CTX_OPEN;
    }

    if (dict != nullptr) {
        av_dict_free(&dict);
    }

    //检测input上下文是否存在流
    ret = avformat_find_stream_info(mInputCtx, nullptr);
    if (ret < 0) {
        strError = "Cannot find input stream information, errorInfo: " + getFFmpegErro(ret);
        return common::ErrorCode::ERR_FFMPEG_FIND_STREAM;
    }

    if (mInputCtx->duration != AV_NOPTS_VALUE) {
        int hours, mins, secs, us;
        int64_t duration = mInputCtx->duration + 5000;
        secs = duration / AV_TIME_BASE;
        us = duration % AV_TIME_BASE;
        mins = secs / 60;
        secs %= 60;
        hours = mins / 60;
        mins %= 60;

        IVS_DEBUG("totalSeconds:{0}:{1}:{2}.{3}\n", hours, mins, secs, (100 * us) / AV_TIME_BASE);

    }

    //从input上下文获取视频流,并得到视频流的index
    mStreamIndex = av_find_best_stream(mInputCtx, AVMEDIA_TYPE_VIDEO, -1, -1, &mDecoder, 0);
    if (mStreamIndex < 0) {
        strError = "Cannot find a video stream in the input file.";
        return common::ErrorCode::ERR_FFMPEG_FIND_VIDEO_STREAM;
    }
    mPacket = av_packet_alloc();
    if (mPacket == nullptr) {
        strError = "Cannot find a video stream in the input file.";
        return common::ErrorCode::ERR_FFMPEG_PACKET_ALLOC;
    }

    mCodecId = mDecoder->id;

    mVideo = mInputCtx->streams[mStreamIndex];

//    int frame_rate=mVideo->avg_frame_rate.num/mVideo->avg_frame_rate.den;

//    IVS_DEBUG("frame_rate:{0}", frame_rate);

    mWidth = mVideo->codecpar->width;
    mHeight = mVideo->codecpar->height;
    if(mWidth==0||mHeight==0) 
        return common::ErrorCode::ERR_FFMPEG_CODEC_LENGHT; 

    return (common::ErrorCode)ret;
}

common::ErrorCode FFmpegFormatInput::readFrame(AVPacket **packet, std::string &strError, double &timeStamp, int *flag) {
    int ret = (int)common::ErrorCode::SUCCESS;
    if(mPacket==nullptr) return common::ErrorCode::SUCCESS;
    av_packet_unref(mPacket);
    //拿到avpacket
    if ((ret = av_read_frame(mInputCtx, mPacket)) < 0) {
        if (ret == AVERROR_EOF) {
            *packet = mPacket;
            IVS_DEBUG("end of stream ,stream index:{0}", mPacket->stream_index);
            return common::ErrorCode::STREAM_END;
        }

        else {
            try {
                strError = "av_read_frame failed, ret:" + std::to_string(ret) + ",info:{1}" + getFFmpegErro(ret);

            } catch (std::exception &e) {
                IVS_ERROR("{0}", e.what());
            }
            *packet = nullptr;
            return common::ErrorCode::ERR_FFMPEG_READ_FRAME;
        }
    }
    if(flag!=nullptr){
        *flag = mPacket->flags;
    }
    if (mPacket->pts == AV_NOPTS_VALUE) {
        IVS_WARN("pts invalid--url:{0}-pts:{1}-dts:{2}", mRtspUrl, mPacket->pts, mPacket->dts);
    }
    timeStamp = mPacket->pts * av_q2d(mVideo->time_base);

    *packet = mPacket;
    return (common::ErrorCode)ret;
}

common::ErrorCode FFmpegFormatInput::reopen(const int timeout, std::string &strError) {
    destroy();
    return init(mRtspUrl, timeout, strError);
}

int FFmpegFormatInput::getCodecId() {
    return mCodecId;
}

common::ErrorCode FFmpegFormatInput::destroy() {
    if (nullptr != mPacket) {
        av_packet_free(&mPacket);
        mPacket = nullptr;
    }
    if (nullptr != mInputCtx) {
        avformat_close_input(&mInputCtx);
        mInputCtx = nullptr;
    }
    return common::ErrorCode::SUCCESS;
}

}//namespace gpu
}//namespace decode
}//namespace process
}//namespace multimedia
}//namespace ivs
}//namespace lynxi
