//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#include "template_inference.h"

namespace sophon_stream {
namespace element {
namespace template {

TemplateInference::~TemplateInference() {}

void TemplateInference::init(std::shared_ptr<TemplateContext> context) {}

std::shared_ptr<sophon_stream::common::bmTensors>
TemplateInference::mergeInputDeviceMem(std::shared_ptr<TemplateContext> context,
                                     common::ObjectMetadatas& objectMetadatas) {
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
    STREAM_CHECK(ret == 0, "Alloc Device Memory Failed! Program Terminated.")                                     
    // d2d
    for (int j = 0; j < objectMetadatas.size(); ++j) {
      if (objectMetadatas[j]->mFrame->mEndOfStream) break;
      bm_memcpy_d2d_byte(
          inputTensors->handle, inputTensors->tensors[i]->device_mem,
          j * input_bytes / context->max_batch,
          objectMetadatas[j]->mInputBMtensors->tensors[i]->device_mem, 0,
          input_bytes / context->max_batch);
    }
  }
  return inputTensors;
}

std::shared_ptr<sophon_stream::common::bmTensors>
TemplateInference::getOutputDeviceMem(std::shared_ptr<TemplateContext> context) {
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
    STREAM_CHECK(ret == 0, "Alloc Device Memory Failed! Program Terminated.")                              
  }
  return outputTensors;
}

void TemplateInference::splitOutputMemIntoObjectMetadatas(
    std::shared_ptr<TemplateContext> context,
    common::ObjectMetadatas& objectMetadatas,
    std::shared_ptr<sophon_stream::common::bmTensors> outputTensors) {
  // 把outputTensors的显存拆出来给objectMetadatas
  for (int i = 0; i < objectMetadatas.size(); ++i) {
    if (objectMetadatas[i]->mFrame->mEndOfStream) break;
    objectMetadatas[i]->mOutputBMtensors =
        std::make_shared<sophon_stream::common::bmTensors>();
    objectMetadatas[i]->mOutputBMtensors.reset(
        new sophon_stream::common::bmTensors(),
        [](sophon_stream::common::bmTensors* p) {
          for (int i = 0; i < p->tensors.size(); ++i)
            if (p->tensors[i]->device_mem.u.device.device_addr != 0) {
              bm_free_device(p->handle, p->tensors[i]->device_mem);
            }
          delete p;
          p = nullptr;
        });
    objectMetadatas[i]->mOutputBMtensors->tensors.resize(context->output_num);
    objectMetadatas[i]->mOutputBMtensors->handle = context->handle;
    for (int j = 0; j < context->output_num; ++j) {
      objectMetadatas[i]->mOutputBMtensors->tensors[j] =
          std::make_shared<bm_tensor_t>();
      objectMetadatas[i]->mOutputBMtensors->tensors[j]->dtype =
          context->bmNetwork->m_netinfo->output_dtypes[j];
      objectMetadatas[i]->mOutputBMtensors->tensors[j]->shape =
          context->bmNetwork->m_netinfo->stages[0].output_shapes[j];
      objectMetadatas[i]->mOutputBMtensors->tensors[j]->shape.dims[0] /=
          context->max_batch;
      objectMetadatas[i]->mOutputBMtensors->tensors[j]->st_mode = BM_STORE_1N;
      size_t max_size = 0;
      for (int s = 0; s < context->bmNetwork->m_netinfo->stage_num; s++) {
        size_t out_size = bmrt_shape_count(
            &context->bmNetwork->m_netinfo->stages[s].output_shapes[j]);
        if (max_size < out_size) {
          max_size = out_size;
        }
      }
      if (BM_FLOAT32 == context->bmNetwork->m_netinfo->output_dtypes[j])
        max_size *= 4;
      max_size /= context->max_batch;
      auto ret = bm_malloc_device_byte(
          objectMetadatas[i]->mOutputBMtensors->handle,
          &objectMetadatas[i]->mOutputBMtensors->tensors[j]->device_mem,
          max_size);
      STREAM_CHECK(ret == 0, "Alloc Device Memory Failed! Program Terminated.")
      bm_memcpy_d2d_byte(
          context->handle,
          objectMetadatas[i]->mOutputBMtensors->tensors[j]->device_mem, 0,
          outputTensors->tensors[j]->device_mem, i * max_size, max_size);
    }
  }
}

common::ErrorCode TemplateInference::predict(
    std::shared_ptr<TemplateContext> context,
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

}  // namespace template
}  // namespace element
}  // namespace sophon_stream