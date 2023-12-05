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
#include "fisheye.h"
namespace sophon_stream {
namespace element {
namespace fisheye {

#define YUV_8BIT(y, u, v) \
  ((((y) & 0xff) << 16) | (((u) & 0xff) << 8) | ((v) & 0xff))

bm_status_t set_fish_default_param(bmcv_fisheye_attr_s* fisheye_attr) {
  fisheye_attr->bEnable = 1;
  fisheye_attr->bBgColor = 1;
  fisheye_attr->u32BgColor = YUV_8BIT(0, 128, 128);
  fisheye_attr->s32HorOffset = 512;
  fisheye_attr->s32VerOffset = 512;
  fisheye_attr->u32TrapezoidCoef = 0;
  fisheye_attr->s32FanStrength = 0;
  fisheye_attr->enMountMode = BMCV_FISHEYE_DESKTOP_MOUNT;
  fisheye_attr->enUseMode = BMCV_MODE_PANORAMA_360;
  fisheye_attr->enViewMode = BMCV_FISHEYE_VIEW_360_PANORAMA;
  fisheye_attr->u32RegionNum = 1;
  return BM_SUCCESS;
}

Fisheye::Fisheye() {}
Fisheye::~Fisheye() {}

common::ErrorCode Fisheye::initInternal(const std::string& json) {
  common::ErrorCode errorCode = common::ErrorCode::SUCCESS;
  auto configure = nlohmann::json::parse(json, nullptr, false);
  if (!configure.is_object()) {
    errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
  }

  auto is_gray = configure.find(CONFIG_INTERNAL_IS_GRAY_FILED)->get<bool>();

  src_fmt = is_gray ? FORMAT_GRAY : FORMAT_YUV420P;
  set_fish_default_param(&fisheye_attr);

  return common::ErrorCode::SUCCESS;
}

void Fisheye::fisheye_work(std::shared_ptr<common::ObjectMetadata> fisheyeObj) {
  if (fisheyeObj != nullptr) {
    std::shared_ptr<bm_image> fisheye_image = nullptr;
    fisheye_image.reset(new bm_image, [](bm_image* p) {
      bm_image_destroy(*p);
      delete p;
      p = nullptr;
    });

    bm_status_t ret =
        bm_image_create(fisheyeObj->mFrame->mHandle, 720, 1280, src_fmt,
                        DATA_TYPE_EXT_1N_BYTE, fisheye_image.get());
    bm_image_alloc_dev_mem(*fisheye_image, 1);

    bm_image input;
    ret = bm_image_create(fisheyeObj->mFrame->mHandle,
                          fisheyeObj->mFrame->mSpData->height,
                          fisheyeObj->mFrame->mSpData->width, src_fmt,
                          fisheyeObj->mFrame->mSpData->data_type, &input, NULL);
    bm_image_alloc_dev_mem(input, 1);
    ret = bmcv_image_storage_convert(fisheyeObj->mFrame->mHandle, 1,
                                     fisheyeObj->mFrame->mSpData.get(), &input);

    bmcv_dwa_fisheye(fisheyeObj->mFrame->mHandle, input, *fisheye_image,
                     fisheye_attr);

    fisheyeObj->mFrame->mSpDataDwa = fisheye_image;  // 是否需要dwa，效果不好
    bm_image_destroy(input);
  }
}

common::ErrorCode Fisheye::doWork(int dataPipeId) {
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
    fisheye_work(objectMetadata);
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

REGISTER_WORKER("fisheye", Fisheye)
}  // namespace fisheye
}  // namespace element
}  // namespace sophon_stream

#endif
