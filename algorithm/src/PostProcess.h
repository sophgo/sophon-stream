#pragma once

#include "Context.h"
#include "common/ObjectMetadata.h"

namespace sophon_stream {
namespace algorithm {
/**
 * 后处理基类
 */
class PostProcess {
  public:
    /**
     * 初始化
     * @param[in] context: 初始化的参数
     */
    virtual void init(algorithm::Context& context) = 0;
    /**
     * 执行后处理
     * @param[in] context: 预测的结果
     * @param[in/out] objectMetadatas:  输入数据和最终输出的结果
     */
    virtual void postProcess(algorithm::Context& context,
                             common::ObjectMetadatas& objectMetadatas) = 0;
};

} // namespace algorithm
} // namespace sophon_stream

