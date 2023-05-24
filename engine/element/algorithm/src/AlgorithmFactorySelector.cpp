#include "AlgorithmFactorySelector.h"

namespace sophon_stream {
namespace algorithm {
/**
 * 存储算法工厂
 * @param[in] side:  设备类型
 * @param[in] algorithmName:  算法名字
 * @param[in] algorithmFactoryFetcher:  算法工厂
 */
void AlgorithmFactorySelector::setAlgoritemFactoryFetcher(const std::string& side,
        const std::string& algorithmName,
        AlgorithmFactoryFetcher algorithmFactoryFetcher) {
    mAlgorithmFactoryFetcherMapMap[side][algorithmName] = algorithmFactoryFetcher;
}

/**
 * 删除算法工厂
 * @param[in] side:  设备类型
 * @param[in] algorithmName:  算法名字
 */
void AlgorithmFactorySelector::resetAlgoritemFactoryFetcher(const std::string& side,
        const std::string& algorithmName) {
    auto algorithmFactoryFetcherMapIt = mAlgorithmFactoryFetcherMapMap.find(side);
    if (mAlgorithmFactoryFetcherMapMap.end() == algorithmFactoryFetcherMapIt) {
        return;
    }

    auto& algorithmFactoryFetcherMap = algorithmFactoryFetcherMapIt->second;
    algorithmFactoryFetcherMap.erase(algorithmName);

    if (algorithmFactoryFetcherMap.empty()) {
        mAlgorithmFactoryFetcherMapMap.erase(side);
    }
}

/**
 * 获取算法工厂
 * @param[in] side:  设备类型
 * @param[in] algorithmName:  算法名字
 */
algorithm::AlgorithmFactory& AlgorithmFactorySelector::fetch(const std::string& side,
        const std::string& algorithmName) {

    auto algorithmFactoryFetcherMapIt = mAlgorithmFactoryFetcherMapMap.find(side);
    if (mAlgorithmFactoryFetcherMapMap.end() == algorithmFactoryFetcherMapIt) {
        return *(algorithm::AlgorithmFactory*)nullptr;
    }

    auto& algorithmFactoryFetcherMap = algorithmFactoryFetcherMapIt->second;

    auto iter = algorithmFactoryFetcherMap.find(algorithmName);

    if(algorithmFactoryFetcherMap.end()==iter) {
        return *(algorithm::AlgorithmFactory*)nullptr;
    }
    return mAlgorithmFactoryFetcherMapMap[side][algorithmName]();
}

} // namespace algorithm
} // namespace sophon_stream
