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

std::shared_ptr<sophon_stream::common::bmTensors>
Posec3dInference::getOutputDeviceMem(std::shared_ptr<Posec3dContext> context) {
  std::shared_ptr<sophon_stream::common::bmTensors> outputTensors =
      std::make_shared<sophon_stream::common::bmTensors>();
  outputTensors.reset(
      new sophon_stream::common::bmTensors(),
      [](sophon_stream::common::bmTensors* p) {
        for (int i = 0; i < p->tensors.size(); ++i)
          if (p->tensors[i]->device_mem.u.device.device_addr != 0) {
            bm_free_device(p->handle, p->tensors[i]->device_mem);
          }
        delete p;
        p = nullptr;
      });
  outputTensors->handle = context->handle;
  outputTensors->tensors.resize(context->output_num);
  for (int i = 0; i < context->output_num; ++i) {
    outputTensors->tensors[i] = std::make_shared<bm_tensor_t>();
    outputTensors->tensors[i]->dtype =
        context->bmNetwork->m_netinfo->output_dtypes[i];
    outputTensors->tensors[i]->shape =
        context->bmNetwork->m_netinfo->stages[0].output_shapes[i];
    outputTensors->tensors[i]->st_mode = BM_STORE_1N;
    // 计算大小
    size_t max_size = 0;
    for (int s = 0; s < context->bmNetwork->m_netinfo->stage_num; s++) {
      size_t out_size = bmrt_shape_count(
          &context->bmNetwork->m_netinfo->stages[s].output_shapes[i]);
      if (max_size < out_size) {
        max_size = out_size;
      }
    }
    if (BM_FLOAT32 == context->bmNetwork->m_netinfo->output_dtypes[i])
      max_size *= 4;
    // malloc空间
    auto ret =
        bm_malloc_device_byte(outputTensors->handle,
                              &outputTensors->tensors[i]->device_mem, max_size);
  }
  return outputTensors;
}

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