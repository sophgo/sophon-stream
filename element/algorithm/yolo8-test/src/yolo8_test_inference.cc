//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#include "yolo8_test_inference.h"

namespace sophon_stream {
namespace element {
namespace yolo8_test {

Yolo8TestInference::~Yolo8TestInference() {}

void Yolo8TestInference::init(std::shared_ptr<Yolo8TestContext> context) {}

common::ErrorCode Yolo8TestInference::predict(
    std::shared_ptr<Yolo8TestContext> context,
    common::ObjectMetadatas& objectMetadatas) {
  if (objectMetadatas.size() == 0) return common::ErrorCode::SUCCESS;

  // batch 模型：先把多帧的输入显存合并成连续 batch 输入，
  // forward 后再把 batch 输出拆回每个 ObjectMetadata。
  if (context->max_batch > 1) {
    auto inputTensors = mergeInputDeviceMem(context, objectMetadatas);
    auto outputTensors = getOutputDeviceMem(context);

    int ret = 0;
    ret = context->bmNetwork->forward(inputTensors->tensors,
                                      outputTensors->tensors);

    splitOutputMemIntoObjectMetadatas(context, objectMetadatas, outputTensors);
  } else {
    // batch=1：直接使用当前帧的输入 tensor 做 forward。
    // EOS 帧只负责传递结束信号，不需要推理。
    if (objectMetadatas[0]->mFrame->mEndOfStream)
      return common::ErrorCode::SUCCESS;
    objectMetadatas[0]->mOutputBMtensors = getOutputDeviceMem(context);
    int ret = context->bmNetwork->forward(
        objectMetadatas[0]->mInputBMtensors->tensors,
        objectMetadatas[0]->mOutputBMtensors->tensors);
  }

  // 推理完成后释放输入 tensor 引用，避免后续阶段继续占用输入显存。
  for (auto obj : objectMetadatas) {
    obj->mInputBMtensors = nullptr;
  }

  return common::ErrorCode::SUCCESS;
}

}  // namespace yolo8_test
}  // namespace element
}  // namespace sophon_stream
