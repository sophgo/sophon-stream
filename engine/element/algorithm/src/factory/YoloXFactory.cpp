#include "YoloXFactory.h"

#include "../AlgorithmFactorySelector.h"
#include "../inference/YoloXInference.h"
#include "../post_process/YoloXPost.h"
#include "../pre_process/YoloXPre.h"
#include "../context/SophgoContext.h"


namespace sophon_stream {
namespace algorithm {
namespace factory {

/**
 * 创建context
 * @return context对象
 */
std::shared_ptr<algorithm::Context> YoloXFactory::makeContext() {
    return std::static_pointer_cast<algorithm::Context>(std::make_shared<context::SophgoContext>());
}

/**
 * 创建预处理
 * @return 预处理对象
 */
std::shared_ptr<algorithm::PreProcess> YoloXFactory::makePreProcess() {
    return std::static_pointer_cast<algorithm::PreProcess>(std::make_shared<pre_process::YoloXPre>());
}

/**
 * 创建推理
 * @return 推理对象
 */
std::shared_ptr<algorithm::Inference> YoloXFactory::makeInference() {
    return std::static_pointer_cast<algorithm::Inference>(std::make_shared<inference::YoloXInference>());
}

/**
 * 创建后处理
 * @return 后处理对象
 */
std::shared_ptr<algorithm::PostProcess> YoloXFactory::makePostProcess() {
    return std::static_pointer_cast<algorithm::PostProcess>(std::make_shared<post_process::YoloXPost>());
}

//注册sophgo YoloX
REGISTER_ALGORITHM_FACTORY("sophgo", "YoloX", YoloXFactory);

} // namespace factory
} // namespace algorithm
} // namespace sophon_stream
