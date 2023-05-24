#include "ActionElement.h"

#include <dlfcn.h>
#include <set>
#include <nlohmann/json.hpp>

#include "algorithm/src/AlgorithmApiFactory.h"
#include "common/Logger.h"
#include "common/ObjectMetadata.h"
#include "framework/ElementFactory.h"

namespace sophon_stream {
namespace element {

ActionElement::ActionElement()
    : mBatch(1),
      mLastDataCount(0) {
}

ActionElement::~ActionElement() {
}

void ActionElement::doSth() {

}

constexpr const char* JSON_BATCH_FIELD = "batch";
constexpr const char* JSON_MODELS_FIELD = "models";
constexpr const char* JSON_MODEL_NAME_FIELD = "name";
constexpr const char* JSON_MODEL_SHARED_OBJECT_FIELD = "shared_object";
#define JSON_ALGORITHM_NAME_FIELD "algorithm_name"

common::ErrorCode ActionElement::initInternal(const std::string& json) {
    common::ErrorCode errorCode = common::ErrorCode::SUCCESS;

    do {
        auto configure = nlohmann::json::parse(json, nullptr, false);
        if (!configure.is_object()) {
            IVS_ERROR("Parse json fail or json is not object, element id: {0:d}, json: {0}",
                      getId(),
                      json);
            errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
            break;
        }

        auto batchIt = configure.find(JSON_BATCH_FIELD);
        if (configure.end() == batchIt
                || !batchIt->is_number_integer()) {
            IVS_ERROR("Can not find {0} with integer type in action element json configure, element id: {1:d}, json: {2}",
                      JSON_BATCH_FIELD,
                      getId(),
                      json);
            errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
            break;
        }

        mBatch = batchIt->get<int>();
        mPendingObjectMetadatas.reserve(mBatch);

        auto modelsIt = configure.find(JSON_MODELS_FIELD);
        if (configure.end() == modelsIt
                || !modelsIt->is_array()
                || modelsIt->empty()) {
            IVS_ERROR("Can not find non empty {0} with array type in action element json configure, element id: {1:d}, json: {2}",
                      JSON_MODELS_FIELD,
                      getId(),
                      json);
            errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
            break;
        }


        std::set<std::string> sharedObjectSet;
        mSharedObjectHandles.reserve(modelsIt->size());
        mAlgorithmApis.reserve(modelsIt->size());
        for (auto modelConfigure : *modelsIt) {
            auto sharedObjectIt = modelConfigure.find(JSON_MODEL_SHARED_OBJECT_FIELD);
            if (modelConfigure.end() == sharedObjectIt
                    || !sharedObjectIt->is_string()
                    || sharedObjectIt->empty()) {
                IVS_ERROR("Can not find non empty {0} with string type in model json configure, element id: {1:d}, json: {2}",
                          JSON_MODEL_SHARED_OBJECT_FIELD,
                          getId(),
                          modelConfigure.dump());
                errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
                break;
            }

            if (sharedObjectSet.end() == sharedObjectSet.find(*sharedObjectIt)) {
                const auto& sharedObject = sharedObjectIt->get<std::string>();
                sharedObjectSet.insert(sharedObject);

                void* sharedObjectHandle = dlopen(sharedObject.c_str(), RTLD_NOW | RTLD_GLOBAL);
                if (NULL == sharedObjectHandle) {
                    IVS_ERROR("Load dynamic shared object file fail, element id: {0:d}, "
                              "shared object: {1}  error info:{2}",
                              getId(),
                              sharedObject,dlerror());
                    errorCode = common::ErrorCode::DLOPEN_FAIL;
                    break;
                }

                mSharedObjectHandles.push_back(std::shared_ptr<void>(sharedObjectHandle,
                [](void* sharedObjectHandle) {
                    dlclose(sharedObjectHandle);
                }));
            }


            if (!modelConfigure.is_object()) {
                errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
                break;
            }
            //找到算法名字
            auto algotirhmNameIt = modelConfigure.find(JSON_ALGORITHM_NAME_FIELD);
            if (modelConfigure.end() == algotirhmNameIt
                    || !algotirhmNameIt->is_string()) {
                errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
                break;
            }
            //根据设备类型和算法名字找到对应的工厂
            std::string algorithmName = algotirhmNameIt->get<std::string>();

            auto& algorithmApiFactory = algorithm::SingletonAlgorithmApiFactory::getInstance();
            auto algorithmApi = algorithmApiFactory.make(algorithmName);
            if (!algorithmApi) {
                errorCode = common::ErrorCode::MAKE_ALGORITHM_API_FAIL;
                IVS_ERROR("Make algorithm api fail, element id: {0:d}",
                          getId());
                break;
            }

            errorCode = algorithmApi->init(getSide(),
                                           getDeviceId(),
                                           modelConfigure.dump());
            if (common::ErrorCode::SUCCESS != errorCode) {
                IVS_ERROR("Init model fail, element id: {0:d}, json: {1}, errorcode: {2}",
                          getId(),
                          modelConfigure.dump(), (int)errorCode);
                break;
            }

            std::string name;
            auto nameIt = modelConfigure.find(JSON_MODEL_NAME_FIELD);
            if (modelConfigure.end() != nameIt
                    && nameIt->is_string()) {
                name = nameIt->get<std::string>();
            }


            mAlgorithmApis.push_back(std::make_pair(name, algorithmApi));
        }
        if (common::ErrorCode::SUCCESS != errorCode) {
            break;
        }
    } while (false);

    return errorCode;
}

void ActionElement::uninitInternal() {
    for(auto& algorithm:mAlgorithmApis){
        algorithm.second->uninit();
    }
    mAlgorithmApis.clear();
    mSharedObjectHandles.clear();
    mBatch = 1;
}

common::ErrorCode ActionElement::doWork() {
    common::ErrorCode errorCode = common::ErrorCode::SUCCESS;

    common::ObjectMetadatas objectMetadatas;
    {
        std::lock_guard<std::mutex> lock(mMutex);
        
        std::size_t currentDataCount = getDataCount(0);
        bool timeout = mLastDataCount == currentDataCount;
        mLastDataCount = currentDataCount;

        //IVS_INFO("element id:{2}, timeout:{0}, current data count:{1}",timeout, currentDataCount, getId());
        errorCode = sendProcessedData();
        if (common::ErrorCode::SUCCESS != errorCode) {
            return errorCode;
        }

        int pendingSize = mPendingObjectMetadatas.size();
        int iCurrentdataCount = currentDataCount;
        if (mBatch - pendingSize > iCurrentdataCount
                && !timeout) {

            return common::ErrorCode::SUCCESS;
        }

        // ActionElement凑batch
        while(mPendingObjectMetadatas.size() < mBatch && !hasEof)
        {
            // 如果队列为空则等待
            if(getDataCount(0) == 0)
            {
                continue;
            }
            auto data = getData(0);
            if (!data) {
                popData(0);
                continue;
            }
            auto objectMetadata = std::static_pointer_cast<common::ObjectMetadata>(data);
            bool skip = false;

            if (objectMetadata->mFilter) {
                skip = true;
            }

            auto modelConfigureMap = objectMetadata->mModelConfigureMap;
            if (modelConfigureMap) {
                IVS_WARN("modelConfigure map has data!");
                bool skipAllModel = true;
                for (auto pair : mAlgorithmApis) {
                    auto modelConfigureIt = modelConfigureMap->find(pair.first);
                    if (modelConfigureMap->end() == modelConfigureIt
                            || modelConfigureIt->second) {
                        skipAllModel = false;
                        break;
                    }
                }
                if (skipAllModel) {
                    skip = true;
                }
            }

            if (skip) {
                mProcessedObjectMetadatas.push_back(objectMetadata);
            } else {
                mPendingObjectMetadatas.push_back(objectMetadata);
            }
            popData(0);
            
            // 如果遇到了eof，将eof帧凑到batch里之后截止
            if(objectMetadata->mFrame->mEndOfStream)
            {    
                hasEof = true;
                break;
            }
        }

        // 遇到了eof的情况凑batch
        if(mPendingObjectMetadatas.size() != 0 && mPendingObjectMetadatas.back()->mFrame!=nullptr&& mPendingObjectMetadatas.back()->mFrame->mEndOfStream)
        {
            while(mPendingObjectMetadatas.size() < mBatch)
            {
                auto ObjectMetadata = std::make_shared<common::ObjectMetadata>();
                ObjectMetadata->mFrame = std::make_shared<common::Frame>();
                ObjectMetadata->mFrame->mEndOfStream = true;
                mPendingObjectMetadatas.push_back(ObjectMetadata);
            }
        }

        mLastDataCount = getDataCount(0);

        errorCode = sendProcessedData();
        if (common::ErrorCode::SUCCESS != errorCode) {
            return errorCode;
        }

        if (mPendingObjectMetadatas.size() < mBatch
                && !timeout) {
            return common::ErrorCode::SUCCESS;
        }

        objectMetadatas.swap(mPendingObjectMetadatas);
    }


    for (auto pair : mAlgorithmApis) {
        auto algorithmApi = pair.second;
        algorithmApi->process(objectMetadatas);
    }

    {
        std::lock_guard<std::mutex> lock(mMutex);

        for (auto objectMetadata : objectMetadatas) {
            mProcessedObjectMetadatas.push_back(objectMetadata);
        }
        errorCode = sendProcessedData();
        if (common::ErrorCode::SUCCESS != errorCode) {
            return errorCode;
        }
    }

    return common::ErrorCode::SUCCESS;
}

common::ErrorCode ActionElement::sendProcessedData() {
    while (!mProcessedObjectMetadatas.empty()) {
        auto objectMetadata = mProcessedObjectMetadatas.front();
        common::ErrorCode errorCode = sendData(0,
                                               std::static_pointer_cast<void>(objectMetadata),
                                               std::chrono::milliseconds(200));
        if (common::ErrorCode::SUCCESS != errorCode) {
            IVS_WARN("Send data fail, element id: {0:d}, output port: {1:d}, data: {2:p}, element id:{3:d}",
                     getId(),
                     0,
                     static_cast<void*>(objectMetadata.get()), getId());
            return errorCode;
        }

        mProcessedObjectMetadatas.pop_front();
    }

    return common::ErrorCode::SUCCESS;
}
    

static constexpr const char* NAME = "action_element";
REGISTER_WORKER(NAME, ActionElement)

} // namespace framework
} // namespace sophon_stream
