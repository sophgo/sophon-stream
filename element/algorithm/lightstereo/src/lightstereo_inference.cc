//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#include "lightstereo_inference.h"

namespace sophon_stream {
namespace element {
namespace lightstereo {

LightstereoInference::~LightstereoInference() {}

void LightstereoInference::init(std::shared_ptr<LightstereoContext> context) {}

std::shared_ptr<sophon_stream::common::bmTensors>
LightstereoInference::mergeInputDeviceMem(std::shared_ptr<LightstereoContext> context,
                                     common::ObjectMetadatas& leftObjectMetadatas,
                                     common::ObjectMetadatas& rightObjectMetadatas) {
  // 合并inputBMtensors，并且申请连续的outputBMtensors
  std::shared_ptr<sophon_stream::common::bmTensors> inputTensors =
      std::make_shared<sophon_stream::common::bmTensors>();
  inputTensors.reset(
      new sophon_stream::common::bmTensors(),
      [](sophon_stream::common::bmTensors* p) {
        for (int i = 0; i < p->tensors.size(); ++i)
          if (p->tensors[i]->device_mem.u.device.device_addr != 0) {
            bm_free_device(p->handle, p->tensors[i]->device_mem);
          }
        delete p;
        p = nullptr;
      });
  inputTensors->handle = context->handle;
  inputTensors->tensors.resize(context->input_num);
  for (int i = 0; i < context->input_num; ++i) {
    inputTensors->tensors[i] = std::make_shared<bm_tensor_t>();
    inputTensors->tensors[i]->dtype =
        context->bmNetwork->m_netinfo->input_dtypes[i];
    inputTensors->tensors[i]->shape =
        context->bmNetwork->m_netinfo->stages[0].input_shapes[i];
    inputTensors->tensors[i]->st_mode = BM_STORE_1N;
    // 计算大小
    int input_bytes = context->max_batch *
                      inputTensors->tensors[i]->shape.dims[1] * context->net_h *
                      context->net_w;
    if (BM_FLOAT32 == context->bmNetwork->m_netinfo->input_dtypes[0])
      input_bytes *= 4;
    // malloc空间
    auto ret = bm_malloc_device_byte(inputTensors->handle,
                                     &inputTensors->tensors[i]->device_mem,
                                     input_bytes);
    auto objectMetadatas = i == 0 ? leftObjectMetadatas : rightObjectMetadatas;
    // d2d
    for (int j = 0; j < objectMetadatas.size(); ++j) {
      if (objectMetadatas[j]->mFrame->mEndOfStream) break;
      bm_memcpy_d2d_byte(
          inputTensors->handle, inputTensors->tensors[i]->device_mem,
          j * input_bytes / context->max_batch,
          objectMetadatas[j]->mInputBMtensors->tensors[0]->device_mem, 0,
          input_bytes / context->max_batch);
    }
  }

  return inputTensors;
}

common::ErrorCode LightstereoInference::predict(
    std::shared_ptr<LightstereoContext> context,
    common::ObjectMetadatas& leftObjectMetadatas,
    common::ObjectMetadatas& rightObjectMetadatas,
    common::ObjectMetadatas& outputObjectMetadatas) {
  if (leftObjectMetadatas.size() == 0 || rightObjectMetadatas.size() == 0) return common::ErrorCode::SUCCESS;

  if (context->max_batch > 1) {
    auto inputTensors = mergeInputDeviceMem(context, leftObjectMetadatas, rightObjectMetadatas);
    auto outputTensors = getOutputDeviceMem(context);

    int ret = 0;
    ret = context->bmNetwork->forward(inputTensors->tensors,
                                      outputTensors->tensors);

    splitOutputMemIntoObjectMetadatas(context, outputObjectMetadatas, outputTensors);
  } else {
    std::vector<std::shared_ptr<bm_tensor_t>> input_tensors = {
      leftObjectMetadatas[0]->mInputBMtensors->tensors[0],
      rightObjectMetadatas[0]->mInputBMtensors->tensors[0]
    };
    outputObjectMetadatas[0]->mOutputBMtensors = getOutputDeviceMem(context);
    int ret = context->bmNetwork->forward(
        input_tensors,
        outputObjectMetadatas[0]->mOutputBMtensors->tensors);
  }

  for(auto obj : outputObjectMetadatas) {
    obj->mInputBMtensors = nullptr;
  }

  return common::ErrorCode::SUCCESS;
}

}  // namespace lightstereo
}  // namespace element
}  // namespace sophon_stream