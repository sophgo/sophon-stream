#pragma once

#include <string>
#include <vector>
#include <memory>
#include "common/ErrorCode.h"
#include "common/Graphics.hpp"
namespace sophon_stream {
namespace multimedia {


/**
 * context基类
 */
struct Context {
    std::string multimediaName;
    int deviceId;
    std::shared_ptr<void> data = nullptr;
    
    std::string mUrl;
    common::Rectangle<int> mRoi;
    float mResizeRate = 1.0f;
    int mTimeout = 0;
    int mSourceType = 0;//0视频文件1是文件夹2是rtsp或rtmp
    //std::vector<float> result;

    /**
     * context初始化
     * @param[in] json: 初始化的json字符串
     * @return 错误码
     */
    virtual common::ErrorCode init(const std::string&)=0;
};

} // namespace multimedia 
} // namespace sophon_stream

