#pragma once

#include <functional>
#include <memory>

#include "common/Singleton.hpp"

#include "AlgorithmApi.h"

namespace sophon_stream {
namespace algorithm {

/**
 * 算法模块API工厂
 */
class AlgorithmApiFactory {
public:
    using AlgorithmApiMaker = std::function<std::shared_ptr<algorithm::AlgorithmApi>()>;

    /**
     * 设置算法API产生器
     * @param[in] algorithmApiMaker:
     */
    void addAlgorithmApiMaker(const std::string& algorithmApiName, AlgorithmApiMaker algorithmApiMaker) {
        auto algorithmApiMakerIt = mAlgorithmApiMakerMap.find(algorithmApiName);
        std::cout << "current algorithmApi added:" << algorithmApiName << std::endl;
        if (mAlgorithmApiMakerMap.end() != algorithmApiMakerIt) {
        }

        mAlgorithmApiMakerMap[algorithmApiName] = algorithmApiMaker;
    }

    /**
     * 获取产生器
     */
    std::shared_ptr<algorithm::AlgorithmApi> make(const std::string& algorithmApiName) {
        auto algorithmApiMakerIt = mAlgorithmApiMakerMap.find(algorithmApiName);
        if (mAlgorithmApiMakerMap.end() != algorithmApiMakerIt
                && algorithmApiMakerIt->second) {
            return algorithmApiMakerIt->second();
        } else {
            return std::shared_ptr<algorithm::AlgorithmApi>();
        }
    }
    
private:
    friend class common::Singleton<AlgorithmApiFactory>;

    AlgorithmApiFactory() {}
    ~AlgorithmApiFactory() {}

    AlgorithmApiFactory(const AlgorithmApiFactory&) = delete;
    AlgorithmApiFactory& operator =(const AlgorithmApiFactory&) = delete;
    AlgorithmApiFactory(AlgorithmApiFactory&&) = delete;
    AlgorithmApiFactory& operator =(AlgorithmApiFactory&&) = delete;

    std::map<std::string, AlgorithmApiMaker> mAlgorithmApiMakerMap;
};

// 算法API的工厂是个单例
using SingletonAlgorithmApiFactory = common::Singleton<AlgorithmApiFactory>;

// 算法API的注册函数
#define REGISTER_ALGORITHM_API(algorithmApiName, AlgorithmApiClass) \
    struct AlgorithmApiClass##Register { \
        AlgorithmApiClass##Register() { \
            std::cout<<algorithmApiName<<std::endl; \
            auto& algorithmApiFactory = ::sophon_stream::algorithm::SingletonAlgorithmApiFactory::getInstance(); \
            algorithmApiFactory.addAlgorithmApiMaker(algorithmApiName, []() { \
                                                         return std::make_shared<AlgorithmApiClass>(); \
                                                     }); \
        } \
    }; \
    static AlgorithmApiClass##Register g##AlgorithmApiClass##Register;

} // namespace algorithm
} // namespace sophon_stream

