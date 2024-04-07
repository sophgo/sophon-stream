//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#include "fastpose_pre_process.h"

#include "common/logger.h"

namespace sophon_stream {
namespace element {
namespace fastpose {

void FastposePreProcess::init(std::shared_ptr<FastposeContext> context) {}

void FastposePreProcess::initTensors(std::shared_ptr<FastposeContext> context,
                                     common::ObjectMetadatas& objectMetadatas) {
  for (auto& obj : objectMetadatas) {
    obj->mSubInputBMtensors =
        std::make_shared<sophon_stream::common::bmSubTensors>();
    int channelId = obj->mFrame->mChannelId;
    int frameId = obj->mFrame->mFrameId;
    obj->mSubInputBMtensors.reset(
        new sophon_stream::common::bmSubTensors(),
        [channelId, frameId](sophon_stream::common::bmSubTensors* p) {
          for (int i = 0; i < p->tensors.size(); ++i) {
            for (int j = 0; j < p->tensors[i].size(); j++) {
              if (p->tensors[i][j]->device_mem.u.device.device_addr != 0) {
                bm_free_device(p->handle, p->tensors[i][j]->device_mem);
              }
            }
          }

          delete p;
          p = nullptr;
        });
    obj->mSubInputBMtensors->handle = context->handle;
    obj->mSubInputBMtensors->tensors.resize(
        obj->mDetectedObjectMetadatas.size());
    for (int i = 0; i < obj->mDetectedObjectMetadatas.size(); ++i) {
      obj->mSubInputBMtensors->tensors[i].resize(context->input_num);
      for (int j = 0; j < context->input_num; j++) {
        obj->mSubInputBMtensors->tensors[i][j] =
            std::make_shared<bm_tensor_t>();
        obj->mSubInputBMtensors->tensors[i][j]->dtype =
            context->bmNetwork->m_netinfo->input_dtypes[j];
        obj->mSubInputBMtensors->tensors[i][j]->shape =
            context->bmNetwork->m_netinfo->stages[0].input_shapes[j];
        obj->mSubInputBMtensors->tensors[i][j]->shape.dims[0] = 1;
        obj->mSubInputBMtensors->tensors[i][j]->st_mode = BM_STORE_1N;
      }
    }
  }
}

common::ErrorCode FastposePreProcess::preProcess(
    std::shared_ptr<FastposeContext> context,
    common::ObjectMetadatas& objectMetadatas) {
  if (objectMetadatas.size() == 0) return common::ErrorCode::SUCCESS;
  initTensors(context, objectMetadatas);

  int i = 0;
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

    int j = 0;
    // filter noise box
    while (j < objMetadata->mDetectedObjectMetadatas.size()) {
      if (objMetadata->mDetectedObjectMetadatas[j]->mBox.mWidth < 16 ||
          objMetadata->mDetectedObjectMetadatas[j]->mBox.mHeight < 16)
        objMetadata->mDetectedObjectMetadatas.erase(
            objMetadata->mDetectedObjectMetadatas.begin() + j,
            objMetadata->mDetectedObjectMetadatas.begin() + j + 1);
      else
        j++;
    }

    j = 0;
    for (auto& detObj : objMetadata->mDetectedObjectMetadatas) {
      int mCenterX = detObj->mBox.mX + detObj->mBox.mWidth / 2;
      int mCenterY = detObj->mBox.mY + detObj->mBox.mHeight / 2;
      int scale_x, scale_y;
      float aspect_ratio = float(context->m_net_w) / float(context->m_net_h);
      if (detObj->mBox.mWidth > aspect_ratio * detObj->mBox.mHeight) {
        scale_y = int(float(detObj->mBox.mWidth) / aspect_ratio);
        scale_x = detObj->mBox.mWidth;
      } else if (detObj->mBox.mWidth < aspect_ratio * detObj->mBox.mHeight) {
        scale_y = detObj->mBox.mHeight;
        scale_x = int(float(detObj->mBox.mHeight) * aspect_ratio);
      } else {
        scale_y = detObj->mBox.mHeight;
        scale_x = detObj->mBox.mWidth;
      }
      if (mCenterX != -1) {
        scale_x *= 1.25;
        scale_y *= 1.25;
      }

      detObj->mCroppedBox.mX = mCenterX - scale_x / 2;
      detObj->mCroppedBox.mY = mCenterY - scale_y / 2;
      detObj->mCroppedBox.mWidth = scale_x;
      detObj->mCroppedBox.mHeight = scale_y;
      int padding_left =
          detObj->mCroppedBox.mX < 0 ? -detObj->mCroppedBox.mX : 0;
      int padding_right = ((detObj->mCroppedBox.mX + detObj->mCroppedBox.mWidth) >=
                                  image_aligned.width)
                              ? (detObj->mCroppedBox.mX +
                                    detObj->mCroppedBox.mWidth -
                                    image_aligned.width + 1)
                              : 0;
      int padding_top =
          detObj->mCroppedBox.mY < 0 ? -detObj->mCroppedBox.mY : 0;
      int padding_bottom =
          ((detObj->mCroppedBox.mY + detObj->mCroppedBox.mHeight) >=
                  image_aligned.height)
              ? (detObj->mCroppedBox.mY + detObj->mCroppedBox.mHeight -
                    image_aligned.height + 1)
              : 0;
      bmcv_rect_t crop_rect = {
          .start_x = detObj->mCroppedBox.mX + padding_left,
          .start_y = detObj->mCroppedBox.mY + padding_top,
          .crop_w = detObj->mCroppedBox.mWidth - padding_left - padding_right,
          .crop_h = detObj->mCroppedBox.mHeight - padding_top - padding_bottom};

      bm_image resized_img;
      int aligned_net_w = FFALIGN(context->m_net_w, 64);
      int strides[3] = {aligned_net_w, aligned_net_w, aligned_net_w};
      auto ret = bm_image_create(context->bmContext->handle(), context->m_net_h,
                                 context->m_net_w, FORMAT_BGR_PLANAR,
                                 DATA_TYPE_EXT_1N_BYTE, &resized_img, strides);
      assert(BM_SUCCESS == ret);
      bm_image converto_img;

      bmcv_padding_atrr_t padding_attr = {
          .dst_crop_stx = padding_left * context->m_net_w / scale_x,
          .dst_crop_sty = padding_top * context->m_net_h / scale_y,
          .dst_crop_w = crop_rect.crop_w * context->m_net_w / scale_x,
          .dst_crop_h = crop_rect.crop_h * context->m_net_h / scale_y,
          .padding_r = 0,
          .padding_g = 0,
          .padding_b = 0};
      ret = bmcv_image_vpp_convert_padding(context->bmContext->handle(), 1,
                                           image_aligned, &resized_img,
                                           &padding_attr, &crop_rect);
      assert(BM_SUCCESS == ret);

      bm_image_data_format_ext img_dtype = DATA_TYPE_EXT_FLOAT32;
      auto tensor = context->bmNetwork->inputTensor(0);
      if (tensor->get_dtype() == BM_INT8) {
        img_dtype = DATA_TYPE_EXT_1N_BYTE_SIGNED;
      }

      bm_image_create(context->bmNetwork->m_handle, context->m_net_h,
                      context->m_net_w, FORMAT_BGR_PLANAR, img_dtype,
                      &converto_img);

      bm_device_mem_t mem;
      int size_byte = 0;
      bm_image_get_byte_size(converto_img, &size_byte);
      ret =
          bm_malloc_device_byte(context->bmContext->handle(), &mem, size_byte);
      bm_image_attach(converto_img, &mem);

      bmcv_image_convert_to(context->bmContext->handle(), 1,
                            context->converto_attr, &resized_img,
                            &converto_img);

      bm_image_destroy(resized_img);

      bm_image_get_device_mem(
          converto_img,
          &objectMetadatas[i]->mSubInputBMtensors->tensors[j][0]->device_mem);
      bm_image_detach(converto_img);
      bm_image_destroy(converto_img);
      j++;
    }
    if (need_copy) bm_image_destroy(image_aligned);
    i++;
  }

  return common::ErrorCode::SUCCESS;
}

}  // namespace fastpose
}  // namespace element
}  // namespace sophon_stream