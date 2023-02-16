/**********************************************

 Copyright (c) 2018 LynxiTech Inc - All rights reserved.

 NOTICE: All information contained here is, and remains
 the property of LynxiTech Incorporation. This file can not
 be copied or distributed without permission of LynxiTech Inc.

 Author: written by Jun Yang <jun.yang@lynxi.com>, 20-3-2

**************************************************/

#include "PostModuleElement.h"

#include <nlohmann/json.hpp>

#include "common/ObjectMetadata.h"

#include "ElementFactory.h"

namespace sophon_stream {
namespace framework {

/**
 * Constructor of class PreModuleElement.
 */
PostModuleElement::PostModuleElement()
    : mModuleCount(0) {
}

/**
 * Destructor of class PreModuleElement.
 */
PostModuleElement::~PostModuleElement() {
}

int PostModuleElement::gCurrentElementId = PostModuleElement::BEGIN_WORKER_ID;
constexpr const char* PostModuleElement::NAME;
constexpr const char* PostModuleElement::JSON_MODULE_COUNT_FIELD;

/**
 * 执行初始化.
 * @param[in] json:
 * @return 错误码.
 */
common::ErrorCode PostModuleElement::initInternal(const std::string& json) {
    common::ErrorCode errorCode = common::ErrorCode::SUCCESS;

    do {
        auto configure = nlohmann::json::parse(json, nullptr, false); 
        if (!configure.is_object()) {
            IVS_ERROR("Parse json fail or json is not object, json: {0}", json);
            errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
            break;
        }

        auto moduleCountIt = configure.find(JSON_MODULE_COUNT_FIELD);
        if (configure.end() == moduleCountIt
                || !moduleCountIt->is_number_integer()) {
            IVS_ERROR("Can not find {0} with integer type in post module element json configure, json: {1}", 
                       JSON_MODULE_COUNT_FIELD, 
                       json);
            errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
            break;
        }

        mModuleCount = moduleCountIt->get<int>();
    } while (false);

    return errorCode;
}

/**
 * 执行销毁.
 */
void PostModuleElement::uninitInternal() {
}

/**
 * When push data to input port of this Element, will call this function in run(). 
 * Each push data will cause one success call.
 * If milliseconds timeout is not 0, timeout will also call this function, 
 * and if repeated timeout is true, this function will be call repeated with repeated timeout. 
 */
common::ErrorCode PostModuleElement::doWork() {
    if (0 != getDataCount(1)) {
        auto subData = getData(1);
        if (subData) {
            auto subObjectMetadata = std::static_pointer_cast<common::ObjectMetadata>(subData);
            mSubObjectMetadataSet.insert(subObjectMetadata);
        }

        popData(1);
    }

    while (0 != getDataCount(0)) {
        auto data = getData(0);
        if (!data) {
            popData(0);
            return common::ErrorCode::SUCCESS;
        }

        auto objectMetadata = std::static_pointer_cast<common::ObjectMetadata>(data);
        auto subObjectMetadatas = objectMetadata->mSubObjectMetadatas;
        for (auto subObjectMetadata : subObjectMetadatas) {
            if (!subObjectMetadata) {
                IVS_ERROR("Sub object metadata is null");
                continue;
            }

            auto subDataInformation = subObjectMetadata->mSpDataInformation;
            if (!subDataInformation) {
                IVS_ERROR("Sub detected object metadata is null");
                continue;
            }

            int classify = subDataInformation->mClassify;
            if (classify < 0
                    || classify >= mModuleCount) {
                continue;
            }

            if (mSubObjectMetadataSet.end() == mSubObjectMetadataSet.find(subObjectMetadata)) {
                return common::ErrorCode::SUCCESS;
            }
        }

        common::ErrorCode errorCode = sendData(0, 
                                               data, 
                                               std::chrono::milliseconds(200));
        if (common::ErrorCode::SUCCESS != errorCode) {
            return errorCode;
        }

        for (auto subObjectMetadata : subObjectMetadatas) {
            mSubObjectMetadataSet.erase(subObjectMetadata);
        }
        popData(0);
    }

    return common::ErrorCode::SUCCESS;
}

REGISTER_WORKER(PostModuleElement::NAME, PostModuleElement)

} // namespace framework
} // namespace sophon_stream
