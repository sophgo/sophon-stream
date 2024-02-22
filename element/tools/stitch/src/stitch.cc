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
  return common::ErrorCode::SUCCESS;
}

common::ErrorCode Stitch::stitch_work(
    std::shared_ptr<bm_image> left_image, std::shared_ptr<bm_image> right_image,std::shared_ptr<common::ObjectMetadata> stitchObj) {
  auto start = std::chrono::high_resolution_clock::now();

  bm_status_t ret;

  std::shared_ptr<bm_image> stitch_image = nullptr;
  stitch_image.reset(new bm_image, [](bm_image* p) {
    bm_image_destroy(*p);
    delete p;
    p = nullptr;
  });
  ret = bm_image_create(
      stitchObj->mFrame->mHandle, left_image->height,
      left_image->width+right_image->width + 0, FORMAT_YUV444P,
      DATA_TYPE_EXT_1N_BYTE, stitch_image.get());
  bm_image_alloc_dev_mem(*stitch_image, 1);

  bm_device_mem_t dpu_mem[3];
  bm_image_get_device_mem(*stitch_image, dpu_mem);
  bm_memset_device(stitchObj->mFrame->mHandle, 0, dpu_mem[0]);
  bm_memset_device(stitchObj->mFrame->mHandle, 0, dpu_mem[1]);
  bm_memset_device(stitchObj->mFrame->mHandle, 0, dpu_mem[2]);

  int input_num = 2;
  bmcv_rect_t dst_crop[input_num];

  dst_crop[0] = {.start_x = 0,
                 .start_y = 0,
                 .crop_w = (unsigned int)left_image->width,
                 .crop_h = (unsigned int)left_image->height};
  dst_crop[1] = {
      .start_x = (unsigned int)left_image->width * 1 + 0,
      .start_y = 0,
      .crop_w = (unsigned int)right_image->width,
      .crop_h = (unsigned int)right_image->height};

  bm_image src_img[2] = {*left_image,
                         *right_image};
  bmcv_image_vpp_stitch(stitchObj->mFrame->mHandle, input_num, src_img, *stitch_image, dst_crop,
                        NULL);

  auto end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double, std::milli> duration = end - start;
  std::cout << "bmcv_image_vpp_stitch程序执行时间：" << duration.count()
            << " ms" << std::endl;

  stitchObj->mFrame->mSpData = stitch_image;

  stitchObj->mFrame->mWidth = stitchObj->mFrame->mSpData->width;
  stitchObj->mFrame->mHeight = stitchObj->mFrame->mSpData->height;

  return common::ErrorCode::SUCCESS;
}

common::ErrorCode Stitch::doWork(int dataPipeId) {
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
    stitch_work(objectMetadata->mFrame->mSpDataDwa,objectMetadata->mFrame->mSpDataDpu,objectMetadata);
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

REGISTER_WORKER("stitch", Stitch)
}  // namespace stitch
}  // namespace element
}  // namespace sophon_stream

#endif