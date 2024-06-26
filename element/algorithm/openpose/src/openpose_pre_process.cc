//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#include "openpose_pre_process.h"

namespace sophon_stream {
namespace element {
namespace openpose {

void OpenposePreProcess::init(std::shared_ptr<OpenposeContext> context) {}

common::ErrorCode OpenposePreProcess::preProcess(
    std::shared_ptr<OpenposeContext> context,
    common::ObjectMetadatas& objectMetadatas) {
  if (objectMetadatas.size() == 0) return common::ErrorCode::SUCCESS;
  initTensors(context, objectMetadatas);

  int i = 0;
  for (auto& objMetadata : objectMetadatas) {
    if (objMetadata->mFrame->mSpData == nullptr) continue;
    bm_image resized_img;
    int aligned_net_w = FFALIGN(context->net_w, 64);
    int strides[3] = {aligned_net_w, aligned_net_w, aligned_net_w};
    auto ret = bm_image_create(context->bmContext->handle(), context->net_h,
                               context->net_w, FORMAT_BGR_PLANAR,
                               DATA_TYPE_EXT_1N_BYTE, &resized_img, strides);
    assert(BM_SUCCESS == ret);
    bm_image converto_img;
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

    ret = bmcv_image_vpp_convert(context->bmContext->handle(), 1, image_aligned,
                                 &resized_img);
    STREAM_CHECK(ret == 0, "Vpp Convert Failed! Program Terminated.")

    if (need_copy) bm_image_destroy(image_aligned);

    bm_image_data_format_ext img_dtype = DATA_TYPE_EXT_FLOAT32;
    auto tensor = context->bmNetwork->inputTensor(0);
    if (tensor->get_dtype() == BM_INT8) {
      img_dtype = DATA_TYPE_EXT_1N_BYTE_SIGNED;
    }

    bm_image_create(context->bmNetwork->m_handle, context->net_h,
                    context->net_w, FORMAT_BGR_PLANAR, img_dtype,
                    &converto_img);

    bm_device_mem_t mem;
    int size_byte = 0;
    bm_image_get_byte_size(converto_img, &size_byte);
    ret = bm_malloc_device_byte(context->bmContext->handle(), &mem, size_byte);
    STREAM_CHECK(ret == 0, "Alloc Device Memory Failed! Program Terminated.")
    bm_image_attach(converto_img, &mem);

    bmcv_image_convert_to(context->bmContext->handle(), 1,
                          context->converto_attr, &resized_img, &converto_img);

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

}  // namespace openpose
}  // namespace element
}  // namespace sophon_stream