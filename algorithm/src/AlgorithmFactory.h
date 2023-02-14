#pragma once

#include <memory>

#include "Context.h"
#include "Inference.h"
#include "PostProcess.h"
#include "PreProcess.h"

namespace sophon_stream {
namespace algorithm {
/**
 * 算法工厂
 */
class AlgorithmFactory {
  public:
    /**
     * 创建context
     * @return context对象
     */
    virtual std::shared_ptr<algorithm::Context> makeContext() = 0;
    /**
     * 创建预处理
     * @return 预处理对象
     */
    virtual std::shared_ptr<algorithm::PreProcess> makePreProcess() = 0;
    /**
     * 创建推理
     * @return 推理对象
     */
    virtual std::shared_ptr<algorithm::Inference> makeInference() = 0;
    /**
     * 创建后处理
     * @return 后处理对象
     */
    virtual std::shared_ptr<algorithm::PostProcess> makePostProcess() = 0;
};

} // namespace algorithm
} // namespace sophon_stream

