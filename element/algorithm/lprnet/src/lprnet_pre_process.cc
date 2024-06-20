//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#include "lprnet_pre_process.h"

namespace sophon_stream {
namespace element {
namespace lprnet {

void LprnetPreProcess::init(std::shared_ptr<LprnetContext> context) {}

common::ErrorCode LprnetPreProcess::preProcess(
    std::shared_ptr<LprnetContext> context,
    common::ObjectMetadatas& objectMetadatas) {
  if (objectMetadatas.size() == 0) return common::ErrorCode::SUCCESS;
  initTensors(context, objectMetadatas);

  // 0. load images
  int i = 0;
  for (auto& objMetadata : objectMetadatas) {
    if (objMetadata->mFrame->mSpData == nullptr) continue;
    bm_image resized_img;
    bm_image converto_img;
    bm_image image0 =
        *objMetadata->mFrame->mSpData;  // get bm_image for 1 bacth

    // 1. Copy image1 to image_aligned, align at 64
    bm_image image_aligned;
    bool need_copy = image0.width & (64 - 1);
    if (need_copy) {
      int stride1[3], stride2[3];
      bm_image_get_stride(image0, stride1);
      stride2[0] = FFALIGN(stride1[0], 64);
      stride2[1] = FFALIGN(stride1[1], 64);
      stride2[2] = FFALIGN(stride1[2], 64);
      bm_image_create(context->bmContext->handle(), image0.height, image0.width,
                      image0.image_format, image0.data_type, &image_aligned,
                      stride2);

      auto ret = bm_image_alloc_dev_mem(image_aligned, BMCV_IMAGE_FOR_IN);
      STREAM_CHECK(ret == 0, "Alloc Device Memory Failed! Program Terminated.")
      bmcv_copy_to_atrr_t copyToAttr;
      memset(&copyToAttr, 0, sizeof(copyToAttr));
      copyToAttr.start_x = 0;
      copyToAttr.start_y = 0;
      copyToAttr.if_padding = 1;
      bmcv_image_copy_to(context->bmContext->handle(), copyToAttr, image0,
                         image_aligned);
    } else {
      image_aligned = image0;
    }

    // 2. Resize image
    int aligned_net_w = FFALIGN(context->net_w, 64);
    int strides[3] = {aligned_net_w, aligned_net_w, aligned_net_w};
    bm_image_create(context->handle, context->net_h, context->net_w,
                    FORMAT_BGR_PLANAR, DATA_TYPE_EXT_1N_BYTE, &resized_img,
                    strides);
    auto ret = bm_image_alloc_dev_mem_heap_mask(resized_img, 4);
    STREAM_CHECK(ret == 0, "Alloc Device Memory Failed! Program Terminated.")
    bmcv_image_vpp_convert(
        context->bmContext->handle(), 1, image_aligned,
        &resized_img);  // heap mask = 1, mask code is 001, on heap0, for TPU
    STREAM_CHECK(ret == 0, "Vpp Convert Padding Failed! Program Terminated.")

    // Initialize converto_img
    bm_image_data_format_ext img_dtype = DATA_TYPE_EXT_FLOAT32;
    auto tensor = context->bmNetwork->inputTensor(0);
    if (tensor->get_dtype() == BM_INT8) {
      img_dtype = DATA_TYPE_EXT_1N_BYTE_SIGNED;
    }
    bm_image_create(context->handle, context->net_h, context->net_w,
                    FORMAT_BGR_PLANAR, img_dtype, &converto_img);

    bm_device_mem_t mem;
    int size_byte = 0;
    bm_image_get_byte_size(converto_img, &size_byte);
    ret = bm_malloc_device_byte_heap(context->handle, &mem, 0, size_byte);
    STREAM_CHECK(ret == 0, "Alloc Device Memory Failed! Program Terminated.")

    bm_image_attach(converto_img, &mem);

    // 3. Convert to
    bmcv_image_convert_to(context->handle, 1, context->converto_attr,
                          &resized_img, &converto_img);

    // 4. Attach converto_img to tensor (on device memory)
    ret = bm_image_get_device_mem(
        converto_img,
        &objectMetadatas[i]->mInputBMtensors->tensors[0]->device_mem);
    assert(ret == BM_SUCCESS);

    // Avoid memory fragment
    if (need_copy) bm_image_destroy(image_aligned);
    bm_image_destroy(resized_img);
    bm_image_detach(converto_img);
    bm_image_destroy(converto_img);

    i++;
  }
  return common::ErrorCode::SUCCESS;
}

}  // namespace lprnet
}  // namespace element
}  // namespace sophon_stream