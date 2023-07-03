//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-DEMO is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//
#include "resnet_classify.h"

#include "common/logger.h"

#define USE_ASPECT_RATIO 1
#define DUMP_FILE 0

namespace sophon_stream {
namespace element {
namespace resnet {

ResNetClassify::~ResNetClassify() {}

void ResNetClassify::init(std::shared_ptr<ResNetContext> context) {}

void ResNetClassify::initTensors(std::shared_ptr<ResNetContext> context,
                                 common::ObjectMetadatas& objectMetadatas) {
  for (auto& obj : objectMetadatas) {
    obj->mInputBMtensors = std::make_shared<sophon_stream::common::bmTensors>();
    int channelId = obj->mFrame->mChannelId;
    int frameId = obj->mFrame->mFrameId;
    obj->mInputBMtensors.reset(
        new sophon_stream::common::bmTensors(),
        [channelId, frameId](sophon_stream::common::bmTensors* p) {
          for (int i = 0; i < p->tensors.size(); ++i) {
            if (p->tensors[i]->device_mem.u.device.device_addr != 0) {
              bm_free_device(p->handle, p->tensors[i]->device_mem);
            }
          }

          delete p;
          p = nullptr;
        });
    obj->mInputBMtensors->handle = context->handle;
    obj->mInputBMtensors->tensors.resize(context->input_num);
    for (int i = 0; i < context->input_num; ++i) {
      obj->mInputBMtensors->tensors[i] = std::make_shared<bm_tensor_t>();
      obj->mInputBMtensors->tensors[i]->dtype =
          context->bmNetwork->m_netinfo->input_dtypes[i];
      obj->mInputBMtensors->tensors[i]->shape =
          context->bmNetwork->m_netinfo->stages[0].input_shapes[i];
      obj->mInputBMtensors->tensors[i]->shape.dims[0] = 1;
      obj->mInputBMtensors->tensors[i]->st_mode = BM_STORE_1N;
    }
  }
}

common::ErrorCode ResNetClassify::classify(
    std::shared_ptr<ResNetContext> context,
    common::ObjectMetadatas& objectMetadatas) {
  common::ErrorCode errorCode = common::ErrorCode::SUCCESS;
  // 1. preprocess
  errorCode = pre_process(context, objectMetadatas);
  if (common::ErrorCode::SUCCESS != errorCode) {
    IVS_ERROR("ResNet pre_process error");
    return errorCode;
  }
  // 2. forward
  errorCode = predict(context, objectMetadatas);
  if (common::ErrorCode::SUCCESS != errorCode) {
    IVS_ERROR("ResNet predict error");
    return errorCode;
  }
  // 3. post process
  errorCode = post_process(context, objectMetadatas);
  if (common::ErrorCode::SUCCESS != errorCode) {
    IVS_ERROR("ResNet post_process error");
    return errorCode;
  }

  return errorCode;
}

common::ErrorCode ResNetClassify::pre_process(
    std::shared_ptr<ResNetContext> context,
    common::ObjectMetadatas& objectMetadatas) {
  if (objectMetadatas.size() == 0) return common::ErrorCode::SUCCESS;
  initTensors(context, objectMetadatas);

  auto jsonPlanner = context->bgr2rgb ? FORMAT_RGB_PLANAR : FORMAT_BGR_PLANAR;
  int i = 0;
  for (auto& objMetadata : objectMetadatas) {
    if (objMetadata->mFrame->mSpData == nullptr) continue;
    bm_image resized_img;
    bm_image converto_img;
    bm_image image0 = *objMetadata->mFrame->mSpData;
    bm_image image1;
    // convert to RGB_PLANAR
    if (image0.image_format != jsonPlanner) {
      bm_image_create(context->handle, image0.height, image0.width, jsonPlanner,
                      image0.data_type, &image1);
      bm_image_alloc_dev_mem(image1, BMCV_IMAGE_FOR_IN);
      bmcv_image_storage_convert(context->handle, 1, &image0, &image1);
    } else {
      image1 = image0;
    }

    bm_image image_aligned;
    bool need_copy = image1.width & (64 - 1);
    if (need_copy) {
      int stride1[3], stride2[3];
      bm_image_get_stride(image1, stride1);
      stride2[0] = FFALIGN(stride1[0], 64);
      stride2[1] = FFALIGN(stride1[1], 64);
      stride2[2] = FFALIGN(stride1[2], 64);
      bm_image_create(context->bmContext->handle(), image1.height,
                      image1.width, image1.image_format, image1.data_type,
                      &image_aligned, stride2);

      bm_image_alloc_dev_mem(image_aligned, BMCV_IMAGE_FOR_IN);
      bmcv_copy_to_atrr_t copyToAttr;
      memset(&copyToAttr, 0, sizeof(copyToAttr));
      copyToAttr.start_x = 0;
      copyToAttr.start_y = 0;
      copyToAttr.if_padding = 1;
      bmcv_image_copy_to(context->bmContext->handle(), copyToAttr, image1,
                         image_aligned);
    } else {
      image_aligned = image1;
    }
    // #ifdef USE_ASPECT_RATIO
    bool isAlignWidth = false;
    float ratio =
        get_aspect_scaled_ratio(image0.width, image0.height, context->net_w,
                                context->net_h, &isAlignWidth);
    bmcv_padding_atrr_t padding_attr;
    memset(&padding_attr, 0, sizeof(padding_attr));
    padding_attr.dst_crop_sty = 0;
    padding_attr.dst_crop_stx = 0;
    padding_attr.padding_b = 114;
    padding_attr.padding_g = 114;
    padding_attr.padding_r = 114;
    padding_attr.if_memset = 1;
    if (isAlignWidth) {
      padding_attr.dst_crop_h = image0.height * ratio;
      padding_attr.dst_crop_w = context->net_w;

      int ty1 = (int)((context->net_h - padding_attr.dst_crop_h) / 2);
      padding_attr.dst_crop_sty = ty1;
      padding_attr.dst_crop_stx = 0;
    } else {
      padding_attr.dst_crop_h = context->net_h;
      padding_attr.dst_crop_w = image0.width * ratio;

      int tx1 = (int)((context->net_w - padding_attr.dst_crop_w) / 2);
      padding_attr.dst_crop_sty = 0;
      padding_attr.dst_crop_stx = tx1;
    }

    int aligned_net_w = FFALIGN(context->net_w, 64);
    int strides[3] = {aligned_net_w, aligned_net_w, aligned_net_w};

    bm_image_create(context->handle, context->net_h, context->net_w,
                    jsonPlanner, DATA_TYPE_EXT_1N_BYTE, &resized_img, strides);
    bmcv_rect_t crop_rect{0, 0, image1.width, image1.height};
    bm_status_t ret = BM_SUCCESS;
    if (context->roi_predefined) {
      if (context->roi.start_x > image1.width ||
          context->roi.start_y > image1.height ||
          (context->roi.start_x + context->roi.crop_w) > image1.width ||
          (context->roi.start_y + context->roi.crop_h) > image1.height) {
        IVS_CRITICAL("ROI AREA OUT OF RANGE");
        abort();
      }
      ret = bmcv_image_vpp_convert_padding(context->bmContext->handle(), 1,
                                           image_aligned, &resized_img,
                                           &padding_attr, &context->roi);
    } else {
      ret = bmcv_image_vpp_convert_padding(context->bmContext->handle(), 1,
                                           image_aligned, &resized_img,
                                           &padding_attr, &crop_rect);
    }

    assert(BM_SUCCESS == ret);

    if (image0.image_format != FORMAT_BGR_PLANAR) {
      bm_image_destroy(image1);
    }
    if (need_copy) bm_image_destroy(image_aligned);

    bm_image_data_format_ext img_dtype = DATA_TYPE_EXT_FLOAT32;
    auto tensor = context->bmNetwork->inputTensor(0);
    if (tensor->get_dtype() == BM_INT8) {
      img_dtype = DATA_TYPE_EXT_1N_BYTE_SIGNED;
    }

    bm_image_create(context->handle, context->net_h, context->net_w,
                    jsonPlanner, img_dtype, &converto_img);

    bm_device_mem_t mem;
    int size_byte = 0;
    bm_image_get_byte_size(converto_img, &size_byte);
    ret = bm_malloc_device_byte(context->handle, &mem, size_byte);
    bm_image_attach(converto_img, &mem);

    bmcv_image_convert_to(context->handle, 1, context->converto_attr,
                          &resized_img, &converto_img);

    bm_image_destroy(resized_img);

    bm_image_get_device_mem(
        converto_img,
        &objectMetadatas[i]->mInputBMtensors->tensors[0]->device_mem);
    bm_image_detach(converto_img);
    bm_image_destroy(converto_img);
    i++;
  }
  return common::ErrorCode::SUCCESS;
}

common::ErrorCode ResNetClassify::predict(
    std::shared_ptr<ResNetContext> context,
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

  return common::ErrorCode::SUCCESS;
}

common::ErrorCode ResNetClassify::post_process(
    std::shared_ptr<ResNetContext> context,
    common::ObjectMetadatas& objectMetadatas) {
  if (objectMetadatas.size() == 0) return common::ErrorCode::SUCCESS;
  assert(context->output_num == 1);
  for (auto obj : objectMetadatas) {
    if (obj->mFrame->mEndOfStream) break;
    std::shared_ptr<BMNNTensor> outputTensor = std::make_shared<BMNNTensor>(
        obj->mOutputBMtensors->handle,
        context->bmNetwork->m_netinfo->output_names[context->output_num - 1],
        context->bmNetwork->m_netinfo->output_scales[context->output_num - 1],
        obj->mOutputBMtensors->tensors[context->output_num - 1].get(),
        context->bmNetwork->is_soc);

    float* output_data = (float*)outputTensor->get_cpu_data();
    auto output_scale =
        context->bmNetwork->m_netinfo->output_scales[context->output_num - 1];
    float exp_sum = 0;
    for (int j = 0; j < context->class_num; j++) {
      exp_sum += std::exp(*(output_data + j) * output_scale);
    }
    int max_idx = -1;
    float max_score = -1;
    for (int j = 0; j < context->class_num; j++) {
      float score = 0;
      score = std::exp(*(output_data + j) * output_scale) / exp_sum;
      if (max_score < score) {
        max_score = score;
        max_idx = j;
      }
    }

    std::shared_ptr<common::RecognizedObjectMetadata> RecogObj =
        std::make_shared<common::RecognizedObjectMetadata>();

    RecogObj->mScores.push_back(max_score);
    RecogObj->mTopKLabels.push_back(max_idx);
    obj->mRecognizedObjectMetadatas.push_back(RecogObj);

    // std::string filename = std::to_string(obj->mFrame->mChannelId) +
    //                        "-" +
    //                        std::to_string(obj->mFrame->mFrameId) +
    //                        "-car-" + std::to_string(subId) + ".bmp";
    // bm_image_write_to_bmp(*obj->mFrame->mSpData, filename.c_str());
    // ++ subId;

    IVS_DEBUG("recognizition succeed, frame_id: {0}, class_id: {1}",
              obj->mFrame->mFrameId, max_idx);
  }

  return common::ErrorCode::SUCCESS;
}

float ResNetClassify::get_aspect_scaled_ratio(int src_w, int src_h, int dst_w,
                                              int dst_h, bool* pIsAligWidth) {
  float ratio;
  float r_w = (float)dst_w / src_w;
  float r_h = (float)dst_h / src_h;
  if (r_h > r_w) {
    *pIsAligWidth = true;
    ratio = r_w;
  } else {
    *pIsAligWidth = false;
    ratio = r_h;
  }
  return ratio;
}

std::shared_ptr<sophon_stream::common::bmTensors>
ResNetClassify::mergeInputDeviceMem(std::shared_ptr<ResNetContext> context,
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
ResNetClassify::getOutputDeviceMem(std::shared_ptr<ResNetContext> context) {
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

void ResNetClassify::splitOutputMemIntoObjectMetadatas(
    std::shared_ptr<ResNetContext> context,
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
      assert(BM_SUCCESS == ret);
      bm_memcpy_d2d_byte(
          context->handle,
          objectMetadatas[i]->mOutputBMtensors->tensors[j]->device_mem, 0,
          outputTensors->tensors[j]->device_mem, i * max_size, max_size);
    }
  }
}

}  // namespace resnet
}  // namespace element
}  // namespace sophon_stream
