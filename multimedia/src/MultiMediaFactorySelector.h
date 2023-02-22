#pragma once

#include <map>
#include <string>
#include <functional>
#include "common/Singleton.hpp"

#include "MultiMediaFactory.h"

namespace sophon_stream {
namespace multimedia {

/**
 * 多媒体工厂选择器
 */
class MultiMediaFactorySelector {
  public:
    using MultiMediaFactoryFetcher = std::function<multimedia::MultiMediaFactory&()>;

    /**
     * 存储多媒体工厂
     * @param[in] side:  设备类型
     * @param[in] multimediaName:  多媒体名字
     * @param[in] multimediaFactoryFetcher:  多媒体工厂
     */
    void setMultiMediaFactoryFetcher(const std::string& side,
                                    const std::string& multimediaName,
                                    MultiMediaFactoryFetcher multimediaFactoryFetcher);
    /**
     * 删除多媒体工厂
     * @param[in] side:  设备类型
     * @param[in] multimediaName:  多媒体名字
     */
    void resetMultiMediaFactoryFetcher(const std::string& side,
                                      const std::string& multimediaName);

    /**
     * 获取多媒体工厂
     * @param[in] side:  设备类型
     * @param[in] algorithmName:  多媒体名字
     */
    multimedia::MultiMediaFactory& fetch(const std::string& side,
                                       const std::string& multimediaName);

  private:
    friend class common::Singleton<MultiMediaFactorySelector>;

    MultiMediaFactorySelector() {}
    ~MultiMediaFactorySelector() {}

    MultiMediaFactorySelector(const MultiMediaFactorySelector&) = delete;
    MultiMediaFactorySelector& operator =(const MultiMediaFactorySelector&) = delete;
    MultiMediaFactorySelector(MultiMediaFactorySelector&&) = delete;
    MultiMediaFactorySelector& operator =(MultiMediaFactorySelector&&) = delete;

    using MultiMediaFactoryFetcherMap = std::map<std::string, MultiMediaFactoryFetcher>;
    using MultiMediaFactoryFetcherMapMap = std::map<std::string, MultiMediaFactoryFetcherMap>;
    MultiMediaFactoryFetcherMapMap mMultiMediaFactoryFetcherMapMap;
};

using SingletonMultiMediaFactorySelector = common::Singleton<MultiMediaFactorySelector>;

#define REGISTER_MULTIMEDIA_FACTORY(side, multimediaName, MultiMediaFactoryClass) \
    struct MultiMediaFactoryClass##Register { \
        MultiMediaFactoryClass##Register() { \
            auto& multimediaFactorySelector = ::lynxi::ivs::multimedia::SingletonMultiMediaFactorySelector::getInstance(); \
            multimediaFactorySelector.setMultiMediaFactoryFetcher(side, \
                                                                multimediaName, \
                                                                []() -> multimedia::MultiMediaFactory& { \
                                                                    return common::Singleton<MultiMediaFactoryClass>::getInstance(); \
                                                                }); \
        } \
    }; \
    static MultiMediaFactoryClass##Register g##MultiMediaFactoryClass##Register;

} // namespace multimedia
} // namespace sophon_stream

