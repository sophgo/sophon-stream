//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#include "yolox_inference.h"

namespace sophon_stream {
namespace element {
namespace yolox {




YoloxInference::~YoloxInference() {}

void YoloxInference::init(std::shared_ptr<YoloxContext> context) {}

common::ErrorCode YoloxInference::predict(
    std::shared_ptr<YoloxContext> context,
    common::ObjectMetadatas& objectMetadatas) {
  if (objectMetadatas.size() == 0) return common::ErrorCode::SUCCESS;
  int ret = 0;
  ret = context->bmNetwork->forward(
      objectMetadatas[0]->mInputBMtensors->tensors,
      objectMetadatas[0]->mOutputBMtensors->tensors);
  return common::ErrorCode::SUCCESS;
}

}  // namespace yolox
}  // namespace element
}  // namespace sophon_stream