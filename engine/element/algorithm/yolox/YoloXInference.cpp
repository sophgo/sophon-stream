#include "YoloXInference.h"

#include <fstream>

namespace sophon_stream {
namespace element {
namespace yolox {

YoloXInference::~YoloXInference() {}

common::ErrorCode YoloXInference::init(YoloXSophgoContext& context) {
  YoloXSophgoContext* pSophgoContext = &context;

  float confThresh;
  float nmsThresh;
  std::string coco_names_file;
  pSophgoContext->m_thresh =
      context.threthold;  // thresh[0] confThresh, thresh[1] nmsThresh
  pSophgoContext->m_class_num = context.numClass;
  // pSophgoContext->m_confThreshold= confThresh;
  // pSophgoContext->m_nmsThreshold = nmsThresh;

  // 1. get network
  BMNNHandlePtr handle = std::make_shared<BMNNHandle>(pSophgoContext->deviceId);
  pSophgoContext->m_bmContext = std::make_shared<BMNNContext>(
      handle, pSophgoContext->modelPath[0].c_str());
  pSophgoContext->m_bmNetwork = pSophgoContext->m_bmContext->network(0);
  pSophgoContext->handle = handle->handle();

  // 2. get input
  pSophgoContext->max_batch = pSophgoContext->m_bmNetwork->maxBatch();
  auto tensor = pSophgoContext->m_bmNetwork->inputTensor(0);
  pSophgoContext->input_num = pSophgoContext->m_bmNetwork->m_netinfo->input_num;
  pSophgoContext->m_net_channel = tensor->get_shape()->dims[1];
  pSophgoContext->m_net_h = tensor->get_shape()->dims[2];
  pSophgoContext->m_net_w = tensor->get_shape()->dims[3];

  // 3. get output
  pSophgoContext->output_num = pSophgoContext->m_bmNetwork->outputTensorNum();
  assert(pSophgoContext->output_num > 0);
  pSophgoContext->min_dim =
      pSophgoContext->m_bmNetwork->outputTensor(0)->get_shape()->num_dims;

  // 4. initialize bmimages
  pSophgoContext->m_resized_imgs.resize(pSophgoContext->max_batch);
  pSophgoContext->m_converto_imgs.resize(pSophgoContext->max_batch);
  // some API only accept bm_image whose stride is aligned to 64
  int aligned_net_w = FFALIGN(pSophgoContext->m_net_w, 64);
  int strides[3] = {aligned_net_w, aligned_net_w, aligned_net_w};
  for (int i = 0; i < pSophgoContext->max_batch; i++) {
    auto ret = bm_image_create(pSophgoContext->m_bmContext->handle(),
                               pSophgoContext->m_net_h, pSophgoContext->m_net_w,
                               FORMAT_RGB_PLANAR, DATA_TYPE_EXT_1N_BYTE,
                               &pSophgoContext->m_resized_imgs[i], strides);
    assert(BM_SUCCESS == ret);
  }
  bm_image_alloc_contiguous_mem(pSophgoContext->max_batch,
                                pSophgoContext->m_resized_imgs.data());
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
  pSophgoContext->converto_attr.alpha_0 = input_scale;
  pSophgoContext->converto_attr.beta_0 = 0;
  pSophgoContext->converto_attr.alpha_1 = input_scale;
  pSophgoContext->converto_attr.beta_1 = 0;
  pSophgoContext->converto_attr.alpha_2 = input_scale;
  pSophgoContext->converto_attr.beta_2 = 0;

  return common::ErrorCode::SUCCESS;
}

common::ErrorCode YoloXInference::predict(
    YoloXSophgoContext& context, common::ObjectMetadatas& objectMetadatas) {
  YoloXSophgoContext* pSophgoContext = &context;
  if (objectMetadatas.size() == 0) return common::ErrorCode::SUCCESS;
  int ret = 0;
  if (!objectMetadatas[0]->mFrame->mEndOfStream)
    ret = pSophgoContext->m_bmNetwork->forward(
        objectMetadatas[0]->mInputBMtensors->tensors,
        objectMetadatas[0]->mOutputBMtensors->tensors);
  return static_cast<common::ErrorCode>(ret);
}

void YoloXInference::uninit() {}

}  // namespace yolox
}  // namespace element
}  // namespace sophon_stream