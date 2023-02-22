/**********************************************

 Copyright (c) 2018 LynxiTech Inc - All rights reserved.

 NOTICE: All information contained here is, and remains
 the property of LynxiTech Incorporation. This file can not
 be copied or distributed without permission of LynxiTech Inc.

 Author: written by Yang Jun <jun.yang@lynxi.com>, 20-3-6

**************************************************/

#include "GpuContext.h"
#include <nlohmann/json.hpp>
#include "common/Logger.h"


namespace lynxi {
namespace ivs {
namespace multimedia {
namespace context {

constexpr const char* GpuContext::JSON_URL;
constexpr const char* GpuContext::JSON_RESIZE_RATE;
constexpr const char* GpuContext::JSON_TIMEOUT;
constexpr const char* GpuContext::JSON_CHANNELID;
constexpr const char* GpuContext::JSON_SOURCE_TYPE;
constexpr const char* GpuContext::JSON_ROI;
constexpr const char* GpuContext::JSON_X;
constexpr const char* GpuContext::JSON_Y;
constexpr const char* GpuContext::JSON_W;
constexpr const char* GpuContext::JSON_H;

/**
 * context初始化
 * @param[in] json: 初始化的json字符串
 * @return 错误码
 */
common::ErrorCode GpuContext::init(const std::string& json) {
    common::ErrorCode errorCode = common::ErrorCode::SUCCESS;
    do {
        auto configure = nlohmann::json::parse(json, nullptr, false);
        if (!configure.is_object()) {
            IVS_ERROR("Parse json fail or json is not object, json: {0}", json);
            errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
            break;
        }

        auto urlIt = configure.find(JSON_URL);
        if (configure.end() == urlIt
                || !urlIt->is_string()) {
            IVS_ERROR("Can not find {0} with string type in worker json configure, json: {1}",
                      JSON_URL,
                      json);
            errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
            break;
        }
        mUrl = urlIt->get<std::string>();

//        auto resizeRateIt = configure.find(JSON_RESIZE_RATE);
//        if (configure.end() == resizeRateIt
//                || !resizeRateIt->is_number_float()) {
//            IVS_ERROR("Can not find {0} with float type in worker json configure, json: {1}",
//                      JSON_RESIZE_RATE,
//                      json);
//            errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
//            break;
//        }
//        mResizeRate = resizeRateIt->get<float>();

        auto timeoutIt = configure.find(JSON_TIMEOUT);
        if (configure.end() != timeoutIt
                && timeoutIt->is_number_integer()) {
            mTimeout = timeoutIt->get<int>();
        }

        auto sourceTypeIter = configure.find(JSON_SOURCE_TYPE);
        if(configure.end()==sourceTypeIter
                ||!sourceTypeIter->is_number_integer()){
            IVS_ERROR("Can not find {0} with integer type in worker json configure, json: {1}",
                      JSON_SOURCE_TYPE,
                      json);
            errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
            break;
        }

        auto roiIt = configure.find(JSON_ROI);
        if (configure.end() != roiIt
                && roiIt->is_object()) {
            auto xIt = roiIt->find(JSON_X);
            if (roiIt->end() == xIt
                    || !xIt->is_number_integer()) {
                IVS_ERROR("Parse roi-x failed or x is not integer, json: {0}", json);
                errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
                break;
            }
            mRoi.mX = *xIt;
            auto yIt = roiIt->find(JSON_Y);
            if (roiIt->end() == yIt
                    || !yIt->is_number_integer()) {
                IVS_ERROR("Parse roi-y failed or y is not integer, json: {0}", json);
                errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
                break;
            }
            mRoi.mY = *yIt;
            auto wIt = roiIt->find(JSON_W);
            if (roiIt->end() == wIt
                    || !wIt->is_number_integer()) {
                IVS_ERROR("Parse roi-w failed or w is not integer, json: {0}", json);
                errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
                break;
            }
            mRoi.mWidth = *wIt;
            auto hIt = roiIt->find(JSON_H);
            if (roiIt->end() == hIt
                    || !hIt->is_number_integer()) {
                IVS_ERROR("Parse roi-h failed or h is not integer, json: {0}", json);
                errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
                break;
            }
            mRoi.mHeight = *hIt;
        }
        else{
            mRoi.mX = 0;
            mRoi.mY = 0;
            mRoi.mWidth = 0;
            mRoi.mHeight = 0;
        }

        mSourceType = sourceTypeIter->get<int>();


    } while (false);


    return errorCode;
}
} // namespace context
} // namespace multimedia
} // namespace ivs
} // namespace lynxi


