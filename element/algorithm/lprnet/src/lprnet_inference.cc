//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#include "lprnet_inference.h"

namespace sophon_stream {
namespace element {
namespace lprnet {

LprnetInference::~LprnetInference() {}

void LprnetInference::init(std::shared_ptr<LprnetContext> context) {}

common::ErrorCode LprnetInference::predict(
    std::shared_ptr<LprnetContext> context,
    common::ObjectMetadatas& objectMetadatas) {
  if (objectMetadatas.size() == 0) return common::ErrorCode::SUCCESS;

  if (context->max_batch > 1) {
    auto inputTensors = mergeInputDeviceMem(context, objectMetadatas);
    auto outputTensors = getOutputDeviceMem(context);

    int ret = 0;
    ret = context->bmNetwork->forward(inputTensors->tensors,
                                      outputTensors->tensors);

    splitOutputMemIntoObjectMetadatas(context, objectMetadatas, outputTensors);
  } else {
    objectMetadatas[0]->mOutputBMtensors = getOutputDeviceMem(context);
    int ret = context->bmNetwork->forward(
        objectMetadatas[0]->mInputBMtensors->tensors,
        objectMetadatas[0]->mOutputBMtensors->tensors);
  }

  for (auto obj : objectMetadatas) {
    obj->mInputBMtensors = nullptr;
  }

  return common::ErrorCode::SUCCESS;
}

}  // namespace lprnet
}  // namespace element
}  // namespace sophon_stream