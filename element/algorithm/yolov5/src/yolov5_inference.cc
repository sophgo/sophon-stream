//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#include "yolov5_inference.h"

namespace sophon_stream {
namespace element {
namespace yolov5 {

Yolov5Inference::~Yolov5Inference() {}

void Yolov5Inference::init(std::shared_ptr<Yolov5Context> context) {}

common::ErrorCode Yolov5Inference::predict(
    std::shared_ptr<Yolov5Context> context,
    common::ObjectMetadatas& objectMetadatas) {
  if (objectMetadatas.size() == 0) return common::ErrorCode::SUCCESS;

  if (context->max_batch > 1) {
    auto inputTensors = mergeInputDeviceMem(context, objectMetadatas);
    auto outputTensors = getOutputDeviceMem(context);

    int ret = 0;
#if BMCV_VERSION_MAJOR > 1
    ret = context->bmNetwork->forward<false>(inputTensors->tensors,
                                             outputTensors->tensors);
#else
    ret = context->bmNetwork->forward(inputTensors->tensors,
                                      outputTensors->tensors);
#endif

    splitOutputMemIntoObjectMetadatas(context, objectMetadatas, outputTensors);
  } else {
    if (objectMetadatas[0]->mFrame->mEndOfStream)
      return common::ErrorCode::SUCCESS;
    objectMetadatas[0]->mOutputBMtensors = getOutputDeviceMem(context);
#if BMCV_VERSION_MAJOR > 1
    int ret = context->bmNetwork->forward<false>(
        objectMetadatas[0]->mInputBMtensors->tensors,
        objectMetadatas[0]->mOutputBMtensors->tensors);
#else
    int ret = context->bmNetwork->forward(
        objectMetadatas[0]->mInputBMtensors->tensors,
        objectMetadatas[0]->mOutputBMtensors->tensors);
#endif
  }

  for (auto obj : objectMetadatas) {
    obj->mInputBMtensors = nullptr;
  }

  return common::ErrorCode::SUCCESS;
}

}  // namespace yolov5
}  // namespace element
}  // namespace sophon_stream