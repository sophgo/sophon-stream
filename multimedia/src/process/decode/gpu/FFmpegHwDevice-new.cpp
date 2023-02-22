#include "FFmpegHwDevice-new.h"
#include "common/Logger.h"
extern "C" {
#include <libavutil/error.h>
#ifdef GPU_VERSION
#include <libavutil/hwcontext.h>
#include <libavutil/hwcontext_cuda.h>
#endif
}

namespace lynxi {
namespace ivs {
namespace multimedia {
namespace process {
namespace decode {
namespace gpu {

common::ErrorCode FFmpegHwDevice::init(const int devID) {
    mDevID = devID;
#ifdef GPU_VERSION
    //hwdevice查找对应的cuda设备(ffmpeg编译以前的config中如果没有配置，此处会返回NONE)
    enum AVHWDeviceType type;
    type = av_hwdevice_find_type_by_name("cuda");
    if (type == AV_HWDEVICE_TYPE_NONE) {
        IVS_ERROR("Device type cuda is not supported.");
        while ((type = av_hwdevice_iterate_types(type)) != AV_HWDEVICE_TYPE_NONE)
            IVS_ERROR("Available device type: {0}.", av_hwdevice_get_type_name(type));
        return common::ErrorCode::ERR_FFMPEG_NONE_HWDEVICE;
    }
    mHWDeviceType = type;
    int ret = 0;
    if ((ret = av_hwdevice_ctx_create(&mHwDeviceCtx, type,
                                      std::to_string(mDevID).c_str(), nullptr, 0)) < 0) {
        IVS_ERROR("Failed to create specified HW device,error info: {0}", getFFmpegErro(ret));
        return common::ErrorCode::ERR_FFMPEG_CREATE_HW_DEV;
    }
    IVS_INFO("ffmpeg hardware init succeed!");
#endif
    return common::ErrorCode::SUCCESS;
}

common::ErrorCode FFmpegHwDevice::destroy() {
#ifdef GPU_VERSION
    if (nullptr != mHwDeviceCtx) {
        av_buffer_unref(&mHwDeviceCtx);
        mHwDeviceCtx = nullptr;
        IVS_INFO("ffmpeg hardware destroyed!");
    }
#endif
    return common::ErrorCode::SUCCESS;
}

CUcontext FFmpegHwDevice::getCudaContext() {
#ifdef GPU_VERSION
    if(mHwDeviceCtx==nullptr) return nullptr;
    AVHWDeviceContext *device_ctx = reinterpret_cast<AVHWDeviceContext *>(mHwDeviceCtx->data);
    if(device_ctx==nullptr) return nullptr;
    AVCUDADeviceContext *device_hwctx = reinterpret_cast<AVCUDADeviceContext *>(device_ctx->hwctx);
    mCudaContext = device_hwctx->cuda_ctx;
    return mCudaContext;
#endif
}

std::string getFFmpegErro(const int errorCode) {
    char buf[1024] = {0};
    av_strerror(errorCode, buf, 1024);
    std::string ret(buf);
    return ret;
}

}//namespace gpu
}//namespace decode
}//namespace process
}//namespace multimedia
}//namespace ivs
}//namespace lynxi
