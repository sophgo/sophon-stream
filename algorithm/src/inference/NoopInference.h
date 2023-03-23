#pragma once

#include "../Inference.h"

namespace sophon_stream {
namespace algorithm {
namespace inference {

/**
 * 空的推理
 */
class InferenceNoop : public algorithm::Inference {
  public:
    /**
     * 模型初始化
     * @param[in] context: 模型初始化的路径和配置参数
     * @return 错误码
     */
    common::ErrorCode init(algorithm::Context& context) override;
    /**
     * 模型预测
     * @param[in/out] context: 输入数据和预测结果
     * @return 错误码
     */
    common::ErrorCode predict(algorithm::Context& context) override;

    /**
     * 资源释放
     */
    void uninit() override;

};

} // namespace inference
} // namespace algorithm
} // namespace sophon_stream
