//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//
#include "common/common_defs.h"

#include <unordered_map>

#include "common/logger.h"
#include "element_factory.h"
#include "stitch.h"
namespace sophon_stream {
namespace element {
namespace stitch {
Stitch::Stitch() {}
Stitch::~Stitch() {}

common::ErrorCode Stitch::initInternal(const std::string& json) {
  common::ErrorCode errorCode = common::ErrorCode::SUCCESS;
  auto configure = nlohmann::json::parse(json, nullptr, false);
  if (!configure.is_object()) {
    errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
  }
  mFpsProfiler.config("stitch fps:", 100);
  stitch_mode = configure.find(CONFIG_INTERNAL_STITCH_MODE_FILED)->get<std::string>();
  

  return common::ErrorCode::SUCCESS;
}

common::ErrorCode Stitch::stitch_work(std::shared_ptr<common::ObjectMetadata> leftObj,
    std::shared_ptr<common::ObjectMetadata> rightObj
    ,std::shared_ptr<common::ObjectMetadata> stitchObj) {

  bm_status_t ret;
  std::shared_ptr<bm_image> stitch_image = nullptr;
  stitch_image.reset(new bm_image, [](bm_image* p) {
    bm_image_destroy(*p);
    delete p;
    p = nullptr;
  });
  int dst_h,dst_w;
  int input_num = 2;
  bmcv_rect_t dst_crop[input_num];

  if (stitch_mode == "HORIZONTAL") {
    dst_h = std::max(leftObj->mFrame->mSpData->height, rightObj->mFrame->mSpData->height);
    dst_w = leftObj->mFrame->mSpData->width + rightObj->mFrame->mSpData->width;

    dst_crop[0] = {.start_x = 0,
                 .start_y = 0,
                 .crop_w = (unsigned int)leftObj->mFrame->mSpData->width,
                 .crop_h = (unsigned int)leftObj->mFrame->mSpData->height};
    dst_crop[1] = {
        .start_x = (unsigned int)leftObj->mFrame->mSpData->width,
        .start_y = 0,
        .crop_w = (unsigned int)rightObj->mFrame->mSpData->width,
        .crop_h = (unsigned int)rightObj->mFrame->mSpData->height};

  }else if (stitch_mode == "VERTICAL") {
    dst_h = leftObj->mFrame->mSpData->height + rightObj->mFrame->mSpData->height;
    dst_w = std::max(leftObj->mFrame->mSpData->width, rightObj->mFrame->mSpData->width);

    dst_crop[0] = {.start_x = 0,
                 .start_y = 0,
                 .crop_w = (unsigned int)leftObj->mFrame->mSpData->width,
                 .crop_h = (unsigned int)leftObj->mFrame->mSpData->height};
    dst_crop[1] = {
        .start_x = 0,
        .start_y = (unsigned int)leftObj->mFrame->mSpData->height,
        .crop_w = (unsigned int)rightObj->mFrame->mSpData->width,
        .crop_h = (unsigned int)rightObj->mFrame->mSpData->height};
  }else{
    IVS_ERROR("only support HORIZONTAL and VERTICAL stitch mode");
  }
  ret = bm_image_create(
      stitchObj->mFrame->mHandle, dst_h, dst_w, leftObj->mFrame->mSpData->image_format, DATA_TYPE_EXT_1N_BYTE, stitch_image.get());
  bm_image_alloc_dev_mem(*stitch_image, 1);

  bm_image src_img[2] = {*leftObj->mFrame->mSpData,*rightObj->mFrame->mSpData};
  bmcv_image_vpp_stitch(stitchObj->mFrame->mHandle, input_num, src_img, *stitch_image, dst_crop,NULL);

  stitchObj->mFrame->mSpData = stitch_image;
  stitchObj->mFrame->mWidth = stitchObj->mFrame->mSpData->width;
  stitchObj->mFrame->mHeight = stitchObj->mFrame->mSpData->height;
  stitchObj->mFrame->mChannelId = leftObj->mFrame->mChannelId;
  stitchObj->mFrame->mFrameId = leftObj->mFrame->mFrameId;


  return common::ErrorCode::SUCCESS;
}

common::ErrorCode Stitch::doWork(int dataPipeId) {
  std::vector<int> inputPorts = getInputPorts();
  int outputPort = 0;
  if (!getSinkElementFlag()) {
    std::vector<int> outputPorts = getOutputPorts();
    outputPort = outputPorts[0];
  }

  common::ObjectMetadatas inputs;

  for (auto inputPort : inputPorts) {
    auto data = popInputData(inputPort, dataPipeId);
    while (!data && (getThreadStatus() == ThreadStatus::RUN)) {
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
      data = popInputData(inputPort, dataPipeId);
    }
    if (data == nullptr) return common::ErrorCode::SUCCESS;

    auto objectMetadata =
        std::static_pointer_cast<common::ObjectMetadata>(data);
    inputs.emplace_back(objectMetadata);
    IVS_DEBUG("Got Input, port id = {0}, channel_id = {1}, frame_id = {2}",
              inputPort, objectMetadata->mFrame->mChannelId,
              objectMetadata->mFrame->mFrameId);
  }

  if (inputs[0]->mFrame->mSpData != nullptr &&
      inputs[1]->mFrame->mSpData != nullptr &&
      inputs[0]->mFrame->mFrameId == inputs[1]->mFrame->mFrameId) {
    std::shared_ptr<common::ObjectMetadata> stitchObj =
        std::make_shared<common::ObjectMetadata>();
    stitchObj->mFrame = std::make_shared<sophon_stream::common::Frame>();

    stitch_work(inputs[0], inputs[1], stitchObj);

    mFpsProfiler.add(1);

    int channel_id_internal = stitchObj->mFrame->mChannelIdInternal;
    int outDataPipeId =
        getSinkElementFlag()
            ? 0
            : (channel_id_internal % getOutputConnectorCapacity(outputPort));
    common::ErrorCode errorCode = pushOutputData(
        outputPort, outDataPipeId, std::static_pointer_cast<void>(stitchObj));
    if (common::ErrorCode::SUCCESS != errorCode) {
      IVS_WARN(
          "Send data fail, element id: {0:d}, output port: {1:d}, data: "
          "{2:p}",
          getId(), outputPort, static_cast<void*>(stitchObj.get()));
    }
  }

  return common::ErrorCode::SUCCESS;
}

REGISTER_WORKER("stitch", Stitch)
}  // namespace stitch
}  // namespace element
}  // namespace sophon_stream
