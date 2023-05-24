#pragma once

#include <map>
#include <string>
#include <functional>
#include "common/Singleton.hpp"

#include "AlgorithmFactory.h"
/**
 * 算法工厂选择器
 */
namespace sophon_stream {
namespace algorithm {

class AlgorithmFactorySelector {
  public:
    // 新建一个Algorithm工厂对象
    using AlgorithmFactoryFetcher = std::function<algorithm::AlgorithmFactory&()>;
    /**
     * 存储算法工厂
     * @param[in] side:  设备类型
     * @param[in] algorithmName:  算法名字
     * @param[in] algorithmFactoryFetcher:  算法工厂
     */
    void setAlgoritemFactoryFetcher(const std::string& side,
                                    const std::string& algorithmName,
                                    AlgorithmFactoryFetcher algorithmFactoryFetcher);
    /**
     * 删除算法工厂
     * @param[in] side:  设备类型
     * @param[in] algorithmName:  算法名字
     */
    void resetAlgoritemFactoryFetcher(const std::string& side,
                                      const std::string& algorithmName);

    /**
     * 获取算法工厂
     * @param[in] side:  设备类型
     * @param[in] algorithmName:  算法名字
     */
    algorithm::AlgorithmFactory& fetch(const std::string& side,
                                       const std::string& algorithmName);

  private:
    friend class common::Singleton<AlgorithmFactorySelector>;

    AlgorithmFactorySelector() {}
    ~AlgorithmFactorySelector() {}

    AlgorithmFactorySelector(const AlgorithmFactorySelector&) = delete;
    AlgorithmFactorySelector& operator =(const AlgorithmFactorySelector&) = delete;
    AlgorithmFactorySelector(AlgorithmFactorySelector&&) = delete;
    AlgorithmFactorySelector& operator =(AlgorithmFactorySelector&&) = delete;

    using AlgorithmFactoryFetcherMap = std::map<std::string, AlgorithmFactoryFetcher>;
    using AlgorithmFactoryFetcherMapMap = std::map<std::string, AlgorithmFactoryFetcherMap>;
    //算法工厂存储器
    AlgorithmFactoryFetcherMapMap mAlgorithmFactoryFetcherMapMap;
};
//单例一个算法选择器
using SingletonAlgorithmFactorySelector = common::Singleton<AlgorithmFactorySelector>;
// 注册设备号,算法名字,算法工厂
#define REGISTER_ALGORITHM_FACTORY(side, algorithmName, AlgorithmFactoryClass) \
    struct AlgorithmFactoryClass##Register { \
        AlgorithmFactoryClass##Register() { \
            auto& algorithmFactorySelector = ::sophon_stream::algorithm::SingletonAlgorithmFactorySelector::getInstance(); \
            algorithmFactorySelector.setAlgoritemFactoryFetcher(side, \
                                                                algorithmName, \
                                                                []() -> algorithm::AlgorithmFactory& { \
                                                                    return common::Singleton<AlgorithmFactoryClass>::getInstance(); \
                                                                }); \
        } \
    }; \
    static AlgorithmFactoryClass##Register g##AlgorithmFactoryClass##Register;

} // namespace algorithm
} // namespace sophon_stream


