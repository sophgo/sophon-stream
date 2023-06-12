//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#include "yolox_pre_process.h"

#include "common/logger.h"

namespace sophon_stream {
namespace element {
namespace yolox {

void YoloxPreProcess::init(std::shared_ptr<YoloxContext> context) {}

void YoloxPreProcess::initTensors(std::shared_ptr<YoloxContext> context,
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

common::ErrorCode YoloxPreProcess::preProcess(
    std::shared_ptr<YoloxContext> context,
    common::ObjectMetadatas& objectMetadatas) {
  if (objectMetadatas.size() == 0) return common::ErrorCode::SUCCESS;

  initTensors(context, objectMetadatas);

  int i = 0;
  for (auto& objMetadata : objectMetadatas) {
    if (objMetadata->mFrame->mSpData != nullptr) {
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

      // bm_image_destroy(image1);

      bm_image image_aligned;
      bool need_copy = image1.width & (64 - 1);
      if (need_copy) {
        int stride1[3], stride2[3];
        bm_image_get_stride(image1, stride1);
        stride2[0] = FFALIGN(stride1[0], 64);
        stride2[1] = FFALIGN(stride1[1], 64);
        stride2[2] = FFALIGN(stride1[2], 64);
        bm_image_create(context->handle, image1.height, image1.width,
                        image1.image_format, image1.data_type, &image_aligned,
                        stride2);

        bm_image_alloc_dev_mem(image_aligned, BMCV_IMAGE_FOR_IN);
        bmcv_copy_to_atrr_t copyToAttr;
        memset(&copyToAttr, 0, sizeof(copyToAttr));
        copyToAttr.start_x = 0;
        copyToAttr.start_y = 0;
        copyToAttr.if_padding = 1;
        bmcv_image_copy_to(context->handle, copyToAttr, image1, image_aligned);
      } else {
        image_aligned = image1;
      }

      // bm_image_destroy(image_aligned);

      // // return common::ErrorCode::SUCCESS;

      float scale_w = float(context->net_w) / image_aligned.width;
      float scale_h = float(context->net_h) / image_aligned.height;

      int pad_w = context->net_w;
      int pad_h = context->net_h;

      float scale_min = scale_h;
      if (scale_w < scale_h) {
        pad_h = image_aligned.height * scale_w;
        scale_min = scale_w;
      } else {
        pad_w = image_aligned.width * scale_h;
      }
      bmcv_padding_atrr_t padding_attr;
      memset(&padding_attr, 0, sizeof(padding_attr));
      padding_attr.dst_crop_stx = 0;
      padding_attr.dst_crop_sty = 0;
      padding_attr.dst_crop_w = pad_w;
      padding_attr.dst_crop_h = pad_h;
      padding_attr.padding_b = 114;
      padding_attr.padding_g = 114;
      padding_attr.padding_r = 114;

      // some API only accept bm_image whose stride is aligned to 64
      int aligned_net_w = FFALIGN(context->net_w, 64);
      int strides[3] = {aligned_net_w, aligned_net_w, aligned_net_w};
      bm_image_create(context->handle, context->net_h, context->net_w,
                      FORMAT_RGB_PLANAR, DATA_TYPE_EXT_1N_BYTE, &resized_img,
                      strides);
      bm_image_alloc_dev_mem(resized_img, BMCV_IMAGE_FOR_IN);

      bmcv_rect_t crop_rect{0, 0, image1.width, image1.height};
      auto ret = bmcv_image_vpp_convert_padding(context->handle, 1,
                                                image_aligned, &resized_img,
                                                &padding_attr, &crop_rect);
      assert(BM_SUCCESS == ret);

      if (image0.image_format != FORMAT_BGR_PLANAR) {
        bm_image_destroy(image1);
      }
      if (need_copy) bm_image_destroy(image_aligned);

      bm_image_data_format_ext img_dtype = DATA_TYPE_EXT_FLOAT32;
      const std::shared_ptr<BMNNTensor> tensor =
          context->bmNetwork->inputTensor(0);
      if (tensor->get_dtype() == BM_INT8) {
        img_dtype = DATA_TYPE_EXT_1N_BYTE_SIGNED;
      }

      bm_image_create(context->handle, context->net_h, context->net_w,
                      FORMAT_RGB_PLANAR, img_dtype, &converto_img);

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
  }

  return common::ErrorCode::SUCCESS;
}

}  // namespace yolox
}  // namespace element
}  // namespace sophon_stream
