#include "Algorithm.h"

#include <nlohmann/json.hpp>

#include "AlgorithmApiFactory.h"
#include "AlgorithmFactorySelector.h"
#include "common/Logger.h"
namespace sophon_stream {
namespace algorithm {

/**
 * 构造函数
 */
Algorithm::Algorithm() {
}

/**
 * 析构函数
 */
Algorithm::~Algorithm() {
    // std::cout << "Algorithm uninit" << std::endl;
}

#define JSON_ALGORITHM_NAME_FIELD "algorithm_name"
#define AGENCY_NAME "agency"
/**
 * 初始化函数
 * @param[in] side:  设备类型
 * @param[in] deviceId:  设备ID
 * @param[in] json:  初始化字符串
 * @return 错误码
 */
common::ErrorCode Algorithm::init(const std::string& side,
                                  int deviceId,
                                  const std::string& json) {
    common::ErrorCode errorCode = common::ErrorCode::SUCCESS;
    if(side!=AGENCY_NAME) {
        mAgency = false;
        do {
            //json是否正确
            auto configure = nlohmann::json::parse(json, nullptr, false);
            if (!configure.is_object()) {
                errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
                break;
            }
            //找到算法名字
            auto algotirhmNameIt = configure.find(JSON_ALGORITHM_NAME_FIELD);
            if (configure.end() == algotirhmNameIt
                    || !algotirhmNameIt->is_string()) {
                errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
                break;
            }
            //根据设备类型和算法名字找到对应的工厂
            std::string algorithmName = algotirhmNameIt->get<std::string>();
            auto& algorithmFactorySelector = algorithm::SingletonAlgorithmFactorySelector::getInstance();
            auto& algorithmFactory = algorithmFactorySelector.fetch(side, algorithmName);
            if(&algorithmFactory ==nullptr) {
                IVS_ERROR("No find algorithmName: {0} for: {1}", algorithmName, side);
                errorCode = common::ErrorCode::NO_REGISTER_ALGORITHM;
                break;
            }
            // 新建context,预处理,推理和后处理对象
            mContext = algorithmFactory.makeContext();
            mPreProcess = algorithmFactory.makePreProcess();
            mInference = algorithmFactory.makeInference();
            mPostProcess = algorithmFactory.makePostProcess();
            if(!mPreProcess
                    || !mInference
                    || !mPostProcess || !mContext) {
                errorCode = common::ErrorCode::ALGORITHM_FACTORY_MAKE_FAIL;
                break;
            }


            //context初始化
            mContext->init(configure.dump());


            mContext->algorithmName = algorithmName;
            mContext->deviceId = deviceId;
            //推理初始化
            errorCode = mInference->init(*mContext);
            //后处理初始化
            mPostProcess->init(*mContext);

        } while(false);
    } else {
        // 调用远程python的初始化
        mAgency = true;

    }
    if (common::ErrorCode::SUCCESS != errorCode) {
        uninit();
    }

    return errorCode;
}

/**
 * 预测函数
 * @param[in/out] objectMetadatas:  输入数据和预测结果
 */
void Algorithm::process(common::ObjectMetadatas& objectMetadatas) {
    if(!mAgency) {
//        if (!mPreProcess
//                || !mInference
//                || !mPostProcess) {
//            //TODO: set error code
//            return;
//        }
        //预处理
        common::ErrorCode errorCode = mPreProcess->preProcess(*mContext, objectMetadatas);
        if (common::ErrorCode::SUCCESS != errorCode) {
            for (unsigned i = 0; i < objectMetadatas.size(); i++) {
                objectMetadatas[i]->mErrorCode = errorCode;
            }
            return;
        }
        //推理
        errorCode = mInference->predict(*mContext);
        if (common::ErrorCode::SUCCESS != errorCode) {
            for (unsigned i = 0; i < objectMetadatas.size(); i++) {
                objectMetadatas[i]->mErrorCode = errorCode;
            }
            return;
        }
        //后处理
        mPostProcess->postProcess(*mContext, objectMetadatas);
    } else {
        // 调用python远程的预测

    }
}

/**
 * 资源释放函数
 */
void Algorithm::uninit() {
    if(!mAgency) {
        if (mInference != nullptr) {
            mInference->uninit();
            mInference = nullptr;
        }
    } else {
        //调用python远程的资源释放

    }
}

REGISTER_ALGORITHM_API(Algorithm)

} // namespace algorithm
} // namespace sophon_stream

