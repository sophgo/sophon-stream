#pragma once

#include <string>
#include "common/ErrorCode.h"
#include "config.h"
#ifdef GPU_VERSION
#include <cuda.h>
#endif

struct AVBufferRef;

namespace sophon_stream {
namespace multimedia {
namespace process {
namespace decode {
namespace gpu {

std::string getFFmpegErro(const int errorCode);
/**
 * FFmpegHwDevice类用于初始化hw device
 * 同时解决多卡问题
 */

#ifndef GPU_VERSION
typedef void *CUcontext;

#endif

class FFmpegHwDevice {
  public:
    /**
     * 构造函数
     * @param devID 目前表示gpu设备ID
     */
    FFmpegHwDevice() {}

    /**
     * 析构函数
     */
    ~FFmpegHwDevice() {
        destroy();
    }

    /**
     * init函数 初始化hw device
     */
    common::ErrorCode init(const int devID);

    /**
     * destroy函数 初始化句柄
     */
    common::ErrorCode destroy();

    /**
     * getCudaContext函数 获取cuda句柄
     */
    CUcontext getCudaContext();
    /**
     * getHwDeviceCtx函数 获取hw device上下文句柄
     */
    inline AVBufferRef *getHwDeviceCtx() const {
        return mHwDeviceCtx;
    }

    /**
     * getHWDeviceType函数 获取hw device类型
     */
    inline int getHWDeviceType() const {
        return mHWDeviceType;
    }
  private:
    int mDevID = 0;
    int mHWDeviceType = 0;
    AVBufferRef *mHwDeviceCtx = nullptr;
#ifdef GPU_VERSION
    CUcontext mCudaContext = nullptr;
#endif
};

}//namespace gpu
}//namespace decode
}//namespace process
}//namespace multimedia
}//namespace sophon_stream

