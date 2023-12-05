//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//
#include "common/common_defs.h"
#if BMCV_VERSION_MAJOR > 1

#include "dwa.h"

#include <chrono>
#include <nlohmann/json.hpp>
#include "common/logger.h"
#include "element_factory.h"
namespace sophon_stream {
namespace element {
namespace dwa {
Dwa::Dwa() {}
Dwa::~Dwa() {}

common::ErrorCode Dwa::initInternal(const std::string& json) {
  common::ErrorCode errorCode = common::ErrorCode::SUCCESS;
  auto configure = nlohmann::json::parse(json, nullptr, false);
  if (!configure.is_object()) {
    errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
  }

  auto is_gray = configure.find(CONFIG_INTERNAL_IS_GRAY_FILED)->get<bool>();
  is_resize = configure.find(CONFIG_INTERNAL_IS_RESIZE_FILED)->get<bool>();

  src_fmt = is_gray ? FORMAT_GRAY : FORMAT_YUV420P;

  return common::ErrorCode::SUCCESS;
}

float Dwa::get_aspect_scaled_ratio(int src_w, int src_h, int dst_w, int dst_h,
                                   bool* pIsAligWidth) {
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

common::ErrorCode Dwa::dwa_work(
    std::shared_ptr<common::ObjectMetadata> dwaObj) {
  if (dwaObj != nullptr) {
    // resize
    if (is_resize == true) {
      std::shared_ptr<bm_image> resized_img = nullptr;
      resized_img.reset(new bm_image, [](bm_image* p) {
        bm_image_destroy(*p);
        delete p;
        p = nullptr;
      });

      bm_image image_aligned;
      bool need_copy = (unsigned int)dwaObj->mFrame->mSpData->width & (64 - 1);
      if (need_copy) {
        int stride1[3], stride2[3];
        bm_image_get_stride(*dwaObj->mFrame->mSpData, stride1);
        stride2[0] = FFALIGN(stride1[0], 64);
        stride2[1] = FFALIGN(stride1[1], 64);
        stride2[2] = FFALIGN(stride1[2], 64);
        bm_image_create(dwaObj->mFrame->mHandle,
                        (unsigned int)dwaObj->mFrame->mSpData->height,
                        (unsigned int)dwaObj->mFrame->mSpData->width, src_fmt,
                        DATA_TYPE_EXT_1N_BYTE, &image_aligned, stride2);

        bm_image_alloc_dev_mem(image_aligned, 1);
        bmcv_copy_to_atrr_t copyToAttr;
        memset(&copyToAttr, 0, sizeof(copyToAttr));
        copyToAttr.start_x = 0;
        copyToAttr.start_y = 0;
        copyToAttr.if_padding = 1;
        bmcv_image_copy_to(dwaObj->mFrame->mHandle, copyToAttr,
                           *dwaObj->mFrame->mSpData, image_aligned);
      } else {
        image_aligned = *dwaObj->mFrame->mSpData;
      }

      bool isAlignWidth = false;
      float ratio =
          get_aspect_scaled_ratio((unsigned int)dwaObj->mFrame->mSpData->width,
                                  (unsigned int)dwaObj->mFrame->mSpData->height,
                                  dst_w, dst_h, &isAlignWidth);

      bmcv_rect_t crop_rect{0, 0, (unsigned int)dwaObj->mFrame->mSpData->width,
                            (unsigned int)dwaObj->mFrame->mSpData->height};
      bmcv_padding_atrr_t padding_attr;
      memset(&padding_attr, 0, sizeof(padding_attr));
      padding_attr.dst_crop_sty = 0;
      padding_attr.dst_crop_stx = 0;
      padding_attr.padding_b = 114;
      padding_attr.padding_g = 114;
      padding_attr.padding_r = 114;
      padding_attr.if_memset = 1;
      padding_attr.dst_crop_h = dst_h;
      padding_attr.dst_crop_w =
          ((unsigned int)dwaObj->mFrame->mSpData->width * ratio);

      int tx1 = (int)((dst_w - padding_attr.dst_crop_w) / 2);
      padding_attr.dst_crop_sty = 0;
      padding_attr.dst_crop_stx = tx1;

      int aligned_net_w = FFALIGN(dst_w, 64);
      int strides[3] = {aligned_net_w, aligned_net_w, aligned_net_w};
      bm_image_create(dwaObj->mFrame->mHandle, dst_h, dst_w, src_fmt,
                      DATA_TYPE_EXT_1N_BYTE, resized_img.get(), strides);

#if BMCV_VERSION_MAJOR > 1
      bm_image_alloc_dev_mem_heap_mask(*resized_img, 2);
#else
      bm_image_alloc_dev_mem_heap_mask(*resized_img, 4);
#endif

      bm_status_t ret = bmcv_image_vpp_convert_padding(
          dwaObj->mFrame->mHandle, 1, *dwaObj->mFrame->mSpData,
          resized_img.get(), &padding_attr, &crop_rect);
      assert(BM_SUCCESS == ret);

      std::shared_ptr<bm_image> dwa_image = nullptr;
      dwa_image.reset(new bm_image, [](bm_image* p) {
        bm_image_destroy(*p);
        delete p;
        p = nullptr;
      });

      ret = bm_image_create(dwaObj->mFrame->mHandle, resized_img->height,
                            resized_img->width, src_fmt, DATA_TYPE_EXT_1N_BYTE,
                            dwa_image.get());
#if BMCV_VERSION_MAJOR > 1
      bm_image_alloc_dev_mem_heap_mask(*dwa_image, 2);
#else
      bm_image_alloc_dev_mem_heap_mask(*dwa_image, 4);
#endif

      bmcv_dwa_gdc(dwaObj->mFrame->mHandle, *resized_img, *dwa_image, ldc_attr);

      dwaObj->mFrame->mSpData = resized_img;

      dwaObj->mFrame->mSpDataDwa = dwa_image;

      dwaObj->mFrame->mWidth = resized_img->width;
      dwaObj->mFrame->mHeight = resized_img->height;

      if (need_copy) bm_image_destroy(image_aligned);

    } else {  // dont need resize
      std::shared_ptr<bm_image> dwa_image = nullptr;
      dwa_image.reset(new bm_image, [](bm_image* p) {
        bm_image_destroy(*p);
        delete p;
        p = nullptr;
      });
      bm_status_t ret = bm_image_create(dwaObj->mFrame->mHandle,
                                        dwaObj->mFrame->mSpData->height,
                                        dwaObj->mFrame->mSpData->width, src_fmt,
                                        DATA_TYPE_EXT_1N_BYTE, dwa_image.get());
      bm_image_alloc_dev_mem(*dwa_image, 1);

      bm_image input;
      ret = bm_image_create(dwaObj->mFrame->mHandle,
                            dwaObj->mFrame->mSpData->height,
                            dwaObj->mFrame->mSpData->width, src_fmt,
                            dwaObj->mFrame->mSpData->data_type, &input, NULL);
      bm_image_alloc_dev_mem(input, 1);
      ret = bmcv_image_storage_convert(dwaObj->mFrame->mHandle, 1,
                                       dwaObj->mFrame->mSpData.get(), &input);

      bmcv_dwa_gdc(dwaObj->mFrame->mHandle, input, *dwa_image, ldc_attr);

      dwaObj->mFrame->mSpDataDwa = dwa_image;  // 是否需要dwa，效果不好

      bm_image_destroy(input);
    }
  }

  return common::ErrorCode::SUCCESS;
}

common::ErrorCode Dwa::doWork(int dataPipeId) {
  std::vector<int> inputPorts = getInputPorts();
  int inputPort = inputPorts[0];
  int outputPort = 0;
  if (!getSinkElementFlag()) {
    std::vector<int> outputPorts = getOutputPorts();
    int outputPort = outputPorts[0];
  }

  auto data = popInputData(inputPort, dataPipeId);
  while (!data && (getThreadStatus() == ThreadStatus::RUN)) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    data = popInputData(inputPort, dataPipeId);
  }
  if (data == nullptr) return common::ErrorCode::SUCCESS;

  auto objectMetadata = std::static_pointer_cast<common::ObjectMetadata>(data);

  if (objectMetadata->mFrame != nullptr &&
      objectMetadata->mFrame->mSpData != nullptr) {
    dwa_work(objectMetadata);
  }
  // usleep(10);
  int channel_id_internal = objectMetadata->mFrame->mChannelIdInternal;
  int outDataPipeId =
      getSinkElementFlag()
          ? 0
          : (channel_id_internal % getOutputConnectorCapacity(outputPort));
  common::ErrorCode errorCode =
      pushOutputData(outputPort, outDataPipeId,
                     std::static_pointer_cast<void>(objectMetadata));
  if (common::ErrorCode::SUCCESS != errorCode) {
    IVS_WARN(
        "Send data fail, element id: {0:d}, output port: {1:d}, data: "
        "{2:p}",
        getId(), outputPort, static_cast<void*>(objectMetadata.get()));
  }
  return common::ErrorCode::SUCCESS;
}

REGISTER_WORKER("dwa", Dwa)
}  // namespace dwa
}  // namespace element
}  // namespace sophon_stream

#endif