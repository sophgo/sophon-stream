#pragma once

#include "../AlgorithmFactory.h"
#include "../Context.h"
#include "../Inference.h"
#include "../PostProcess.h"
#include "../PreProcess.h"


namespace sophon_stream {
namespace algorithm {
namespace factory {

/**
 * Yolov5Post工厂
 */
class Yolov5PostFactory : public algorithm::AlgorithmFactory {
  public:
    /**
     * 创建context
     * @return context对象
     */
    std::shared_ptr<algorithm::Context> makeContext() override;
    /**
     * 创建预处理
     * @return 预处理对象
     */
    std::shared_ptr<algorithm::PreProcess> makePreProcess() override;
    /**
     * 创建推理
     * @return 推理对象
     */
    std::shared_ptr<algorithm::Inference> makeInference() override;
    /**
     * 创建后处理
     * @return 后处理对象
     */
    std::shared_ptr<algorithm::PostProcess> makePostProcess() override;

};

} // namespace factory
} // namespace algorithm
} // namespace sophon_stream

