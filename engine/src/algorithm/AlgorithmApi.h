#pragma once

#include <string>

#include "common/ErrorCode.h"
#include "common/ObjectMetadata.h"

namespace sophon_stream {
namespace algorithm {

/**
 * 算法模块API
 */
class AlgorithmApi {
public:
    /**
     * 构造函数
     */
    AlgorithmApi() {}
    /**
     * 析构函数
     */
    virtual ~AlgorithmApi() {}

    AlgorithmApi(const AlgorithmApi&) = delete;
    AlgorithmApi& operator =(const AlgorithmApi&) = delete;
    AlgorithmApi(AlgorithmApi&&) = default;
    AlgorithmApi& operator =(AlgorithmApi&&) = default;

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
    virtual void process(common::ObjectMetadatas& objectMetadatas) = 0;
    /**
     * 资源释放函数
     */
    virtual void uninit() = 0;
};

} // namespace algorithm
} // namespace sophon_stream

