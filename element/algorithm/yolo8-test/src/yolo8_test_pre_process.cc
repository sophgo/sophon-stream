//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#include "yolo8_test_pre_process.h"

namespace sophon_stream {
namespace element {
namespace yolo8_test {

void Yolo8TestPreProcess::init(std::shared_ptr<Yolo8TestContext> context) {}

common::ErrorCode Yolo8TestPreProcess::preProcess(
    std::shared_ptr<Yolo8TestContext> context,
    common::ObjectMetadatas& objectMetadatas) {
  if (objectMetadatas.size() == 0) return common::ErrorCode::SUCCESS;

  // 为每个 ObjectMetadata 初始化输入 tensor 容器。
  // 后面会把预处理结果的 device memory 挂到 mInputBMtensors。
  initTensors(context, objectMetadatas);

  // 根据模型输入布局选择目标图像格式：
  // 普通 YOLOv8 通常是 RGB/BGR PLANAR；部分模型输入是 BGR_PACKED(NHWC)。
  auto jsonPlanner = context->bgr_packed_input
                         ? FORMAT_BGR_PACKED
                         : (context->bgr2rgb ? FORMAT_RGB_PLANAR : FORMAT_BGR_PLANAR);
  int i = 0;
  for (auto& objMetadata : objectMetadatas) {
    if (objMetadata->mFrame->mSpData == nullptr) continue;
    bm_image resized_img;
    bm_image converto_img;
    bm_image image0 = *objMetadata->mFrame->mSpData;
    bm_image image1;

    // 1. 将解码输出转换成模型需要的颜色/排列格式。
    if (image0.image_format != jsonPlanner) {
      bm_image_create(context->handle, image0.height, image0.width, jsonPlanner,
                      image0.data_type, &image1);
      auto ret = bm_image_alloc_dev_mem_heap_mask(image1, STREAM_VPU_HEAP_MASK);
      STREAM_CHECK(ret == 0, "Alloc Device Memory Failed! Program Terminated.")
      bmcv_image_storage_convert(context->handle, 1, &image0, &image1);
    } else {
      image1 = image0;
    }

    // 2. BMCV/VPP 对部分输入宽度有对齐要求，不满足 64 对齐时先复制到对齐图像。
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

      auto ret = bm_image_alloc_dev_mem_heap_mask(image_aligned, STREAM_VPU_HEAP_MASK);
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

    // 3. 计算 letterbox resize 参数：保持宽高比缩放，空白区域填 114。
    // 如果配置了 ROI，则以 ROI 尺寸而不是整图尺寸计算缩放比例。
    bool isAlignWidth = false;
    float ratio = context->roi_predefined
                      ? get_aspect_scaled_ratio(
                            context->roi.crop_w, context->roi.crop_h,
                            context->net_w, context->net_h, &isAlignWidth)
                      : get_aspect_scaled_ratio(image0.width, image0.height,
                                                context->net_w, context->net_h,
                                                &isAlignWidth);
    bmcv_padding_atrr_t padding_attr;
    memset(&padding_attr, 0, sizeof(padding_attr));
    padding_attr.dst_crop_sty = 0;
    padding_attr.dst_crop_stx = 0;
    padding_attr.padding_b = 114;
    padding_attr.padding_g = 114;
    padding_attr.padding_r = 114;
    padding_attr.if_memset = 1;
    if (isAlignWidth) {
      padding_attr.dst_crop_h = context->roi_predefined
                                    ? (context->roi.crop_h * ratio)
                                    : (image0.height * ratio);
      padding_attr.dst_crop_w = context->net_w;

      int ty1 = (int)((context->net_h - padding_attr.dst_crop_h) / 2);
      padding_attr.dst_crop_sty = ty1;
      padding_attr.dst_crop_stx = 0;
    } else {
      padding_attr.dst_crop_h = context->net_h;
      padding_attr.dst_crop_w = context->roi_predefined
                                    ? (context->roi.crop_w * ratio)
                                    : (image0.width * ratio);

      int tx1 = (int)((context->net_w - padding_attr.dst_crop_w) / 2);
      padding_attr.dst_crop_sty = 0;
      padding_attr.dst_crop_stx = tx1;
    }

    // 4. 创建网络输入尺寸的中间图像。BGR_PACKED 与 PLANAR 的 stride 计算不同。
    int aligned_net_w = FFALIGN(context->net_w, 64);
    int strides[3];
    if (context->bgr_packed_input) {
      // BGR_PACKED: single plane, stride = width * 3 bytes
      int row_stride = FFALIGN(context->net_w * 3, 64);
      strides[0] = row_stride;
      strides[1] = row_stride;
      strides[2] = row_stride;
    } else {
      strides[0] = aligned_net_w;
      strides[1] = aligned_net_w;
      strides[2] = aligned_net_w;
    }
    bm_image_create(context->handle, context->net_h, context->net_w,
                    jsonPlanner, DATA_TYPE_EXT_1N_BYTE, &resized_img, strides);
    auto ret = bm_image_alloc_dev_mem_heap_mask(resized_img, STREAM_VPP_HEAP_MASK);
    STREAM_CHECK(ret == 0, "Alloc Device Memory Failed! Program Terminated.")
    bmcv_rect_t crop_rect{0, 0, image1.width, image1.height};

    // 5. 执行 crop + resize + padding。ROI 模式下只裁剪配置区域。
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
    STREAM_CHECK(ret == 0, "Vpp Convert Padding Failed! Program Terminated.")

    if (image0.image_format != jsonPlanner) {
      bm_image_destroy(image1);
    }
    if (need_copy) bm_image_destroy(image_aligned);

    if (context->bgr_packed_input) {
      // BGR_PACKED 输入的模型通常已融合归一化，直接把 resized_img 的显存作为输入 tensor。
      bm_image_get_device_mem(
          resized_img,
          &objectMetadatas[i]->mInputBMtensors->tensors[0]->device_mem);
      bm_image_detach(resized_img);
      bm_image_destroy(resized_img);
    } else {
      // 6. PLANAR 输入需要执行 convert_to，完成 dtype 转换和 mean/std 归一化。
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
      ret = bm_malloc_device_byte_heap(context->handle, &mem, STREAM_NPU_HEAP,
                                       size_byte);
      STREAM_CHECK(ret == 0, "Alloc Device Memory Failed! Program Terminated.")
      bm_image_attach(converto_img, &mem);

      bmcv_image_convert_to(context->handle, 1, context->converto_attr,
                            &resized_img, &converto_img);

      bm_image_destroy(resized_img);

      bm_image_get_device_mem(
          converto_img,
          &objectMetadatas[i]->mInputBMtensors->tensors[0]->device_mem);

      // detach 后 bm_image_destroy 不会释放 tensor 正在使用的 device memory。
      bm_image_detach(converto_img);
      bm_image_destroy(converto_img);
    }
    i++;
  }
  return common::ErrorCode::SUCCESS;
}

}  // namespace yolo8_test
}  // namespace element
}  // namespace sophon_stream
