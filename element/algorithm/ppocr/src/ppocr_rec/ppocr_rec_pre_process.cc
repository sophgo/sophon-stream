//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#include "ppocr_rec_pre_process.h"

#include "common/logger.h"

namespace sophon_stream {
namespace element {
namespace ppocr_rec {

void PpocrRecPreProcess::init(std::shared_ptr<PpocrRecContext> context) {}

void PpocrRecPreProcess::initTensors(std::shared_ptr<PpocrRecContext> context,
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

common::ErrorCode PpocrRecPreProcess::preProcess(
    std::shared_ptr<PpocrRecContext> context,
    common::ObjectMetadatas& objectMetadatas) {
  if (objectMetadatas.size() == 0) return common::ErrorCode::SUCCESS;
  initTensors(context, objectMetadatas);

  for (auto& objMetadata : objectMetadatas) {
    if (objMetadata->mFrame->mSpData == nullptr) continue;
    bm_image image1 = *objMetadata->mFrame->mSpData;

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

      auto ret = bm_image_alloc_dev_mem(image_aligned, BMCV_IMAGE_FOR_IN);
      STREAM_CHECK(ret == 0, "Alloc Device Memory Failed! Program Terminated.")
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

    int h = image_aligned.height;
    int w = image_aligned.width;
    float ratio = w / float(h);
    int resize_h;
    int resize_w;
    int padding_w;
    if (ratio > context->img_ratio.back()) {
      resize_h = context->img_size[context->img_size.size() - 1].h;
      resize_w = context->img_size[context->img_size.size() - 1].w;
    } else {
      for (int i = 0; i < context->img_ratio.size(); i++) {
        if (ratio <= context->img_ratio[i]) {
          resize_h = context->img_size[i].h;
          resize_w = (int)(resize_h * ratio);
          padding_w = context->img_size[i].w;
          break;
        }
      }
    }

    // resize + padding
    bmcv_padding_atrr_t padding_attr;
    memset(&padding_attr, 0, sizeof(padding_attr));
    padding_attr.dst_crop_sty = 0;
    padding_attr.dst_crop_stx = 0;
    padding_attr.dst_crop_w = resize_w;
    padding_attr.dst_crop_h = resize_h;
    padding_attr.padding_b = 0;
    padding_attr.padding_g = 0;
    padding_attr.padding_r = 0;
    padding_attr.if_memset = 1;
    bmcv_rect_t crop_rect{0, 0, image_aligned.width, image_aligned.height};

    bm_image resized_img;
    int aligned_net_w = FFALIGN(context->m_net_w, 64);
    int strides[3] = {aligned_net_w, aligned_net_w, aligned_net_w};
    auto ret = bm_image_create(context->bmContext->handle(), context->m_net_h,
                               context->m_net_w, FORMAT_BGR_PLANAR,
                               DATA_TYPE_EXT_1N_BYTE, &resized_img, strides);
    assert(BM_SUCCESS == ret);

    bmcv_image_vpp_convert_padding(context->bmContext->handle(), 1,
                                   image_aligned, &resized_img, &padding_attr,
                                   &crop_rect);

    // converto
    bm_image_data_format_ext img_dtype = DATA_TYPE_EXT_FLOAT32;
    auto tensor = context->bmNetwork->inputTensor(0);
    if (tensor->get_dtype() == BM_INT8) {
      img_dtype = DATA_TYPE_EXT_1N_BYTE_SIGNED;
    }
    bm_image converto_img;
    bm_image_create(context->bmNetwork->m_handle, context->m_net_h,
                    context->m_net_w, FORMAT_BGR_PLANAR, img_dtype,
                    &converto_img);
    bm_device_mem_t input_dev_mem;
    int size_byte = 0;
    bm_image_get_byte_size(converto_img, &size_byte);
    ret = bm_malloc_device_byte(context->bmContext->handle(), &input_dev_mem,
                                size_byte);
    STREAM_CHECK(ret == 0, "Alloc Device Memory Failed! Program Terminated.")                                
    bm_image_attach(converto_img, &input_dev_mem);
    bmcv_image_convert_to(context->bmContext->handle(), 1,
                          context->converto_attr, &resized_img, &converto_img);
    bm_image_get_contiguous_device_mem(1, &converto_img, &input_dev_mem);

    bm_image_destroy(resized_img);

    bm_image_get_device_mem(
        converto_img, &objMetadata->mInputBMtensors->tensors[0]->device_mem);

    bm_image_detach(converto_img);
    bm_image_destroy(converto_img);
    if (need_copy) bm_image_destroy(image_aligned);
  }

  return common::ErrorCode::SUCCESS;
}

}  // namespace ppocr_rec
}  // namespace element
}  // namespace sophon_stream