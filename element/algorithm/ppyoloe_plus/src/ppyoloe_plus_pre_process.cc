//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#include "ppyoloe_plus_pre_process.h"

namespace sophon_stream {
namespace element {
namespace ppyoloe_plus {

void Ppyoloe_plusPreProcess::init(std::shared_ptr<Ppyoloe_plusContext> context) {}

common::ErrorCode Ppyoloe_plusPreProcess::preProcess(
    std::shared_ptr<Ppyoloe_plusContext> context,
    common::ObjectMetadatas& objectMetadatas) {
  if (objectMetadatas.size() == 0) return common::ErrorCode::SUCCESS;
  initTensors(context, objectMetadatas);
  
  // write your pre process here
  auto jsonPlanner = context->bgr2rgb ? FORMAT_RGB_PLANAR : FORMAT_BGR_PLANAR;
  int i = 0;
  for (auto& objMetadata : objectMetadatas) {
    if (objMetadata->mFrame->mSpData == nullptr) continue;
    
    // 1、图片预处理，准备模型第一个输入张量
    bm_image image0 = *objMetadata->mFrame->mSpData;
    bm_image rgb_img; // rgb图片
    bm_image image_aligned; // 对齐图片
    bm_image resized_img; // 等比缩放图片
    bm_image converto_img; // 归一化图片
    
    // convert to RGB_PLANAR
    if (image0.image_format != jsonPlanner) {
      bm_image_create(context->handle, image0.height, image0.width, jsonPlanner,
                      image0.data_type, &rgb_img);
      auto ret = bm_image_alloc_dev_mem_heap_mask(rgb_img, STREAM_VPU_HEAP_MASK);
      STREAM_CHECK(ret == 0, "Alloc Device Memory Failed! Program Terminated.")
      bmcv_image_storage_convert(context->handle, 1, &image0, &rgb_img);
    } else {
      rgb_img = image0;
    }

    // 对齐
    bool need_copy = rgb_img.width & (64 - 1);
    if (need_copy) {
      int stride1[3], stride2[3];
      bm_image_get_stride(rgb_img, stride1);
      stride2[0] = FFALIGN(stride1[0], 64);
      stride2[1] = FFALIGN(stride1[1], 64);
      stride2[2] = FFALIGN(stride1[2], 64);
      bm_image_create(context->bmContext->handle(), rgb_img.height, rgb_img.width,
                      rgb_img.image_format, rgb_img.data_type, &image_aligned,
                      stride2);

      auto ret = bm_image_alloc_dev_mem_heap_mask(image_aligned, STREAM_VPU_HEAP_MASK);
      STREAM_CHECK(ret == 0, "Alloc Device Memory Failed! Program Terminated.")
      bmcv_copy_to_atrr_t copyToAttr;
      memset(&copyToAttr, 0, sizeof(copyToAttr));
      copyToAttr.start_x = 0;
      copyToAttr.start_y = 0;
      copyToAttr.if_padding = 1;
      bmcv_image_copy_to(context->bmContext->handle(), copyToAttr, rgb_img,
                         image_aligned);
    } else {
      image_aligned = rgb_img;
    }

    // ----等比缩放填充操作 letterbox---
    int aligned_net_w = FFALIGN(context->net_w, 64);
    int strides[3] = {aligned_net_w, aligned_net_w, aligned_net_w};
    bm_image_create(context->handle, context->net_h, context->net_w,
                    jsonPlanner, DATA_TYPE_EXT_1N_BYTE, &resized_img, strides);
    auto ret = bm_image_alloc_dev_mem_heap_mask(resized_img, STREAM_VPP_HEAP_MASK);
    STREAM_CHECK(ret == 0, "Alloc Device Memory Failed! Program Terminated.")

    bool isAlignWidth = false;  // 是否是按宽度缩放比例缩放
    float ratio = get_aspect_scaled_ratio(image_aligned.width, image_aligned.height, context->net_w, context->net_h, &isAlignWidth);
    bmcv_padding_atrr_t padding_attr;
    memset(&padding_attr, 0, sizeof(padding_attr));
    padding_attr.dst_crop_sty = 0;
    padding_attr.dst_crop_stx = 0;
    padding_attr.padding_b = 114;
    padding_attr.padding_g = 114;
    padding_attr.padding_r = 114;
    padding_attr.if_memset = 1;
    if (isAlignWidth) {
        padding_attr.dst_crop_h = image_aligned.height*ratio;  // 高度按比例缩放
        padding_attr.dst_crop_w = context->net_w;  // 宽度等于网络输入宽

        int ty1 = (int)((context->net_h - padding_attr.dst_crop_h) / 2);
        padding_attr.dst_crop_sty = ty1;  // 垂直居中
        padding_attr.dst_crop_stx = 0;  // 水平靠左
    }else{
        padding_attr.dst_crop_h = context->net_h;
        padding_attr.dst_crop_w = image_aligned.width*ratio;

    int tx1 = (int)((context->net_w - padding_attr.dst_crop_w) / 2);
    padding_attr.dst_crop_sty = 0;
    padding_attr.dst_crop_stx = tx1;
    }

    bmcv_rect_t crop_rect{0, 0, image_aligned.width, image_aligned.height};
    auto ret1 = bmcv_image_vpp_convert_padding(context->handle, 1, image_aligned, &resized_img,
        &padding_attr, &crop_rect, BMCV_INTER_NEAREST);
    assert(BM_SUCCESS == ret1);

    if(need_copy) bm_image_destroy(image_aligned);
    if (image0.image_format != FORMAT_BGR_PLANAR) {
      bm_image_destroy(rgb_img);
    }

    // -----图片归一化-----
    bm_image_data_format_ext img_dtype = DATA_TYPE_EXT_FLOAT32;
    auto tensor = context->bmNetwork->inputTensor(0);
    if (tensor->get_dtype() == BM_INT8) {
      img_dtype = DATA_TYPE_EXT_1N_BYTE_SIGNED;
    } else if (tensor->get_dtype() == BM_FLOAT16) {
      img_dtype = DATA_TYPE_EXT_FP16;
    }
    
    // converto_img分配空间
    bm_image_create(context->handle, context->net_h, context->net_w,
                    jsonPlanner, img_dtype, &converto_img);
    bm_device_mem_t mem;
    int size_byte = 0;
    bm_image_get_byte_size(converto_img, &size_byte);
    ret = bm_malloc_device_byte_heap(context->handle, &mem, STREAM_NPU_HEAP, size_byte);
    STREAM_CHECK(ret == 0, "Alloc Device Memory Failed! Program Terminated.")

    bm_image_attach(converto_img, &mem);

    // 图片归一化, 标准化
    bmcv_image_convert_to(context->handle, 1, context->converto_attr,
                          &resized_img, &converto_img);


    bm_image_destroy(resized_img);

    // converto_img绑定到objectMetadatas
    bm_image_get_device_mem(
        converto_img,
        &objectMetadatas[i]->mInputBMtensors->tensors[0]->device_mem);
    bm_image_detach(converto_img);
    bm_image_destroy(converto_img);


    // 2、--- prepare ratio tensor (robust version) ---
    auto net_info = bmrt_get_network_info(context->bmContext->bmrt(),
                                         context->bmContext->network_name(0).c_str());

    // ensure tensors vector has at least input_num slots
    int input_num = context->bmNetwork->m_netinfo->input_num; // or context->bmNetwork->input_num
    if ((int)objectMetadatas[i]->mInputBMtensors->tensors.size() < input_num) {
      objectMetadatas[i]->mInputBMtensors->tensors.resize(input_num);
    }

    // create shared_ptr slot if empty
    if (!objectMetadatas[i]->mInputBMtensors->tensors[1]) {
      objectMetadatas[i]->mInputBMtensors->tensors[1] = std::make_shared<bm_tensor_t>();
    }

    // allocate device mem for ratio (prefer to do this once in init; shown here per-frame for clarity)
    bm_device_mem_t dev_mem;
    bm_status_t st = bm_malloc_device_byte(context->bmContext->handle(),
                                           &dev_mem,
                                           net_info->max_input_bytes[1]);
    STREAM_CHECK(st == 0, "Alloc device mem for ratio failed");

    // build the bm_tensor_t that runtime expects
    auto ratio_tensor = objectMetadatas[i]->mInputBMtensors->tensors[1];
    ratio_tensor->device_mem = dev_mem;                         // device mem
    ratio_tensor->dtype = context->bmNetwork->inputTensor(1)->get_dtype();
    ratio_tensor->st_mode = BM_STORE_1N;
    // set shape exactly as net_info expects (dims array must match)
    ratio_tensor->shape = {2, { net_info->stages[0].input_shapes[1].dims[0], 2 } };

    // compute ratio (example: h_ratio, w_ratio OR width/height according to your model)
    float ratio_arr[2];
    ratio_arr[0] = ratio; // consistent with your previous code
    ratio_arr[1] = ratio;

    // check bytesize expected by runtime for this tensor
    size_t expect_bytes = bmrt_tensor_bytesize(ratio_tensor.get());
    if (expect_bytes == 0) {
      IVS_CRITICAL("bmrt_tensor_bytesize returned 0 for ratio tensor");
    }

    // copy host -> device to the exact device_mem we assigned
    st = bm_memcpy_s2d_partial(context->bmContext->handle(),
                               ratio_tensor->device_mem,
                               (void*)ratio_arr,
                               expect_bytes);
    STREAM_CHECK(st == 0, "bm_memcpy_s2d_partial for ratio failed");
    
    i++;
  }
  
  return common::ErrorCode::SUCCESS;
}

}  // namespace ppyoloe_plus
}  // namespace element
}  // namespace sophon_stream