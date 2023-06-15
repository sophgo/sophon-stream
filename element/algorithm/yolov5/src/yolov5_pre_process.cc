//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#include "yolov5_pre_process.h"

#include "common/logger.h"

namespace sophon_stream {
namespace element {
namespace yolov5 {

float Yolov5PreProcess::get_aspect_scaled_ratio(int src_w, int src_h, int dst_w,
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

void Yolov5PreProcess::init(std::shared_ptr<Yolov5Context> context) {}

void Yolov5PreProcess::initTensors(std::shared_ptr<Yolov5Context> context,
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

common::ErrorCode Yolov5PreProcess::preProcess(
    std::shared_ptr<Yolov5Context> context,
    common::ObjectMetadatas& objectMetadatas) {
  if (objectMetadatas.size() == 0) return common::ErrorCode::SUCCESS;
  initTensors(context, objectMetadatas);

  int i = 0;
  for (auto& objMetadata : objectMetadatas) {
    if (objMetadata->mFrame->mSpData == nullptr) continue;
    bm_image resized_img;
    bm_image converto_img;
    bm_image image0 = *objMetadata->mFrame->mSpData;
    bm_image image1;
    // convert to RGB_PLANAR
    if (image0.image_format != FORMAT_BGR_PLANAR) {
      bm_image_create(context->handle, image0.height, image0.width,
                      FORMAT_BGR_PLANAR, image0.data_type, &image1);
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
      bm_image_create(context->bmContext->handle(), image1.height, image1.width,
                      image1.image_format, image1.data_type, &image_aligned,
                      stride2);

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
    // for (int i = 0; i < context->max_batch; i++) {
    bm_image_create(context->handle, context->net_h, context->net_w,
                    FORMAT_BGR_PLANAR, DATA_TYPE_EXT_1N_BYTE, &resized_img,
                    strides);
    bmcv_rect_t crop_rect{0, 0, image1.width, image1.height};
    auto ret = bmcv_image_vpp_convert_padding(context->bmContext->handle(), 1,
                                              image_aligned, &resized_img,
                                              &padding_attr, &crop_rect);
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

    if (context->bgr2rgb)
      bm_image_create(context->handle, context->net_h, context->net_w,
                      FORMAT_RGB_PLANAR, img_dtype, &converto_img);
    else
      bm_image_create(context->handle, context->net_h, context->net_w,
                      FORMAT_BGR_PLANAR, img_dtype,
                      &converto_img);  // TODO, 无需调用bmcv_image_convert_to

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

}  // namespace yolov5
}  // namespace element
}  // namespace sophon_stream