
#include "Yolov5PostFactory.h"

#include "../AlgorithmFactorySelector.h"
#include "../inference/Yolov5PostInference.h"
#include "../post_process/Yolov5PostPost.h"
#include "../pre_process/Yolov5PostPre.h"
#include "../context/SophgoContext.h"


namespace sophon_stream {
namespace algorithm {
namespace factory {

/**
 * 创建context
 * @return context对象
 */
std::shared_ptr<algorithm::Context> Yolov5PostFactory::makeContext() {
    return std::static_pointer_cast<algorithm::Context>(std::make_shared<context::SophgoContext>());
}

/**
 * 创建预处理
 * @return 预处理对象
 */
std::shared_ptr<algorithm::PreProcess> Yolov5PostFactory::makePreProcess() {
    return std::static_pointer_cast<algorithm::PreProcess>(std::make_shared<pre_process::Yolov5PostPre>());
}

/**
 * 创建推理
 * @return 推理对象
 */
std::shared_ptr<algorithm::Inference> Yolov5PostFactory::makeInference() {
    return std::static_pointer_cast<algorithm::Inference>(std::make_shared<inference::Yolov5PostInference>());
}

/**
 * 创建后处理
 * @return 后处理对象
 */
std::shared_ptr<algorithm::PostProcess> Yolov5PostFactory::makePostProcess() {
    return std::static_pointer_cast<algorithm::PostProcess>(std::make_shared<post_process::Yolov5PostPost>());
}

//注册sophgo Yolov5Post
REGISTER_ALGORITHM_FACTORY("sophgo", "Yolov5Post", Yolov5PostFactory);

} // namespace factory
} // namespace algorithm
} // namespace sophon_stream
