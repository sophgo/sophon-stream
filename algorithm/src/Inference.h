#pragma once

#include <memory>
#include <string>
#include <vector>
#include "Context.h"
#include "common/ObjectMetadata.h"

namespace sophon_stream {
namespace algorithm {

/**
 * 推理基类
 */
class Inference {
  public:
    /**
     * 模型初始化
     * @param[in] context: 模型初始化的路径和配置参数
     * @return 错误码
     */
    virtual common::ErrorCode init(algorithm::Context& context) = 0;

    /**
     * 模型预测
     * @param[in/out] context: 输入数据和预测结果
     * @return 错误码
     */
    virtual common::ErrorCode predict(algorithm::Context& context, common::ObjectMetadatas &objectMetadatas) = 0;

    virtual void uninit() = 0;


};

} // namespace algorithm
} // namespace sophon_stream

