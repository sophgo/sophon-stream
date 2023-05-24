#pragma once

#include <string>

#include "common/ErrorCode.h"
#include "common/ObjectMetadata.h"

namespace sophon_stream {
namespace multimedia {

/**
 * 多媒体模块API
 */
class MultiMediaApi {
  public:
    /**
     * 构造函数
     */
    MultiMediaApi() {}
    /**
     * 析构函数
     */
    virtual ~MultiMediaApi() {}

    MultiMediaApi(const MultiMediaApi&) = delete;
    MultiMediaApi& operator =(const MultiMediaApi&) = delete;
    MultiMediaApi(MultiMediaApi&&) = default;
    MultiMediaApi& operator =(MultiMediaApi&&) = default;

    /**
     * 初始化函数
     * @param[in] side:  设备类型
     * @param[in] deviceId:  设备ID
     * @param[in] json:  初始化字符串
     * @return 错误码
     */
    virtual common::ErrorCode init(const std::string& side,
                                   int deviceId,
                                   const std::string& json) = 0;
    /**
     * 预测函数
     * @param[in/out] objectMetadatas:  输入数据和预测结果
     */
    virtual common::ErrorCode process(std::shared_ptr<common::ObjectMetadata>& objectMetadata) = 0;
    /**
     * 资源释放函数
     */
    virtual void uninit() = 0;
};

} // namespace multimedia 
} // namespace sophon_stream

