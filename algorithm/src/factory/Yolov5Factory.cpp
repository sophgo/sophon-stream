/**********************************************

 Copyright (c) 2018 LynxiTech Inc - All rights reserved.

 NOTICE: All information contained here is, and remains
 the property of LynxiTech Incorporation. This file can not
 be copied or distributed without permission of LynxiTech Inc.

 Author: written by chao.tang, 2020-03-11

**************************************************/

#include "Yolov5Factory.h"

#include "../AlgorithmFactorySelector.h"
#include "../inference/Yolov5Inference.h"
#include "../post_process/Yolov5Post.h"
#include "../pre_process/Yolov5Pre.h"
#include "../context/SophgoContext.h"


namespace sophon_stream {
namespace algorithm {
namespace factory {

/**
 * 创建context
 * @return context对象
 */
std::shared_ptr<algorithm::Context> Yolov5Factory::makeContext() {
    return std::static_pointer_cast<algorithm::Context>(std::make_shared<context::SophgoContext>());
}

/**
 * 创建预处理
 * @return 预处理对象
 */
std::shared_ptr<algorithm::PreProcess> Yolov5Factory::makePreProcess() {
    return std::static_pointer_cast<algorithm::PreProcess>(std::make_shared<pre_process::Yolov5Pre>());
}

/**
 * 创建推理
 * @return 推理对象
 */
std::shared_ptr<algorithm::Inference> Yolov5Factory::makeInference() {
    return std::static_pointer_cast<algorithm::Inference>(std::make_shared<inference::Yolov5Inference>());
}

/**
 * 创建后处理
 * @return 后处理对象
 */
std::shared_ptr<algorithm::PostProcess> Yolov5Factory::makePostProcess() {
    return std::static_pointer_cast<algorithm::PostProcess>(std::make_shared<post_process::Yolov5Post>());
}

//注册lyn YoloV3
REGISTER_ALGORITHM_FACTORY("sophogo", "Yolov5", Yolov5Factory);

} // namespace factory
} // namespace algorithm
} // namespace sophon_stream
