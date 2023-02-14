#pragma once

#include "Context.h"
#include "common/ObjectMetadata.h"

namespace sophon_stream {
namespace algorithm {
/**
 * 预处理基类
 */
class PreProcess {
  public:
    /**
     * 执行预处理
     * @param[in] objectMetadatas:  输入数据
     * @param[out] context: 传输给推理模型的数据
     * @return 错误码
     */
    virtual common::ErrorCode preProcess(algorithm::Context& context,
                                         common::ObjectMetadatas& objectMetadatas) = 0;
};

} // namespace algorithm
} // namespace sophon_stream

