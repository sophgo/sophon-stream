#include "NoopInference.h"

namespace sophon_stream {
namespace algorithm {
namespace inference {

/**
 * 模型初始化
 * @param[in] context: 模型初始化的路径和配置参数
 * @return 错误码
 */
common::ErrorCode InferenceNoop::init(algorithm::Context& context) {
    return common::ErrorCode::SUCCESS;

}

/**
 * 模型预测
 * @param[in/out] context: 输入数据和预测结果
 * @return 错误码
 */
common::ErrorCode InferenceNoop::predict(algorithm::Context& context, common::ObjectMetadatas &objectMetadatas) {
    return common::ErrorCode::SUCCESS;
}


/**
 * 资源释放
 */
void InferenceNoop::uninit() {

}

} // namespace inference
} // namespace algorithm
} // namespace sophon_stream
