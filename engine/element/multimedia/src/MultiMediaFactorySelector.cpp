#include "MultiMediaFactorySelector.h"

namespace sophon_stream {
namespace multimedia {

/**
 * 存储多媒体工厂
 * @param[in] side:  设备类型
 * @param[in] multimediaName:  多媒体名字
 * @param[in] multimediaFactoryFetcher:  多媒体工厂
 */
void MultiMediaFactorySelector::setMultiMediaFactoryFetcher(const std::string& side, 
                                                          const std::string& multimediaName, 
                                                          MultiMediaFactoryFetcher multimediaFactoryFetcher) {
    mMultiMediaFactoryFetcherMapMap[side][multimediaName] = multimediaFactoryFetcher;
}

/**
 * 删除多媒体工厂
 * @param[in] side:  设备类型
 * @param[in] multimediaName:  多媒体名字
 */
void MultiMediaFactorySelector::resetMultiMediaFactoryFetcher(const std::string& side, 
                                                            const std::string& multimediaName) {
    auto multimediaFactoryFetcherMapIt = mMultiMediaFactoryFetcherMapMap.find(side);
    if (mMultiMediaFactoryFetcherMapMap.end() == multimediaFactoryFetcherMapIt) {
        return;
    }

    auto& multimediaFactoryFetcherMap = multimediaFactoryFetcherMapIt->second;
    multimediaFactoryFetcherMap.erase(multimediaName);

    if (multimediaFactoryFetcherMap.empty()) {
        mMultiMediaFactoryFetcherMapMap.erase(side);
    }
}

/**
 * 获取多媒体工厂
 * @param[in] side:  设备类型
 * @param[in] algorithmName:  多媒体名字
 */
multimedia::MultiMediaFactory& MultiMediaFactorySelector::fetch(const std::string& side, 
                                       const std::string& multimediaName){
    
    auto multimediaFactoryFetcherMapIt = mMultiMediaFactoryFetcherMapMap.find(side);
    if (mMultiMediaFactoryFetcherMapMap.end() == multimediaFactoryFetcherMapIt) {
        return *(multimedia::MultiMediaFactory*)nullptr;
    }

    auto& multimediaFactoryFetcherMap = multimediaFactoryFetcherMapIt->second;
    
    auto iter = multimediaFactoryFetcherMap.find(multimediaName);
    
    if(multimediaFactoryFetcherMap.end()==iter){
        return *(multimedia::MultiMediaFactory*)nullptr;
    }    
    return mMultiMediaFactoryFetcherMapMap[side][multimediaName]();
}

} // namespace multimedia
} // namespace sophon_stream
