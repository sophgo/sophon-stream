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

#include "common/logger.h"
#include "element_factory.h"
#include "../../encode/include/encode.h"

namespace sophon_stream {
namespace element {
namespace osd {

const std::vector<std::vector<int>> colors = {
    {220, 20, 60},   {119, 11, 32},   {0, 0, 142},     {0, 0, 230},
    {106, 0, 228},   {0, 60, 100},    {0, 80, 100},    {0, 0, 70},
    {0, 0, 192},     {250, 170, 30},  {100, 170, 30},  {220, 220, 0},
    {175, 116, 175}, {250, 0, 30},    {165, 42, 42},   {255, 77, 255},
    {0, 226, 252},   {182, 182, 255}, {0, 82, 0},      {120, 166, 157},
    {110, 76, 0},    {174, 57, 255},  {199, 100, 0},   {72, 0, 118},
    {255, 179, 240}, {0, 125, 92},    {209, 0, 151},   {188, 208, 182},
    {0, 220, 176},   {255, 99, 164},  {92, 0, 73},     {133, 129, 255},
    {78, 180, 255},  {0, 228, 0},     {174, 255, 243}, {45, 89, 255},
    {134, 134, 103}, {145, 148, 174}, {255, 208, 186}, {197, 226, 255},
    {171, 134, 1},   {109, 63, 54},   {207, 138, 255}, {151, 0, 95},
    {9, 80, 61},     {84, 105, 51},   {74, 65, 105},   {166, 196, 102},
    {208, 195, 210}, {255, 109, 65},  {0, 143, 149},   {179, 0, 194},
    {209, 99, 106},  {5, 121, 0},     {227, 255, 205}, {147, 186, 208},
    {153, 69, 1},    {3, 95, 161},    {163, 255, 0},   {119, 0, 170},
    {0, 182, 199},   {0, 165, 120},   {183, 130, 88},  {95, 32, 0},
    {130, 114, 135}, {110, 129, 133}, {166, 74, 118},  {219, 142, 185},
    {79, 210, 114},  {178, 90, 62},   {65, 70, 15},    {127, 167, 115},
    {59, 105, 106},  {142, 108, 45},  {196, 172, 0},   {95, 54, 80},
    {128, 76, 255},  {201, 57, 1},    {246, 0, 122},   {191, 162, 208}};

void draw_det_result(bm_handle_t& handle, int classId,
                     std::vector<std::string>& class_names, float conf,
                     int left, int top, int width, int height, bm_image& frame,
                     bool put_text_flag) {
  int colors_num = colors.size();
  bmcv_rect_t rect;
  rect.start_x = left;
  rect.start_y = top;
  rect.crop_w = width;
  rect.crop_h = height;
  std::vector<int> color = colors[classId % colors_num];
  bmcv_image_draw_rectangle(handle, frame, 1, &rect, 3, color[0], color[1],
                            color[2]);
  if (put_text_flag) {
    std::string label = class_names[classId] + ":" + cv::format("%.2f", conf);
    int org_x = left;
    int org_y = top;
    if (org_y<20) org_y += 20;
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

void draw_track_result(bm_handle_t& handle, int track_id, int left, int top,
                       int width, int height, bm_image& frame,
                       bool put_text_flag) {
  int colors_num = colors.size();
  bmcv_rect_t rect;
  rect.start_x = left;
  rect.start_y = top;
  rect.crop_w = width;
  rect.crop_h = height;
  std::vector<int> color = colors[track_id % colors_num];
  bmcv_image_draw_rectangle(handle, frame, 1, &rect, 3, color[0], color[1],
                            color[2]);
  if (put_text_flag) {
    std::string label = std::to_string(track_id);

    int org_x = left;
    int org_y = top;
    if (org_y<20) org_y += 20;
    bmcv_point_t org = {org_x, org_y};
    bmcv_color_t bmcv_color = {255,0,0};
    int thickness = 2;
    float fontScale = 1;
    if (BM_SUCCESS != bmcv_image_put_text(handle, frame, label.c_str(), org,
                                          bmcv_color, fontScale, thickness)) {
      IVS_ERROR("bmcv put text error !!!");
    }
  }
}

constexpr const char* CONFIG_INTERNAL_OSD_TYPE_FIELD = "osd_type";
constexpr const char* CONFIG_INTERNAL_CLASS_NAMES_FIELD = "class_names";

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

    std::string osd_type =
        configure.find(CONFIG_INTERNAL_OSD_TYPE_FIELD)->get<std::string>();
    mOsdType = OsdType::UNKNOWN;
    if (osd_type == "DET") mOsdType = OsdType::DET;
    if (osd_type == "TRACK") mOsdType = OsdType::TRACK;

    if (mOsdType == OsdType::DET) {
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
  if (!getLastElementFlag()) {
    std::vector<int> outputPorts = getOutputPorts();
    int outputPort = outputPorts[0];
  }

  std::shared_ptr<void> data;
  while (getThreadStatus() == ThreadStatus::RUN) {
    data = getInputData(inputPort, dataPipeId);
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
      getLastElementFlag()
          ? 0
          : (channel_id_internal % getOutputConnector(outputPort)->getDataPipeCount());
  errorCode = pushOutputData(outputPort, outDataPipeId, objectMetadata);
  if (common::ErrorCode::SUCCESS != errorCode) {
    IVS_WARN(
        "Send data fail, element id: {0:d}, output port: {1:d}, data: "
        "{2:p}",
        getId(), outputPort, static_cast<void*>(objectMetadata.get()));
  }

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
  for (auto subObj : objectMetadata->mSubObjectMetadatas) {
    switch (mOsdType) {
      case OsdType::DET:

        draw_det_result(objectMetadata->mFrame->mHandle,
                        subObj->mDetectedObjectMetadata->mClassify, mClassNames,
                        subObj->mDetectedObjectMetadata->mScores[0],
                        subObj->mDetectedObjectMetadata->mBox.mX,
                        subObj->mDetectedObjectMetadata->mBox.mY,
                        subObj->mDetectedObjectMetadata->mBox.mWidth,
                        subObj->mDetectedObjectMetadata->mBox.mHeight,
                        *imageStorage, true);
        break;

      case OsdType::TRACK:
        draw_track_result(objectMetadata->mFrame->mHandle,
                          subObj->mTrackedObjectMetadata->mTrackId,
                          subObj->mDetectedObjectMetadata->mBox.mX,
                          subObj->mDetectedObjectMetadata->mBox.mY,
                          subObj->mDetectedObjectMetadata->mBox.mWidth,
                          subObj->mDetectedObjectMetadata->mBox.mHeight,
                          *imageStorage, true);
        break;
      default:
        IVS_WARN("osd_type not support");
    }
  }
  objectMetadata->mFrame->mSpDataOsd = imageStorage;
}

REGISTER_WORKER("osd", Osd)

}  // namespace osd
}  // namespace element
}  // namespace sophon_stream
