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
    // 新建一个AlgorithmApi对象
    using AlgorithmApiMaker = std::function<std::shared_ptr<algorithm::AlgorithmApi>()>;
    /**
     * 设置算法API产生器
     * @param[in] algorithmApiMaker:
     */
    void setAlgorithmApiMaker(AlgorithmApiMaker algorithmApiMaker) {
        mAlgorithmApiMaker = algorithmApiMaker;
    }
    /**
     * 获取产生器
     */
    std::shared_ptr<algorithm::AlgorithmApi> make() {
        return mAlgorithmApiMaker();
    }

  private:
    friend class common::Singleton<AlgorithmApiFactory>;

    AlgorithmApiFactory() {}
    ~AlgorithmApiFactory() {}

    AlgorithmApiFactory(const AlgorithmApiFactory&) = delete;
    AlgorithmApiFactory& operator =(const AlgorithmApiFactory&) = delete;
    AlgorithmApiFactory(AlgorithmApiFactory&&) = delete;
    AlgorithmApiFactory& operator =(AlgorithmApiFactory&&) = delete;

    AlgorithmApiMaker mAlgorithmApiMaker;
};

// 算法API的工厂是个单例
using SingletonAlgorithmApiFactory = common::Singleton<AlgorithmApiFactory>;

// 算法API的注册函数
#define REGISTER_ALGORITHM_API(AlgorithmApiClass) \
    struct AlgorithmApiClass##Register { \
        AlgorithmApiClass##Register() { \
            auto& algorithmApiFactory = ::sophon_stream::algorithm::SingletonAlgorithmApiFactory::getInstance(); \
            algorithmApiFactory.setAlgorithmApiMaker([]() { \
                                                         return std::make_shared<AlgorithmApiClass>(); \
                                                     }); \
        } \
    }; \
    static AlgorithmApiClass##Register g##AlgorithmApiClass##Register;

} // namespace algorithm
} // namespace sophon_stream

