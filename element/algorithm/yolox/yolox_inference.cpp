//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#include "yolox_inference.h"

#include <fstream>

namespace sophon_stream {
namespace element {
namespace yolox {

YoloxInference::~YoloxInference() {}

common::ErrorCode YoloxInference::init(std::shared_ptr<YoloxContext> context) {
  float confThresh;
  float nmsThresh;
  std::string coco_names_file;
  context->m_thresh = context->threthold;
  context->m_class_num = context->numClass;

  // 1. get network
  BMNNHandlePtr handle = std::make_shared<BMNNHandle>(context->deviceId);
  context->m_bmContext =
      std::make_shared<BMNNContext>(handle, context->modelPath[0].c_str());
  context->m_bmNetwork = context->m_bmContext->network(0);
  context->handle = handle->handle();

  // 2. get input
  context->max_batch = context->m_bmNetwork->maxBatch();
  auto tensor = context->m_bmNetwork->inputTensor(0);
  context->input_num = context->m_bmNetwork->m_netinfo->input_num;
  context->m_net_channel = tensor->get_shape()->dims[1];
  context->m_net_h = tensor->get_shape()->dims[2];
  context->m_net_w = tensor->get_shape()->dims[3];

  // 3. get output
  context->output_num = context->m_bmNetwork->outputTensorNum();
  assert(context->output_num > 0);
  context->min_dim =
      context->m_bmNetwork->outputTensor(0)->get_shape()->num_dims;

  // 4. initialize bmimages
  context->m_resized_imgs.resize(context->max_batch);
  context->m_converto_imgs.resize(context->max_batch);
  // some API only accept bm_image whose stride is aligned to 64
  int aligned_net_w = FFALIGN(context->m_net_w, 64);
  int strides[3] = {aligned_net_w, aligned_net_w, aligned_net_w};
  for (int i = 0; i < context->max_batch; i++) {
    auto ret = bm_image_create(context->m_bmContext->handle(), context->m_net_h,
                               context->m_net_w, FORMAT_RGB_PLANAR,
                               DATA_TYPE_EXT_1N_BYTE,
                               &context->m_resized_imgs[i], strides);
    assert(BM_SUCCESS == ret);
  }
  bm_image_alloc_contiguous_mem(context->max_batch,
                                context->m_resized_imgs.data());
  bm_image_data_format_ext img_dtype = DATA_TYPE_EXT_FLOAT32;
  if (tensor->get_dtype() == BM_INT8) {
    img_dtype = DATA_TYPE_EXT_1N_BYTE_SIGNED;
  }
  // auto ret = bm_image_create_batch(pSophgoContext->m_bmContext->handle(),
  // pSophgoContext->m_net_h, pSophgoContext->m_net_w, FORMAT_RGB_PLANAR,
  // img_dtype, pSophgoContext->m_converto_imgs.data(),
  // pSophgoContext->max_batch); assert(BM_SUCCESS == ret);

  // 5.converto
  float input_scale = tensor->get_scale();
  // yolox原始模型输入是0-255,scale=1.0意味着不需要做缩放
  // input_scale /= 255;
  context->converto_attr.alpha_0 = input_scale;
  context->converto_attr.beta_0 = 0;
  context->converto_attr.alpha_1 = input_scale;
  context->converto_attr.beta_1 = 0;
  context->converto_attr.alpha_2 = input_scale;
  context->converto_attr.beta_2 = 0;

  return common::ErrorCode::SUCCESS;
}

common::ErrorCode YoloxInference::predict(
    std::shared_ptr<YoloxContext> context,
    common::ObjectMetadatas& objectMetadatas) {
  if (objectMetadatas.size() == 0) return common::ErrorCode::SUCCESS;
  int ret = 0;
  if (!objectMetadatas[0]->mFrame->mEndOfStream)
    ret = context->m_bmNetwork->forward(
        objectMetadatas[0]->mInputBMtensors->tensors,
        objectMetadatas[0]->mOutputBMtensors->tensors);
  return static_cast<common::ErrorCode>(ret);
}

void YoloxInference::uninit() {}

}  // namespace yolox
}  // namespace element
}  // namespace sophon_stream