#pragma once

#include <string>
#include <memory>
#include "common/ErrorCode.h"

struct AVFormatContext;
struct AVCodec;
struct AVStream;
struct AVPacket;

namespace sophon_stream {
namespace multimedia {
namespace process {
namespace decode {
namespace gpu {

/**
 * 用ffmpeg进行解协议和解封装
 */
class FFmpegFormatInput {
  public:
    FFmpegFormatInput();
    ~FFmpegFormatInput();

    /**
     * 初始化ffmpeg
     * @param   url[IN]   rtsp流地址或本地文件路径
     *
     * @return  0为成功 其他见ivs.h中result_code定义
     */
    common::ErrorCode init(const std::string &url, const int timeout, std::string &strError);

    /**
     * 读取一帧
     * @return  0为成功 其他见ivs.h中result_code定义
     */
    common::ErrorCode readFrame(AVPacket **packet, std::string &strError, double &timeStamp, int *flag = 0);

    /**
     * 重新打开视频流 用于网络不稳定时的重连等情况
     * @return  0为成功 其他见ivs.h中result_code定义
     */
    common::ErrorCode reopen(const int timeout, std::string &strError);

    int getCodecId();

    /**
     * 销毁ffmpeg资源
     * @return  0为成功 其他见ivs.h中result_code定义
     */
    common::ErrorCode destroy();

    AVStream *getStream() {
        return mVideo;
    }
    int getStreamIndex() {
        return mStreamIndex;
    }

    int getWidth() {
        return mWidth;
    }

    int getHeight() {
        return mHeight;
    }

  private:
    std::string mRtspUrl;                           //rtsp流地址
    AVFormatContext *mInputCtx = nullptr;           //format context
    int mCodecId = 0;
    AVStream *mVideo = nullptr;                     //stream视频流
    AVCodec *mDecoder = nullptr;
    AVPacket *mPacket = nullptr;    //h264 packet
    int mWidth = -1;                                //视频原始宽度
    int mHeight = -1;                               //视频原始高度

    int mStreamIndex = -1;

    static int interrupt_cb(void *ctx);
};

}//namespace gpu
}//namespace decode
}//namespace process
}//namespace multimedia
}//namespace sophon_stream

