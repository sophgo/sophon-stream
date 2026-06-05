//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#include "ppyoloe_plus_inference.h"

namespace sophon_stream {
namespace element {
namespace ppyoloe_plus {

Ppyoloe_plusInference::~Ppyoloe_plusInference() {}

void Ppyoloe_plusInference::init(std::shared_ptr<Ppyoloe_plusContext> context) {}

common::ErrorCode Ppyoloe_plusInference::predict(
    std::shared_ptr<Ppyoloe_plusContext> context,
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

  for(auto obj : objectMetadatas) {
    obj->mInputBMtensors = nullptr;
  }

  return common::ErrorCode::SUCCESS;
}

}  // namespace ppyoloe_plus
}  // namespace element
}  // namespace sophon_stream