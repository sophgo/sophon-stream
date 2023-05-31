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

/**
 * network predict output
 * @param[in] context: inputData and outputDat
 */
common::ErrorCode Yolov5Inference::predict(
    std::shared_ptr<Yolov5Context> context,
    common::ObjectMetadatas& objectMetadatas) {
  if (objectMetadatas.size() == 0) return common::ErrorCode::SUCCESS;

  // for(auto& obj : objectMetadatas)
  // {
  //   printf("infer channel_id: %d and frame_id: %d\n", obj->mFrame->mChannelId, obj->mFrame->mFrameId);
  // }


  int ret = context->m_bmNetwork->forward(
      objectMetadatas[0]->mInputBMtensors->tensors,
      objectMetadatas[0]->mOutputBMtensors->tensors);
  return common::ErrorCode::SUCCESS;
}

}  // namespace yolov5
}  // namespace element
}  // namespace sophon_stream