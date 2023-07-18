//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_ELEMENT_OSD_DRAW_UTILS_H_
#define SOPHON_STREAM_ELEMENT_OSD_DRAW_UTILS_H_

#include <stdlib.h>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>

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

void draw_bmcv_det_result(
    bm_handle_t& handle, std::shared_ptr<common::ObjectMetadata> objectMetadata,
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

void draw_bmcv_track_result(
    bm_handle_t& handle, std::shared_ptr<common::ObjectMetadata> objectMetadata,
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

void draw_opencv_det_result(
    std::shared_ptr<common::ObjectMetadata> objectMetadata,
    std::vector<std::string>& class_names, cv::Mat& frame, bool put_text_flag) {
  // Draw a rectangle displaying the bounding box
  int colors_num = colors.size();
  int thickness = 2;
  float fontScale = 1;
  for (auto detObj : objectMetadata->mDetectedObjectMetadatas) {
    int classId = detObj->mClassify;
    cv::Scalar color(colors[classId % colors_num][0],
                     colors[classId % colors_num][1],
                     colors[classId % colors_num][2]);
    cv::rectangle(frame, cv::Point(detObj->mBox.mX, detObj->mBox.mY),
                  cv::Point(detObj->mBox.mX + detObj->mBox.mWidth,
                            detObj->mBox.mY + detObj->mBox.mHeight),
                  color, thickness);

    if (put_text_flag) {
      std::string label =
          class_names[classId] + ":" +
          cv::format("%.2f", detObj->mScores[0]);  // Display the label at the
                                                   // top of the bounding box
      int baseLine;
      cv::Size labelSize =
          getTextSize(label, cv::FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseLine);
      cv::putText(frame, label,
                  cv::Point(detObj->mBox.mX,
                            std::max(detObj->mBox.mY, labelSize.height) - 5),
                  cv::FONT_HERSHEY_SIMPLEX, fontScale, color, thickness);
    }
  }
}

void draw_opencv_track_result(
    std::shared_ptr<common::ObjectMetadata> objectMetadata,
    std::vector<std::string>& class_names, cv::Mat& frame, bool put_text_flag) {
  // Draw a rectangle displaying the bounding box
  int colors_num = colors.size();
  int thickness = 2;
  float fontScale = 1;
  int idx = 0;
  for (auto detObj : objectMetadata->mDetectedObjectMetadatas) {
    int track_id = objectMetadata->mTrackedObjectMetadatas[idx]->mTrackId;
    cv::Scalar color(colors[track_id % colors_num][0],
                     colors[track_id % colors_num][1],
                     colors[track_id % colors_num][2]);
    cv::rectangle(frame, cv::Point(detObj->mBox.mX, detObj->mBox.mY),
                  cv::Point(detObj->mBox.mX + detObj->mBox.mWidth,
                            detObj->mBox.mY + detObj->mBox.mHeight),
                  color, thickness);

    if (put_text_flag) {
      std::string label = std::to_string(
          objectMetadata->mTrackedObjectMetadatas[idx]->mTrackId);
      int baseLine;
      cv::Size labelSize =
          getTextSize(label, cv::FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseLine);
      cv::putText(frame, label,
                  cv::Point(detObj->mBox.mX,
                            std::max(detObj->mBox.mY, labelSize.height) - 5),
                  cv::FONT_HERSHEY_SIMPLEX, fontScale, color, thickness);
    }
    ++idx;
  }
}

}  // namespace osd
}  // namespace element
}  // namespace sophon_stream

#endif  // SOPHON_STREAM_ELEMENT_OSD_DRAW_UTILS_H_