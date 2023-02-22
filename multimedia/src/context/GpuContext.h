/**********************************************

 Copyright (c) 2018 LynxiTech Inc - All rights reserved.

 NOTICE: All information contained here is, and remains
 the property of LynxiTech Incorporation. This file can not
 be copied or distributed without permission of LynxiTech Inc.

 Author: written by Yang Jun <jun.yang@lynxi.com>, 20-3-6

**************************************************/

#ifndef LYNXI_IVS_ALGORITHM_CONTEXT_GPU_H_
#define LYNXI_IVS_ALGORITHM_CONTEXT_GPU_H_

#include "../Context.h"
namespace lynxi {
namespace ivs {
namespace multimedia {
namespace context {


struct GpuContext :public multimedia::Context {

    static constexpr const char* JSON_URL = "url";
    static constexpr const char* JSON_RESIZE_RATE = "resize_rate";
    static constexpr const char* JSON_TIMEOUT = "timeout";
    static constexpr const char* JSON_CHANNELID = "channel_id";
    static constexpr const char* JSON_SOURCE_TYPE = "source_type";
    static constexpr const char* JSON_ROI = "roi";
    static constexpr const char* JSON_X = "x";
    static constexpr const char* JSON_Y = "y";
    static constexpr const char* JSON_W = "w";
    static constexpr const char* JSON_H = "h";

    /**
     * context初始化
     * @param[in] json: 初始化的json字符串
     * @return 错误码
     */
    common::ErrorCode init(const std::string& json) override;
};
} // namespace context
} // namespace multimedia
} // namespace ivs
} // namespace lynxi

#endif // LYNXI_IVS_ALGORITHM_CONTEXT_GPU_H_

