#pragma once

#include <functional>
#include <memory>

#include "common/Singleton.hpp"

#include "MultiMediaApi.h"

namespace sophon_stream {
namespace multimedia {

/**
 * 多媒体模块API工厂
 */
class MultiMediaApiFactory {
  public:
    using MultiMediaApiMaker = std::function<std::shared_ptr<multimedia::MultiMediaApi>()>;

    /**
     * 设置算法API产生器
     * @param[in] algorithmApiMaker:
     */
    void setMultiMediaApiMaker(MultiMediaApiMaker multimediaApiMaker) {
        mMultiMediaApiMaker = multimediaApiMaker;
    }

    /**
     * 获取产生器
     */
    std::shared_ptr<multimedia::MultiMediaApi> make() {
        return mMultiMediaApiMaker();
    }

  private:
    friend class common::Singleton<MultiMediaApiFactory>;

    MultiMediaApiFactory() {}
    ~MultiMediaApiFactory() {}

    MultiMediaApiFactory(const MultiMediaApiFactory&) = delete;
    MultiMediaApiFactory& operator =(const MultiMediaApiFactory&) = delete;
    MultiMediaApiFactory(MultiMediaApiFactory&&) = delete;
    MultiMediaApiFactory& operator =(MultiMediaApiFactory&&) = delete;

    MultiMediaApiMaker mMultiMediaApiMaker;
};

using SingletonMultiMediaApiFactory = common::Singleton<MultiMediaApiFactory>;

#define REGISTER_MULTIMEDIA_API(MultiMediaApiClass) \
    struct MultiMediaApiClass##Register { \
        MultiMediaApiClass##Register() { \
            auto& multimediaApiFactory = ::sophon_stream::multimedia::SingletonMultiMediaApiFactory::getInstance(); \
            multimediaApiFactory.setMultiMediaApiMaker([]() { \
                                                         return std::make_shared<MultiMediaApiClass>(); \
                                                     }); \
        } \
    }; \
    static MultiMediaApiClass##Register g##MultiMediaApiClass##Register;

} // namespace multimedia
} // namespace sophon_stream

