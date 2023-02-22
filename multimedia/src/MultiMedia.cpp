#include "MultiMedia.h"

#include <nlohmann/json.hpp>

#include "MultiMediaApiFactory.h"
#include "MultiMediaFactorySelector.h"

#include "common/Logger.h"

namespace sophon_stream {
namespace multimedia {

/**
 * 构造函数
 */
MultiMedia::MultiMedia() {
}

/**
 * 析构函数
 */
MultiMedia::~MultiMedia() {
}

#define JSON_MULTIMEDIA_NAME_FIELD "multimedia_name"

/**
 * 初始化函数
 * @param[in] side:  设备类型
 * @param[in] deviceId:  设备ID
 * @param[in] json:  初始化字符串
 * @return 错误码
 */
common::ErrorCode MultiMedia::init(const std::string& side,
                                  int deviceId,
                                  const std::string& json) {
    common::ErrorCode errorCode = common::ErrorCode::SUCCESS;
    do {
        auto configure = nlohmann::json::parse(json, nullptr, false);
        if (!configure.is_object()) {
            errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
            IVS_ERROR("json parse failed! json:{0}", json);
            break;
        }

        auto multimediaNameIt = configure.find(JSON_MULTIMEDIA_NAME_FIELD);
        if (configure.end() == multimediaNameIt
                || !multimediaNameIt->is_string()) {
            errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
            IVS_ERROR("can not find {0} with string type, json parse failed! json:{1}",JSON_MULTIMEDIA_NAME_FIELD, json);
            break;
        }
        std::string multimediaName = multimediaNameIt->get<std::string>();
        auto& multimediaFactorySelector = multimedia::SingletonMultiMediaFactorySelector::getInstance();
        auto& multimediaFactory = multimediaFactorySelector.fetch(side, multimediaName);

        mContext = multimediaFactory.makeContext();
        mProcess = multimediaFactory.makeProcess();
        mContext->init(configure.dump());


        mContext->multimediaName = multimediaName;
        mContext->deviceId = deviceId;
        errorCode = mProcess->init(*mContext);

    } while(false);
    if (common::ErrorCode::SUCCESS != errorCode) {
        uninit();
    }

    return errorCode;
}

/**
 * 预测函数
 * @param[in/out] objectMetadatas:  输入数据和预测结果
 */
common::ErrorCode MultiMedia::process(std::shared_ptr<common::ObjectMetadata>& objectMetadata) {
    if (!mProcess) {
        //TODO: set error code
        IVS_ERROR("process handle is null");
        return (common::ErrorCode)(-1);
    }

    common::ErrorCode errorCode = mProcess->process(*mContext, objectMetadata);
    if (common::ErrorCode::SUCCESS != errorCode) {
        objectMetadata->mErrorCode = errorCode;
    }

    return errorCode;

}

/**
 * 资源释放函数
 */
void MultiMedia::uninit() {
    if (mProcess != nullptr) {
        mProcess->uninit();
        mProcess = nullptr;
    }
}

REGISTER_MULTIMEDIA_API(MultiMedia)

} // namespace multimedia
} // namespace sophon_stream
