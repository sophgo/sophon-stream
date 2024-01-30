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

#include <chrono>
#include <nlohmann/json.hpp>

#include "common/logger.h"
#include "element_factory.h"
#include "resize.h"
namespace sophon_stream {
namespace element {
namespace resize {

Resize::Resize() {}
Resize::~Resize() {}

common::ErrorCode Resize::initInternal(const std::string& json) {
  common::ErrorCode errorCode = common::ErrorCode::SUCCESS;
  auto configure = nlohmann::json::parse(json, nullptr, false);
  if (!configure.is_object()) {
    errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
  }

  mFpsProfiler.config("Resize fps:", 100);

  ratio = configure.find(CONFIG_INTERNAL_RATIO_FILED)->get<float>();
  if (ratio == 0 && ratio == 1) {
    dst_h = configure.find(CONFIG_INTERNAL_DST_H_FILED)->get<int>();
    dst_w = configure.find(CONFIG_INTERNAL_DST_W_FILED)->get<int>();
    ratio = (float)dst_w / dst_h;
  }
  

  return common::ErrorCode::SUCCESS;
}

float Resize::get_aspect_scaled_ratio(int src_w, int src_h, int dst_w,
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

common::ErrorCode Resize::resize_work(
    std::shared_ptr<common::ObjectMetadata> resObj) {

  if (resObj != nullptr) {
    std::shared_ptr<bm_image> resize_image = nullptr;
    resize_image.reset(new bm_image, [](bm_image* p) {
      bm_image_destroy(*p);
      delete p;
      p = nullptr;
    });

    bm_status_t ret =
        bm_image_create(resObj->mFrame->mHandle, resObj->mFrame->mSpData->height*ratio, resObj->mFrame->mSpData->width*ratio, FORMAT_RGB_PLANAR,
                        DATA_TYPE_EXT_1N_BYTE, resize_image.get());
    bm_image_alloc_dev_mem(*resize_image, 1);


    bmcv_rect_t crop_rect{0, 0,
                          (unsigned int)resObj->mFrame->mSpData->width,
                          (unsigned int)resObj->mFrame->mSpData->height};
    bmcv_padding_atrr_t padding_attr;
    memset(&padding_attr, 0, sizeof(padding_attr));
    padding_attr.dst_crop_sty =0;
    padding_attr.dst_crop_stx =0;
    padding_attr.padding_b = 114;
    padding_attr.padding_g = 114;
    padding_attr.padding_r = 114;
    padding_attr.if_memset = 1;
    padding_attr.dst_crop_h = (unsigned int)resObj->mFrame->mSpData->height*ratio;
    padding_attr.dst_crop_w = (unsigned int)resObj->mFrame->mSpData->width*ratio;

    ret = bmcv_image_vpp_convert_padding(resObj->mFrame->mHandle, 1,
                                         *resObj->mFrame->mSpData, resize_image.get(),
                                         &padding_attr, &crop_rect);

    resObj->mFrame->mSpData = resize_image;  

    resObj->mFrame->mWidth = resObj->mFrame->mSpData->width;
    resObj->mFrame->mHeight = resObj->mFrame->mSpData->height;
  }
}

common::ErrorCode Resize::doWork(int dataPipeId) {
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
    resize_work(objectMetadata);
  }
  mFpsProfiler.add(1);
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

REGISTER_WORKER("resize", Resize)
}  // namespace resize
}  // namespace element
}  // namespace sophon_stream

#endif