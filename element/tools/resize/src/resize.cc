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

  dst_h = configure.find(CONFIG_INTERNAL_DST_H_FILED)->get<int>();
  dst_w = configure.find(CONFIG_INTERNAL_DST_W_FILED)->get<int>();
  crop_top = configure.find(CONFIG_INTERNAL_CROP_TOP_FILED)->get<int>();
  crop_left = configure.find(CONFIG_INTERNAL_CROP_LEFT_FILED)->get<int>();
  crop_h = configure.find(CONFIG_INTERNAL_CROP_H_FILED)->get<int>();
  crop_w = configure.find(CONFIG_INTERNAL_CROP_W_FILED)->get<int>();
  
  

  return common::ErrorCode::SUCCESS;
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
        bm_image_create(resObj->mFrame->mHandle, dst_h, dst_w, FORMAT_YUV420P,
                        DATA_TYPE_EXT_1N_BYTE, resize_image.get());
    bm_image_alloc_dev_mem(*resize_image, 1);


    bmcv_rect_t crop_rect{crop_left, crop_top,
                          (unsigned int)crop_w,
                          (unsigned int)crop_h};
    bmcv_padding_atrr_t padding_attr;
    memset(&padding_attr, 0, sizeof(padding_attr));
    padding_attr.dst_crop_sty =0;
    padding_attr.dst_crop_stx =0;
    padding_attr.padding_b = 114;
    padding_attr.padding_g = 114;
    padding_attr.padding_r = 114;
    padding_attr.if_memset = 1;
    padding_attr.dst_crop_h = (unsigned int)dst_h;
    padding_attr.dst_crop_w = (unsigned int)dst_w;

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
    outputPort = outputPorts[0];
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
