//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#include "posec3d_inference.h"

#include "common/logger.h"

namespace sophon_stream {
namespace element {
namespace posec3d {

Posec3dInference::~Posec3dInference() {}

void Posec3dInference::init(std::shared_ptr<Posec3dContext> context) {}

common::ErrorCode Posec3dInference::predict(
    std::shared_ptr<Posec3dContext> context,
    common::ObjectMetadatas& objectMetadatas) {
  if (objectMetadatas.size() == 0) return common::ErrorCode::SUCCESS;

  objectMetadatas[0]->mOutputBMtensors = getOutputDeviceMem(context);
  int ret = context->bmNetwork->forward(
      objectMetadatas[0]->mInputBMtensors->tensors,
      objectMetadatas[0]->mOutputBMtensors->tensors);

  for (auto obj : objectMetadatas) {
    obj->mInputBMtensors = nullptr;
  }

  return common::ErrorCode::SUCCESS;
}

}  // namespace posec3d
}  // namespace element
}  // namespace sophon_stream