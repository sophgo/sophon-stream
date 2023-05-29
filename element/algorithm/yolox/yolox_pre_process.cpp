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
#include "common/type_trans.hpp"

namespace sophon_stream {
namespace element {
namespace yolox {

#define DUMP_FILE 0

void YoloxPreProcess::initTensors(std::shared_ptr<YoloxContext> context,
                                  common::ObjectMetadatas& objectMetadatas) {
  objectMetadatas[0]->mInputBMtensors =
      std::make_shared<sophon_stream::common::bmTensors>();
  objectMetadatas[0]->mOutputBMtensors =
      std::make_shared<sophon_stream::common::bmTensors>();

  objectMetadatas[0]->mInputBMtensors.reset(
      new sophon_stream::common::bmTensors(),
      [](sophon_stream::common::bmTensors* p) {
        for (int i = 0; i < p->tensors.size(); ++i)
          bm_free_device(p->handle, p->tensors[i]->device_mem);
        delete p;
        p = nullptr;
      });
  objectMetadatas[0]->mOutputBMtensors.reset(
      new sophon_stream::common::bmTensors(),
      [](sophon_stream::common::bmTensors* p) {
        for (int i = 0; i < p->tensors.size(); ++i)
          bm_free_device(p->handle, p->tensors[i]->device_mem);
        delete p;
        p = nullptr;
      });
  objectMetadatas[0]->mInputBMtensors->handle = context->handle;
  objectMetadatas[0]->mOutputBMtensors->handle = context->handle;

  objectMetadatas[0]->mInputBMtensors->tensors.resize(context->input_num);
  objectMetadatas[0]->mOutputBMtensors->tensors.resize(context->output_num);

  for (int i = 0; i < context->input_num; ++i) {
    objectMetadatas[0]->mInputBMtensors->tensors[i] =
        std::make_shared<bm_tensor_t>();
    objectMetadatas[0]->mInputBMtensors->tensors[i]->dtype =
        context->m_bmNetwork->m_netinfo->input_dtypes[i];
    objectMetadatas[0]->mInputBMtensors->tensors[i]->shape =
        context->m_bmNetwork->m_netinfo->stages[0].input_shapes[i];
    objectMetadatas[0]->mInputBMtensors->tensors[i]->st_mode = BM_STORE_1N;
    // 前处理的mInpuptBMtensors不需要申请内存，在preprocess中通过std::move得到
  }
  for (int i = 0; i < context->output_num; ++i) {
    objectMetadatas[0]->mOutputBMtensors->tensors[i] =
        std::make_shared<bm_tensor_t>();
    objectMetadatas[0]->mOutputBMtensors->tensors[i]->dtype =
        context->m_bmNetwork->m_netinfo->output_dtypes[i];
    objectMetadatas[0]->mOutputBMtensors->tensors[i]->shape =
        context->m_bmNetwork->m_netinfo->stages[0].output_shapes[i];
    objectMetadatas[0]->mOutputBMtensors->tensors[i]->st_mode = BM_STORE_1N;
    size_t max_size = 0;
    // 后处理的mOutputBMtensor需要申请内存，在forward中更新
    for (int s = 0; s < context->m_bmNetwork->m_netinfo->stage_num; s++) {
      size_t out_size = bmrt_shape_count(
          &context->m_bmNetwork->m_netinfo->stages[s].output_shapes[i]);
      if (max_size < out_size) {
        max_size = out_size;
      }
    }
    if (BM_FLOAT32 == context->m_bmNetwork->m_netinfo->output_dtypes[i])
      max_size *= 4;
    auto ret = bm_malloc_device_byte(
        objectMetadatas[0]->mOutputBMtensors->handle,
        &objectMetadatas[0]->mOutputBMtensors->tensors[i]->device_mem,
        max_size);
    assert(BM_SUCCESS == ret);
  }
}

common::ErrorCode YoloxPreProcess::preProcess(
    std::shared_ptr<YoloxContext> context,
    common::ObjectMetadatas& objectMetadatas) {
  if (objectMetadatas.size() == 0) return common::ErrorCode::SUCCESS;
  std::vector<bm_image> images;
  for (auto& objMetadata : objectMetadatas) {
    if (objMetadata->mFrame->mEndOfStream) {
      // end of file, create a blank bmimg
      IVS_CRITICAL("END OF STREAM");
      context->mEndOfStream = objMetadata->mFrame->mEndOfStream;
      objectMetadatas[0]->mFrame->mEndOfStream = true;
      auto tensor = context->m_bmNetwork->inputTensor(0);
      context->m_frame_h = tensor->get_shape()->dims[2];
      context->m_frame_w = tensor->get_shape()->dims[3];
      objMetadata->mFrame->mSpData.reset(new bm_image, [&](bm_image* p) {
        bm_image_destroy(*p);
        delete p;
        p = nullptr;
      });
      bm_image_create(context->m_bmContext->handle(), context->m_frame_h,
                      context->m_frame_w, FORMAT_RGB_PLANAR,
                      DATA_TYPE_EXT_1N_BYTE,
                      objMetadata->mFrame->mSpData.get());
      bm_image_alloc_dev_mem(*objMetadata->mFrame->mSpData);
    } else {
      context->m_frame_w = objMetadata->mFrame->mWidth;
      context->m_frame_h = objMetadata->mFrame->mHeight;
      objMetadata->mAlgorithmHandle =
          std::make_shared<bm_handle_t>(context->m_bmContext->handle());
    }
    images.push_back(*objMetadata->mFrame->mSpData);
  }

  initTensors(context, objectMetadatas);
  std::shared_ptr<BMNNTensor> input_tensor =
      context->m_bmNetwork->inputTensor(0);
  int image_n = images.size();

  // 1. resize image
  int ret = 0;
  for (int i = 0; i < image_n; ++i) {
    bm_image image0 = images[i];
    bm_image image1;
    // convert to RGB_PLANAR
    if (image0.image_format != FORMAT_BGR_PLANAR) {
      bm_image_create(context->m_bmContext->handle(), image0.height,
                      image0.width, FORMAT_BGR_PLANAR, image0.data_type,
                      &image1);
      bm_image_alloc_dev_mem(image1, BMCV_IMAGE_FOR_IN);
      bmcv_image_storage_convert(context->m_bmContext->handle(), 1, &image0,
                                 &image1);
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
      bm_image_create(context->m_bmContext->handle(), image1.height,
                      image1.width, image1.image_format, image1.data_type,
                      &image_aligned, stride2);

      bm_image_alloc_dev_mem(image_aligned, BMCV_IMAGE_FOR_IN);
      bmcv_copy_to_atrr_t copyToAttr;
      memset(&copyToAttr, 0, sizeof(copyToAttr));
      copyToAttr.start_x = 0;
      copyToAttr.start_y = 0;
      copyToAttr.if_padding = 1;
      bmcv_image_copy_to(context->m_bmContext->handle(), copyToAttr, image1,
                         image_aligned);
    } else {
      image_aligned = image1;
    }

    float scale_w = float(context->m_net_w) / image_aligned.width;
    float scale_h = float(context->m_net_h) / image_aligned.height;

    int pad_w = context->m_net_w;
    int pad_h = context->m_net_h;

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

    bmcv_rect_t crop_rect{0, 0, image1.width, image1.height};
    auto ret = bmcv_image_vpp_convert_padding(
        context->m_bmContext->handle(), 1, image_aligned,
        &context->m_resized_imgs[i], &padding_attr, &crop_rect);
    assert(BM_SUCCESS == ret);
#if DUMP_FILE
    // static int b = 0;
    // char szpath2[256] = {0};
    // sprintf(szpath2,"resized.bmp",b);
    // std::string strPath2(szpath2);
    bm_image_write_to_bmp(pSophgoContext->m_resized_imgs[i], "resized.bmp");
    // b++;
#endif
    if (image0.image_format != FORMAT_BGR_PLANAR) {
      bm_image_destroy(image1);
    }
    if (need_copy) bm_image_destroy(image_aligned);
  }
  // 2.1 malloc m_converto_imgs
  bm_image_data_format_ext img_dtype = DATA_TYPE_EXT_FLOAT32;
  auto tensor = context->m_bmNetwork->inputTensor(0);
  if (tensor->get_dtype() == BM_INT8) {
    img_dtype = DATA_TYPE_EXT_1N_BYTE_SIGNED;
  }
  ret = bm_image_create_batch(context->m_bmContext->handle(), context->m_net_h,
                              context->m_net_w, FORMAT_RGB_PLANAR, img_dtype,
                              context->m_converto_imgs.data(),
                              context->max_batch);
  assert(BM_SUCCESS == ret);

  // 2.2 converto
  ret = bmcv_image_convert_to(
      context->m_bmContext->handle(), image_n, context->converto_attr,
      context->m_resized_imgs.data(), context->m_converto_imgs.data());
  CV_Assert(ret == 0);

  // 2.3 get contiguous device_mem of m_converto_imgs
  if (image_n != context->max_batch)
    image_n = context->m_bmNetwork->get_nearest_batch(image_n);
  bm_device_mem_t input_dev_mem;
  bm_image_get_contiguous_device_mem(image_n, context->m_converto_imgs.data(),
                                     &input_dev_mem);

  // 2.4 set inputBMtensors with std::move
  objectMetadatas[0]->mInputBMtensors->tensors[0]->device_mem =
      std::move(input_dev_mem);

  // 3. attach to tensor
  //  if(image_n != context->max_batch) image_n =
  //  context->m_bmNetwork->get_nearest_batch(image_n); bm_device_mem_t
  //  input_dev_mem; bm_image_get_contiguous_device_mem(image_n,
  //  context->m_converto_imgs.data(), &input_dev_mem);
  //  input_tensor->set_device_mem(&input_dev_mem);
  //  input_tensor->set_shape_by_dim(0, image_n);  // set real batch number

  return common::ErrorCode::SUCCESS;
}

}  // namespace yolox
}  // namespace element
}  // namespace sophon_stream