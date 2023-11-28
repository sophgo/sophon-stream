//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#include "fastpose_inference.h"

namespace sophon_stream {
namespace element {
namespace fastpose {

FastposeInference::~FastposeInference() {}

void FastposeInference::init(std::shared_ptr<FastposeContext> context) {}

std::shared_ptr<sophon_stream::common::bmSubTensors>
FastposeInference::mergeInputDeviceMem(
    std::shared_ptr<FastposeContext> context,
    common::ObjectMetadatas& objectMetadatas) {
  // 合并inputBMtensors，并且申请连续的outputBMtensors
  std::shared_ptr<sophon_stream::common::bmSubTensors> inputTensors =
      std::make_shared<sophon_stream::common::bmSubTensors>();
  inputTensors.reset(
      new sophon_stream::common::bmSubTensors(),
      [](sophon_stream::common::bmSubTensors* p) {
        for (int i = 0; i < p->tensors.size(); ++i)
          for (int j = 0; j < p->tensors[i].size(); ++j)
            if (p->tensors[i][j]->device_mem.u.device.device_addr != 0) {
              bm_free_device(p->handle, p->tensors[i][j]->device_mem);
            }
        delete p;
        p = nullptr;
      });
  inputTensors->handle = context->handle;

  int merge_sample_num = 0;
  for (int i = 0; i < objectMetadatas.size(); i++) {
    merge_sample_num += objectMetadatas[i]->mSubInputBMtensors->tensors.size();
  }
  int merge_batch_num = merge_sample_num % context->max_batch == 0
                            ? merge_sample_num / context->max_batch
                            : merge_sample_num / context->max_batch + 1;
  inputTensors->tensors.resize(merge_batch_num);

  int objectMetadataId = 0, subObjId = 0;
  for (int i = 0; i < merge_batch_num; i++) {
    inputTensors->tensors[i].resize(context->input_num);
    inputTensors->tensors[i][0] = std::make_shared<bm_tensor_t>();
    inputTensors->tensors[i][0]->dtype =
        context->bmNetwork->m_netinfo->input_dtypes[0];
    inputTensors->tensors[i][0]->shape =
        context->bmNetwork->m_netinfo->stages[0].input_shapes[0];
    inputTensors->tensors[i][0]->st_mode = BM_STORE_1N;
    // 计算大小
    int input_bytes = context->max_batch *
                      inputTensors->tensors[i][0]->shape.dims[1] *
                      context->m_net_h * context->m_net_w;
    if (BM_FLOAT32 == context->bmNetwork->m_netinfo->input_dtypes[0])
      input_bytes *= 4;
    // malloc空间
    auto ret = bm_malloc_device_byte(inputTensors->handle,
                                     &inputTensors->tensors[i][0]->device_mem,
                                     input_bytes);

    // d2d
    for (int j = 0; j < context->max_batch; ++j) {
      if (objectMetadatas[objectMetadataId]->mFrame->mEndOfStream) break;
      bool finish = false;
      while (objectMetadatas[objectMetadataId]
                 ->mSubInputBMtensors->tensors.size() <= subObjId) {
        objectMetadataId++;
        subObjId = 0;
        if (objectMetadataId >= objectMetadatas.size()) {
          for (int z = j; z < context->max_batch; ++z)
            bm_memcpy_d2d_byte(inputTensors->handle,
                               inputTensors->tensors[i][0]->device_mem,
                               z * input_bytes / context->max_batch,
                               inputTensors->tensors[i][0]->device_mem,
                               (z - 1) * input_bytes / context->max_batch,
                               input_bytes / context->max_batch);
          finish = true;
          break;
        }
      }
      if (finish) break;
      bm_memcpy_d2d_byte(inputTensors->handle,
                         inputTensors->tensors[i][0]->device_mem,
                         j * input_bytes / context->max_batch,
                         objectMetadatas[objectMetadataId]
                             ->mSubInputBMtensors->tensors[subObjId][0]
                             ->device_mem,
                         0, input_bytes / context->max_batch);
      subObjId++;
    }
  }
  return inputTensors;
}

std::shared_ptr<sophon_stream::common::bmSubTensors>
FastposeInference::getOutputDeviceMem(
    std::shared_ptr<FastposeContext> context,
    common::ObjectMetadatas& objectMetadatas) {
  std::shared_ptr<sophon_stream::common::bmSubTensors> outputTensors =
      std::make_shared<sophon_stream::common::bmSubTensors>();
  outputTensors.reset(
      new sophon_stream::common::bmSubTensors(),
      [](sophon_stream::common::bmSubTensors* p) {
        for (int i = 0; i < p->tensors.size(); ++i)
          for (int j = 0; j < p->tensors[i].size(); j++)
            if (p->tensors[i][j]->device_mem.u.device.device_addr != 0) {
              bm_free_device(p->handle, p->tensors[i][j]->device_mem);
            }
        delete p;
        p = nullptr;
      });
  outputTensors->handle = context->handle;

  int merge_sample_num = 0;
  for (int i = 0; i < objectMetadatas.size(); i++) {
    merge_sample_num += objectMetadatas[i]->mSubInputBMtensors->tensors.size();
  }
  int merge_batch_num = merge_sample_num % context->max_batch == 0
                            ? merge_sample_num / context->max_batch
                            : merge_sample_num / context->max_batch + 1;
  outputTensors->tensors.resize(merge_batch_num);

  for (int i = 0; i < merge_batch_num; i++) {
    outputTensors->tensors[i].resize(context->output_num);
    outputTensors->tensors[i][0] = std::make_shared<bm_tensor_t>();
    outputTensors->tensors[i][0]->dtype =
        context->bmNetwork->m_netinfo->output_dtypes[0];
    outputTensors->tensors[i][0]->shape =
        context->bmNetwork->m_netinfo->stages[0].output_shapes[0];
    outputTensors->tensors[i][0]->st_mode = BM_STORE_1N;
    // 计算大小
    size_t max_size = 0;
    for (int s = 0; s < context->bmNetwork->m_netinfo->stage_num; s++) {
      size_t out_size = bmrt_shape_count(
          &context->bmNetwork->m_netinfo->stages[s].output_shapes[0]);
      if (max_size < out_size) {
        max_size = out_size;
      }
    }
    if (BM_FLOAT32 == context->bmNetwork->m_netinfo->output_dtypes[0])
      max_size *= 4;
    // malloc空间
    auto ret = bm_malloc_device_byte(outputTensors->handle,
                                     &outputTensors->tensors[i][0]->device_mem,
                                     max_size);
  }
  return outputTensors;
}

void FastposeInference::splitOutputMemIntoObjectMetadatas(
    std::shared_ptr<FastposeContext> context,
    common::ObjectMetadatas& objectMetadatas,
    std::shared_ptr<sophon_stream::common::bmSubTensors>& outputTensors) {
  // 把outputTensors的显存拆出来给objectMetadatas
  int merge_batch_indx = 0, sample_indx = 0;
  for (int i = 0; i < objectMetadatas.size(); ++i) {
    if (objectMetadatas[i]->mFrame->mEndOfStream) break;
    objectMetadatas[i]->mSubOutputBMtensors =
        std::make_shared<sophon_stream::common::bmSubTensors>();
    objectMetadatas[i]->mSubOutputBMtensors.reset(
        new sophon_stream::common::bmSubTensors(),
        [](sophon_stream::common::bmSubTensors* p) {
          for (int i = 0; i < p->tensors.size(); ++i)
            for (int j = 0; j < p->tensors[i].size(); j++)
              if (p->tensors[i][j]->device_mem.u.device.device_addr != 0) {
                bm_free_device(p->handle, p->tensors[i][j]->device_mem);
              }
          delete p;
          p = nullptr;
        });

    objectMetadatas[i]->mSubOutputBMtensors->tensors.resize(
        objectMetadatas[i]->mDetectedObjectMetadatas.size());
    for (int j = 0; j < objectMetadatas[i]->mDetectedObjectMetadatas.size();
         j++) {
      objectMetadatas[i]->mSubOutputBMtensors->tensors[j].resize(
          context->output_num);
      objectMetadatas[i]->mSubOutputBMtensors->handle = context->handle;
      for (int z = 0; z < context->output_num; ++z) {
        objectMetadatas[i]->mSubOutputBMtensors->tensors[j][z] =
            std::make_shared<bm_tensor_t>();
        objectMetadatas[i]->mSubOutputBMtensors->tensors[j][z]->dtype =
            context->bmNetwork->m_netinfo->output_dtypes[z];
        objectMetadatas[i]->mSubOutputBMtensors->tensors[j][z]->shape =
            context->bmNetwork->m_netinfo->stages[0].output_shapes[z];
        objectMetadatas[i]->mSubOutputBMtensors->tensors[j][z]->shape.dims[0] /=
            context->max_batch;
        objectMetadatas[i]->mSubOutputBMtensors->tensors[j][z]->st_mode =
            BM_STORE_1N;
        size_t max_size = 0;
        for (int s = 0; s < context->bmNetwork->m_netinfo->stage_num; s++) {
          size_t out_size = bmrt_shape_count(
              &context->bmNetwork->m_netinfo->stages[s].output_shapes[z]);
          if (max_size < out_size) {
            max_size = out_size;
          }
        }
        if (BM_FLOAT32 == context->bmNetwork->m_netinfo->output_dtypes[z])
          max_size *= 4;
        max_size /= context->max_batch;
        auto ret = bm_malloc_device_byte(
            objectMetadatas[i]->mSubOutputBMtensors->handle,
            &objectMetadatas[i]->mSubOutputBMtensors->tensors[j][z]->device_mem,
            max_size);
        assert(BM_SUCCESS == ret);
        bm_memcpy_d2d_byte(
            context->handle,
            objectMetadatas[i]->mSubOutputBMtensors->tensors[j][z]->device_mem,
            0, outputTensors->tensors[merge_batch_indx][z]->device_mem,
            sample_indx * max_size, max_size);
        sample_indx++;
        if (sample_indx == context->max_batch) {
          sample_indx = 0;
          merge_batch_indx++;
        }
      }
    }
  }
}

common::ErrorCode FastposeInference::predict(
    std::shared_ptr<FastposeContext> context,
    common::ObjectMetadatas& objectMetadatas) {
  if (objectMetadatas.size() == 0) return common::ErrorCode::SUCCESS;

  if (context->max_batch > 1) {
    auto inputTensors = mergeInputDeviceMem(context, objectMetadatas);
    auto outputTensors = getOutputDeviceMem(context, objectMetadatas);

    for (int i = 0; i < inputTensors->tensors.size(); i++) {
      int ret = 0;
      ret = context->bmNetwork->forward(inputTensors->tensors[i],
                                        outputTensors->tensors[i]);
    }

    splitOutputMemIntoObjectMetadatas(context, objectMetadatas, outputTensors);
  } else {
    objectMetadatas[0]->mSubOutputBMtensors =
        getOutputDeviceMem(context, objectMetadatas);
    for (int i = 0; i < objectMetadatas[0]->mSubOutputBMtensors->tensors.size();
         i++) {
      int ret = context->bmNetwork->forward(
          objectMetadatas[0]->mSubInputBMtensors->tensors[i],
          objectMetadatas[0]->mSubOutputBMtensors->tensors[i]);
    }
  }

  for(auto obj : objectMetadatas) {
    obj->mSubInputBMtensors = nullptr;
  }

  return common::ErrorCode::SUCCESS;
}

}  // namespace fastpose
}  // namespace element
}  // namespace sophon_stream