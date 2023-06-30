//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#include "osd.h"

#include <stdlib.h>

#include <chrono>
#include <fstream>
#include <nlohmann/json.hpp>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>

#include "../../encode/include/encode.h"
#include "common/logger.h"
#include "element_factory.h"

namespace sophon_stream {
namespace element {
namespace osd {

const std::vector<std::vector<int>> colors = {
    {0, 0, 0},    {128, 0, 0},   {0, 128, 0},    {128, 128, 0},
    {0, 0, 128},  {128, 0, 128}, {0, 128, 128},  {128, 128, 128},
    {64, 0, 0},   {192, 0, 0},   {64, 128, 0},   {192, 128, 0},
    {64, 0, 128}, {192, 0, 128}, {64, 128, 128}, {192, 128, 128},
    {0, 64, 0},   {128, 64, 0},  {0, 192, 0},    {128, 192, 0},
    {0, 64, 128}};

void draw_rec_result(bm_handle_t& handle,
                     std::shared_ptr<common::ObjectMetadata> objectMetadata,
                     std::vector<std::string>& class_names, bm_image& frame,
                     bool put_text_flag) {
  int colors_num = colors.size();
  std::map<int, std::vector<bmcv_rect_t>> rectsMap;
  if (objectMetadata->mSubObjectMetadatas.size() == 0) return;
  for (auto subObj : objectMetadata->mSubObjectMetadatas) {
    int subId = subObj->mSubId;
    auto detObj = objectMetadata->mDetectedObjectMetadatas[subId];
    bmcv_rect_t rect;
    rect.start_x = detObj->mBox.mX;
    rect.start_y = detObj->mBox.mY;
    rect.crop_w = detObj->mBox.mWidth;
    rect.crop_h = detObj->mBox.mHeight;
    int class_id = subObj->mRecognizedObjectMetadatas[0]->mTopKLabels[0];
    if (!rectsMap.count(class_id % colors_num)) {
      std::vector<bmcv_rect_t> rects;
      rects.push_back(rect);
      rectsMap[class_id % colors_num] = rects;
    } else {
      rectsMap[class_id % colors_num].push_back(rect);
    }
    if (put_text_flag) {
      std::string label = class_names[class_id];
      int org_x = detObj->mBox.mX;
      int org_y = detObj->mBox.mY;
      if (org_y < 20) org_y += 20;
      bmcv_point_t org = {org_x, org_y};
      bmcv_color_t bmcv_color = {255, 0, 0};
      int thickness = 2;
      float fontScale = 1;
      if (BM_SUCCESS != bmcv_image_put_text(handle, frame, label.c_str(), org,
                                            bmcv_color, fontScale, thickness)) {
        IVS_ERROR("bmcv put text error !!!");
      }
    }
  }
  for (auto& rect : rectsMap) {
    bmcv_image_draw_rectangle(handle, frame, rect.second.size(),
                              &rect.second[0], 3, colors[rect.first][0],
                              colors[rect.first][1], colors[rect.first][2]);
  }

  std::string filename =
      std::to_string(objectMetadata->mFrame->mChannelId) +
      "-" + std::to_string(objectMetadata->mFrame->mFrameId) + ".bmp";
  bm_image_write_to_bmp(frame, filename.c_str());
  return;
}

void draw_det_result(bm_handle_t& handle,
                     std::shared_ptr<common::ObjectMetadata> objectMetadata,
                     std::vector<std::string>& class_names, bm_image& frame,
                     bool put_text_flag) {
  int colors_num = colors.size();
  std::map<int, std::vector<bmcv_rect_t>> rectsMap;

  for (auto detObj : objectMetadata->mDetectedObjectMetadatas) {
    bmcv_rect_t rect;
    rect.start_x = detObj->mBox.mX;
    rect.start_y = detObj->mBox.mY;
    rect.crop_w = detObj->mBox.mWidth;
    rect.crop_h = detObj->mBox.mHeight;
    int class_id = detObj->mClassify;
    if (!rectsMap.count(class_id % colors_num)) {
      std::vector<bmcv_rect_t> rects;
      rects.push_back(rect);
      rectsMap[class_id % colors_num] = rects;
    } else {
      rectsMap[class_id % colors_num].push_back(rect);
    }
  }

  for (auto& rect : rectsMap) {
    bmcv_image_draw_rectangle(handle, frame, rect.second.size(),
                              &rect.second[0], 3, colors[rect.first][0],
                              colors[rect.first][1], colors[rect.first][2]);
  }

  if (put_text_flag) {
    for (auto detObj : objectMetadata->mDetectedObjectMetadatas) {
      std::string label = class_names[detObj->mClassify] + ":" +
                          cv::format("%.2f", detObj->mScores[0]);
      int org_x = detObj->mBox.mX;
      int org_y = detObj->mBox.mY;
      if (org_y < 20) org_y += 20;
      bmcv_point_t org = {org_x, org_y};
      bmcv_color_t bmcv_color = {255, 0, 0};
      int thickness = 2;
      float fontScale = 1;
      if (BM_SUCCESS != bmcv_image_put_text(handle, frame, label.c_str(), org,
                                            bmcv_color, fontScale, thickness)) {
        IVS_ERROR("bmcv put text error !!!");
      }
    }
  }
}

void draw_track_result(bm_handle_t& handle,
                       std::shared_ptr<common::ObjectMetadata> objectMetadata,
                       std::vector<std::string>& class_names, bm_image& frame,
                       bool put_text_flag) {
  int colors_num = colors.size();
  std::map<int, std::vector<bmcv_rect_t>> rectsMap;
  int idx = 0;
  for (auto detObj : objectMetadata->mDetectedObjectMetadatas) {
    bmcv_rect_t rect;
    rect.start_x = detObj->mBox.mX;
    rect.start_y = detObj->mBox.mY;
    rect.crop_w = detObj->mBox.mWidth;
    rect.crop_h = detObj->mBox.mHeight;
    int track_id = objectMetadata->mTrackedObjectMetadatas[idx]->mTrackId;
    if (!rectsMap.count(track_id % colors_num)) {
      std::vector<bmcv_rect_t> rects;
      rects.push_back(rect);
      rectsMap[track_id % colors_num] = rects;
    } else {
      rectsMap[track_id % colors_num].push_back(rect);
    }
    ++idx;
  }
  for (auto& rect : rectsMap) {
    bmcv_image_draw_rectangle(handle, frame, rect.second.size(),
                              &rect.second[0], 3, colors[rect.first][0],
                              colors[rect.first][1], colors[rect.first][2]);
  }

  if (put_text_flag) {
    idx = 0;
    for (auto detObj : objectMetadata->mDetectedObjectMetadatas) {
      std::string label = std::to_string(
          objectMetadata->mTrackedObjectMetadatas[idx]->mTrackId);
      int org_x = detObj->mBox.mX;
      int org_y = detObj->mBox.mY;
      if (org_y < 20) org_y += 20;
      bmcv_point_t org = {org_x, org_y};
      bmcv_color_t bmcv_color = {255, 0, 0};
      int thickness = 2;
      float fontScale = 1;
      if (BM_SUCCESS != bmcv_image_put_text(handle, frame, label.c_str(), org,
                                            bmcv_color, fontScale, thickness)) {
        IVS_ERROR("bmcv put text error !!!");
      }
      ++idx;
    }
  }
}

Osd::Osd() {}

Osd::~Osd() {}

common::ErrorCode Osd::initInternal(const std::string& json) {
  common::ErrorCode errorCode = common::ErrorCode::SUCCESS;
  do {
    // json是否正确
    auto configure = nlohmann::json::parse(json, nullptr, false);
    if (!configure.is_object()) {
      errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
      break;
    }
    mFpsProfiler.config("fps_osd", 100);
    std::string osd_type =
        configure.find(CONFIG_INTERNAL_OSD_TYPE_FIELD)->get<std::string>();
    mOsdType = OsdType::UNKNOWN;
    if (osd_type == "DET") mOsdType = OsdType::DET;
    if (osd_type == "TRACK") mOsdType = OsdType::TRACK;
    if (osd_type == "REC") mOsdType = OsdType::REC;

    if (mOsdType == OsdType::DET || mOsdType == OsdType::REC) {
      std::string class_names_file =
          configure.find(CONFIG_INTERNAL_CLASS_NAMES_FIELD)->get<std::string>();
      std::ifstream istream;
      istream.open(class_names_file);
      assert(istream.is_open());
      std::string line;
      while (std::getline(istream, line)) {
        line = line.substr(0, line.length() - 1);
        mClassNames.push_back(line);
      }
      istream.close();
    }

  } while (false);
  return errorCode;
}

void Osd::uninitInternal() {}

common::ErrorCode Osd::doWork(int dataPipeId) {
  common::ErrorCode errorCode = common::ErrorCode::SUCCESS;

  common::ObjectMetadatas objectMetadatas;
  std::vector<int> inputPorts = getInputPorts();
  int inputPort = inputPorts[0];
  int outputPort = 0;
  if (!getSinkElementFlag()) {
    std::vector<int> outputPorts = getOutputPorts();
    int outputPort = outputPorts[0];
  }

  std::shared_ptr<void> data;
  while (getThreadStatus() == ThreadStatus::RUN) {
    data = popInputData(inputPort, dataPipeId);
    if (!data) {
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
      continue;
    }
    break;
  }

  if (!data) return common::ErrorCode::SUCCESS;

  auto objectMetadata = std::static_pointer_cast<common::ObjectMetadata>(data);
  if (!(objectMetadata->mFrame->mEndOfStream)) {
    draw(objectMetadata);
  }

  int channel_id_internal = objectMetadata->mFrame->mChannelIdInternal;
  int outDataPipeId =
      getSinkElementFlag()
          ? 0
          : (channel_id_internal % getOutputConnectorCapacity(outputPort));
  errorCode = pushOutputData(outputPort, outDataPipeId, objectMetadata);
  if (common::ErrorCode::SUCCESS != errorCode) {
    IVS_WARN(
        "Send data fail, element id: {0:d}, output port: {1:d}, data: "
        "{2:p}",
        getId(), outputPort, static_cast<void*>(objectMetadata.get()));
  }
  mFpsProfiler.add(1);

  return common::ErrorCode::SUCCESS;
}

void Osd::draw(std::shared_ptr<common::ObjectMetadata> objectMetadata) {
  std::shared_ptr<bm_image> imageStorage;
  imageStorage.reset(new bm_image,
                     [&](bm_image* img) { bm_image_destroy(*img); });
  bm_image image = *(objectMetadata->mFrame->mSpData);
  bm_image_create(objectMetadata->mFrame->mHandle,
                  objectMetadata->mFrame->mHeight,
                  objectMetadata->mFrame->mWidth, FORMAT_YUV420P,
                  image.data_type, &(*imageStorage));
  bmcv_image_storage_convert(objectMetadata->mFrame->mHandle, 1, &image,
                             &(*imageStorage));
  switch (mOsdType) {
    case OsdType::DET:
      draw_det_result(objectMetadata->mFrame->mHandle, objectMetadata,
                      mClassNames, *imageStorage, false);
      break;

    case OsdType::TRACK:
      draw_track_result(objectMetadata->mFrame->mHandle, objectMetadata,
                        mClassNames, *imageStorage, false);
      break;

    case OsdType::REC:
      draw_rec_result(objectMetadata->mFrame->mHandle, objectMetadata,
                      mClassNames, *imageStorage, true);
    default:
      IVS_WARN("osd_type not support");
  }
  objectMetadata->mFrame->mSpDataOsd = imageStorage;
}

REGISTER_WORKER("osd", Osd)

}  // namespace osd
}  // namespace element
}  // namespace sophon_stream
