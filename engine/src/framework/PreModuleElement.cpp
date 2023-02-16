#include "PreModuleElement.h"

#include <nlohmann/json.hpp>

#include "common/Logger.h"
#include "common/ObjectMetadata.h"

#include "ElementFactory.h"

namespace sophon_stream {
namespace framework {

/**
 * Constructor of class PreModuleElement.
 */
PreModuleElement::PreModuleElement()
    : mStep(0),
      mModuleCount(0) {
}

/**
 * Destructor of class PreModuleElement.
 */
PreModuleElement::~PreModuleElement() {
}

int PreModuleElement::gCurrentElementId = PreModuleElement::BEGIN_WORKER_ID;
constexpr const char* PreModuleElement::NAME;
constexpr const char* PreModuleElement::JSON_MODULE_COUNT_FIELD;

/**
 * 执行初始化.
 * @param[in] json:
 * @return 错误码.
 */
common::ErrorCode PreModuleElement::initInternal(const std::string& json) {
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
            IVS_ERROR("Can not find {0} with integer type in pre module element json configure, json: {1}",
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
void PreModuleElement::uninitInternal() {
}

/**
 * When push data to input port of this Element, will call this function in run(). 
 * Each push data will cause one success call.
 * If milliseconds timeout is not 0, timeout will also call this function, 
 * and if repeated timeout is true, this function will be call repeated with repeated timeout. 
 */
common::ErrorCode PreModuleElement::doWork() {
    if (0 == getDataCount(0)) {
        return common::ErrorCode::SUCCESS;
    }

    auto data = getData(0);
    if (!data) {
        popData(0);
        return common::ErrorCode::SUCCESS;
    }

    auto objectMetadata = std::static_pointer_cast<common::ObjectMetadata>(data);
    int currentStep = 0;
    if (currentStep == mStep) {
        common::ErrorCode errorCode = sendData(0,
                                               data,
                                               std::chrono::milliseconds(200));
        if (common::ErrorCode::SUCCESS != errorCode) {
            IVS_WARN("Send data fail, element id: {0:d}, output port: {1:d}, data: {2:p}",
                     getId(),
                     0,
                     data.get());
            return errorCode;
        }
        ++mStep;
    }

    ++currentStep;
    const auto& subObjectMetadatas = objectMetadata->mSubObjectMetadatas;
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

        if (currentStep == mStep) {
            common::ErrorCode errorCode = sendData(classify + 1,
                                                   std::static_pointer_cast<void>(subObjectMetadata),
                                                   std::chrono::milliseconds(200));
            if (common::ErrorCode::SUCCESS != errorCode) {
                IVS_WARN("Send data fail, element id: {0:d}, output port: {1:d}, data: {2:p}",
                         getId(),
                         classify + 1,
                         data.get());
                return errorCode;
            }
            ++mStep;
        }
        ++currentStep;
    }

    mStep = 0;
    popData(0);
    return common::ErrorCode::SUCCESS;
}

REGISTER_WORKER(PreModuleElement::NAME, PreModuleElement)

} // namespace framework
} // namespace sophon_stream
