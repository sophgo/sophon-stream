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
#include <sys/stat.h>

#include <codecvt>
#include <fstream>
#include <mutex>
#include <nlohmann/json.hpp>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp>
#include <unordered_map>

#include "common/clocker.h"
#include "common/common_defs.h"
#include "common/error_code.h"
#include "common/logger.h"
#include "common/object_metadata.h"
#include "common/posed_object_metadata.h"
#include "cvUniText.h"
#include "element_factory.h"
extern "C" {
extern bm_status_t bmcv_image_overlay(bm_handle_t handle, bm_image image,
                                      int overlay_num,
                                      bmcv_rect_t* overlay_info,
                                      bm_image* overlay_image)
    __attribute__((weak));
}
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

const std::vector<float> pose_colors = {
    255.f, 0.f,   0.f,   255.f, 85.f,  0.f,   255.f, 170.f, 0.f,   255.f, 255.f,
    0.f,   170.f, 255.f, 0.f,   85.f,  255.f, 0.f,   0.f,   255.f, 0.f,   0.f,
    255.f, 85.f,  0.f,   255.f, 170.f, 0.f,   255.f, 255.f, 0.f,   170.f, 255.f,
    0.f,   85.f,  255.f, 0.f,   0.f,   255.f, 85.f,  0.f,   255.f, 170.f, 0.f,
    255.f, 255.f, 0.f,   255.f, 255.f, 0.f,   170.f, 255.f, 0.f,   85.f,  255.f,
    0.f,   0.f,   255.f, 0.f,   255.f, 255.f, 85.f,  255.f, 255.f, 170.f, 255.f,
    255.f, 255.f, 255.f, 170.f, 255.f, 255.f, 85.f,  255.f, 255.f};

std::vector<unsigned int> getPosePairs(
    common::PosedObjectMetadata::EModelType model_type) {
  switch (model_type) {
    case common::PosedObjectMetadata::EModelType::BODY_25:
      return {1,  8,  1,  2,  1,  5,  2,  3,  3,  4,  5,  6,  6,
              7,  8,  9,  9,  10, 10, 11, 8,  12, 12, 13, 13, 14,
              1,  0,  0,  15, 15, 17, 0,  16, 16, 18, 2,  17, 5,
              18, 14, 19, 19, 20, 14, 21, 11, 22, 22, 23, 11, 24};
    case common::PosedObjectMetadata::EModelType::COCO_18:
      return {1, 2,  1,  5,  2,  3,  3,  4,  5,  6,  6,  7, 1,
              8, 8,  9,  9,  10, 1,  11, 11, 12, 12, 13, 1, 0,
              0, 14, 14, 16, 0,  15, 15, 17, 2,  16, 5,  17};
    default:
      // COCO_18
      return {1, 2,  1,  5,  2,  3,  3,  4,  5,  6,  6,  7, 1,
              8, 8,  9,  9,  10, 1,  11, 11, 12, 12, 13, 1, 0,
              0, 14, 14, 16, 0,  15, 15, 17, 2,  16, 5,  17};
  }
}

template <typename T>
inline int intRound(const T a) {
  return int(a + 0.5f);
}

// 抽帧检测时缓存结果
std::map<int, std::shared_ptr<common::ObjectMetadata>> lastObjectMetadataMap;
std::mutex mLastObjectMetaDataMtx;

void draw_bmcv_det_result(
    bm_handle_t& handle, std::shared_ptr<common::ObjectMetadata> objectMetadata,
    std::vector<std::string>& class_names, bm_image& frame,
    bool put_text_flag, bool draw_interval) {
  int colors_num = colors.size();
  std::map<int, std::vector<bmcv_rect_t>> rectsMap;
  int thickness = 2;
  float fontScale = 1;
  std::shared_ptr<common::ObjectMetadata> objData;
  {
    std::lock_guard<std::mutex> lk(mLastObjectMetaDataMtx);
    objData = (objectMetadata->mFilter && draw_interval)
                  ? lastObjectMetadataMap[objectMetadata->mFrame->mChannelId]
                  : objectMetadata;
    lastObjectMetadataMap[objectMetadata->mFrame->mChannelId] = objData;
  }
  for (auto detObj : objData->mDetectedObjectMetadatas) {
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
                              &rect.second[0], thickness, colors[rect.first][0],
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
      if (BM_SUCCESS != bmcv_image_put_text(handle, frame, label.c_str(), org,
                                            bmcv_color, fontScale, thickness)) {
        IVS_ERROR("bmcv put text error !!!");
      }
    }
  }
}

void draw_bmcv_track_result(
    bm_handle_t& handle, std::shared_ptr<common::ObjectMetadata> objectMetadata,
    std::vector<std::string>& class_names, bm_image& frame, bool put_text_flag,
    bool draw_interval) {
  int colors_num = colors.size();
  std::map<int, std::vector<bmcv_rect_t>> rectsMap;
  int track_id;
  int thickness = 2;
  float fontScale = 1;
  int idx = 0;
  std::shared_ptr<common::ObjectMetadata> objData;
  {
    std::lock_guard<std::mutex> lk(mLastObjectMetaDataMtx);
    objData = (objectMetadata->mFilter && draw_interval)
                  ? lastObjectMetadataMap[objectMetadata->mFrame->mChannelId]
                  : objectMetadata;
    lastObjectMetadataMap[objectMetadata->mFrame->mChannelId] = objData;
  }

  for (auto detObj : objData->mDetectedObjectMetadatas) {
    bmcv_rect_t rect;
    rect.start_x = detObj->mBox.mX;
    rect.start_y = detObj->mBox.mY;
    rect.crop_w = detObj->mBox.mWidth;
    rect.crop_h = detObj->mBox.mHeight;
    int track_id = objData->mTrackedObjectMetadatas[idx]->mTrackId;
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
                              &rect.second[0], thickness, colors[rect.first][0],
                              colors[rect.first][1], colors[rect.first][2]);
  }

  if (put_text_flag) {
    idx = 0;
    for (auto detObj : objData->mDetectedObjectMetadatas) {
      std::string label =
          std::to_string(objData->mTrackedObjectMetadatas[idx]->mTrackId);
      int org_x = detObj->mBox.mX;
      int org_y = detObj->mBox.mY;
      if (org_y < 20) org_y += 20;
      bmcv_point_t org = {org_x, org_y};
      bmcv_color_t bmcv_color = {255, 0, 0};
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
    std::vector<std::string>& class_names, cv::Mat& frame, bool put_text_flag,
    bool draw_interval) {
  // Draw a rectangle displaying the bounding box
  int colors_num = colors.size();
  int thickness = 2;
  float fontScale = 1;
  std::shared_ptr<common::ObjectMetadata> objData;
  {
    std::lock_guard<std::mutex> lk(mLastObjectMetaDataMtx);
    objData = (objectMetadata->mFilter && draw_interval)
                  ? lastObjectMetadataMap[objectMetadata->mFrame->mChannelId]
                  : objectMetadata;
    lastObjectMetadataMap[objectMetadata->mFrame->mChannelId] = objData;
  }
  for (auto detObj : objData->mDetectedObjectMetadatas) {
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
    std::vector<std::string>& class_names, cv::Mat& frame, bool put_text_flag,
    bool draw_interval) {
  // Draw a rectangle displaying the bounding box
  int colors_num = colors.size();
  int thickness = 2;
  float fontScale = 1;
  int track_id;
  int idx = 0;
  std::shared_ptr<common::ObjectMetadata> objData;
  {
    std::lock_guard<std::mutex> lk(mLastObjectMetaDataMtx);
    objData = (objectMetadata->mFilter && draw_interval)
                  ? lastObjectMetadataMap[objectMetadata->mFrame->mChannelId]
                  : objectMetadata;
    lastObjectMetadataMap[objectMetadata->mFrame->mChannelId] = objData;
  }

  for (auto detObj : objData->mDetectedObjectMetadatas) {
    int track_id = objData->mTrackedObjectMetadatas[idx]->mTrackId;
    cv::Scalar color(colors[track_id % colors_num][0],
                     colors[track_id % colors_num][1],
                     colors[track_id % colors_num][2]);
    cv::rectangle(frame, cv::Point(detObj->mBox.mX, detObj->mBox.mY),
                  cv::Point(detObj->mBox.mX + detObj->mBox.mWidth,
                            detObj->mBox.mY + detObj->mBox.mHeight),
                  color, thickness);

    if (put_text_flag) {
      std::string label = std::to_string(track_id);
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

void draw_bmcv_pose_result(
    bm_handle_t& handle, std::shared_ptr<common::ObjectMetadata> objectMetadata,
    bm_image& frame, bool draw_interval) {
  // Parameters
  const auto numberColors = pose_colors.size();
  const float threshold = 0.05;
  const float scale = 1.0;
  const auto thicknessLine = 2;

  std::shared_ptr<common::ObjectMetadata> objData;
  {
    std::lock_guard<std::mutex> lk(mLastObjectMetaDataMtx);
    objData = (objectMetadata->mFilter && draw_interval)
                  ? lastObjectMetadataMap[objectMetadata->mFrame->mChannelId]
                  : objectMetadata;
    lastObjectMetadataMap[objectMetadata->mFrame->mChannelId] = objData;
  }

  for (auto poseObj : objData->mPosedObjectMetadatas) {
    const std::vector<float>& poseKeypoints = poseObj->keypoints;
    const auto& pairs = getPosePairs(poseObj->modeltype);

    // Draw lines
    for (auto pair = 0u; pair < pairs.size(); pair += 2) {
      const auto index1 = (pairs[pair]) * 3;
      const auto index2 = (pairs[pair + 1]) * 3;

      if (poseKeypoints[index1 + 2] > threshold &&
          poseKeypoints[index2 + 2] > threshold) {
        const auto colorIndex = pairs[pair + 1] * 3;
        bmcv_color_t color = {pose_colors[(colorIndex + 2) % numberColors],
                              pose_colors[(colorIndex + 1) % numberColors],
                              pose_colors[(colorIndex + 0) % numberColors]};
        bmcv_point_t start = {intRound(poseKeypoints[index1] * scale),
                              intRound(poseKeypoints[index1 + 1] * scale)};
        bmcv_point_t end = {intRound(poseKeypoints[index2] * scale),
                            intRound(poseKeypoints[index2 + 1] * scale)};

        if (BM_SUCCESS != bmcv_image_draw_lines(handle, frame, &start, &end, 1,
                                                color, thicknessLine)) {
          std::cout << "bmcv draw lines error !!!" << std::endl;
        }
      }
    }
  }
}

void draw_opencv_pose_result(
    bm_handle_t& handle, std::shared_ptr<common::ObjectMetadata> objectMetadata,
    cv::Mat& frame, bool draw_interval) {
  // Parameters
  const auto numberColors = pose_colors.size();
  const float threshold = 0.05;
  const float scale = 1.0;
  const auto thicknessLine = 2;

  std::shared_ptr<common::ObjectMetadata> objData;
  {
    std::lock_guard<std::mutex> lk(mLastObjectMetaDataMtx);
    objData = (objectMetadata->mFilter && draw_interval)
                  ? lastObjectMetadataMap[objectMetadata->mFrame->mChannelId]
                  : objectMetadata;
    lastObjectMetadataMap[objectMetadata->mFrame->mChannelId] = objData;
  }

  for (auto poseObj : objData->mPosedObjectMetadatas) {
    const std::vector<float>& poseKeypoints = poseObj->keypoints;
    const auto& pairs = getPosePairs(poseObj->modeltype);

    // Draw lines
    for (auto pair = 0u; pair < pairs.size(); pair += 2) {
      const auto index1 = (pairs[pair]) * 3;
      const auto index2 = (pairs[pair + 1]) * 3;
      if (poseKeypoints[index1 + 2] > threshold &&
          poseKeypoints[index2 + 2] > threshold) {
        const auto colorIndex = pairs[pair + 1] * 3;
        const cv::Scalar color{pose_colors[(colorIndex + 2) % numberColors],
                               pose_colors[(colorIndex + 1) % numberColors],
                               pose_colors[(colorIndex + 0) % numberColors]};
        const cv::Point keypoint1{intRound(poseKeypoints[index1] * scale),
                                  intRound(poseKeypoints[index1 + 1] * scale)};

        const cv::Point keypoint2{intRound(poseKeypoints[index2] * scale),
                                  intRound(poseKeypoints[index2 + 1] * scale)};
        cv::line(frame, keypoint1, keypoint2, color, thicknessLine, 8, 0);
      }
    }
  }
}
void draw_opencv_areas(std::shared_ptr<common::ObjectMetadata> objectMetadata,
                       cv::Mat& frame_to_draw) {
  for (int i = 0; i < objectMetadata->areas.size(); i++) {
    if (objectMetadata->areas[i].size() == 2) {
      const cv::Point start = {objectMetadata->areas[i][0].mY,
                               objectMetadata->areas[i][0].mX};
      const cv::Point end = {objectMetadata->areas[i][1].mY,
                             objectMetadata->areas[i][1].mX};
      const cv::Scalar color = {255, 0, 0};
      cv::line(frame_to_draw, start, end, color, 3, 8, 0);
    }
  }
}
void draw_bmcv_areas(std::shared_ptr<common::ObjectMetadata> objectMetadata,
                     bm_image& frame_to_draw) {
  for (int i = 0; i < objectMetadata->areas.size(); i++) {
    if (objectMetadata->areas[i].size() == 2) {
      bmcv_point_t start = {objectMetadata->areas[i][0].mY,
                            objectMetadata->areas[i][0].mX};
      bmcv_point_t end = {objectMetadata->areas[i][1].mY,
                          objectMetadata->areas[i][1].mX};
      bmcv_color_t color = {255, 0, 0};
      bmcv_image_draw_lines(objectMetadata->mFrame->mHandle, frame_to_draw,
                            &start, &end, 1, color, 3);
    }
  }
}

#define POSE_COLORS_RENDER_CPU                                                 \
  255.f, 0.f, 0.f, 255.f, 85.f, 0.f, 255.f, 170.f, 0.f, 255.f, 255.f, 0.f,     \
      170.f, 255.f, 0.f, 85.f, 255.f, 0.f, 0.f, 255.f, 0.f, 0.f, 255.f, 85.f,  \
      0.f, 255.f, 170.f, 0.f, 255.f, 255.f, 0.f, 170.f, 255.f, 0.f, 85.f,      \
      255.f, 0.f, 0.f, 255.f, 85.f, 0.f, 255.f, 170.f, 0.f, 255.f, 255.f, 0.f, \
      255.f, 255.f, 0.f, 170.f, 255.f, 0.f, 85.f, 255.f, 0.f, 0.f, 255.f, 0.f, \
      255.f, 255.f, 85.f, 255.f, 255.f, 170.f, 255.f, 255.f, 255.f, 255.f,     \
      170.f, 255.f, 255.f, 85.f, 255.f, 255.f

const std::vector<float> POSE_COLORS_RENDER{POSE_COLORS_RENDER_CPU};

// Max/min functions
template <typename T>
static T fastMax(const T a, const T b) {
  return (a > b ? a : b);
}

static void _draw_rectangle_and_text_bmcv(
    bm_handle_t& handle, std::string& lable, int left, int top, int width,
    int height, bm_image& frame, const std::vector<int>& color,
    bool put_text_flag)  // Draw the predicted bounding box
{
  // Draw a rectangle displaying the bounding box
  bmcv_rect_t rect;
  rect.start_x = left;
  rect.start_y = top;
  rect.crop_w = width;
  rect.crop_h = height;
  std::cout << rect.start_x << "," << rect.start_y << "," << rect.crop_w << ","
            << rect.crop_h << std::endl;
  int ret = bmcv_image_draw_rectangle(handle, frame, 1, &rect, 3, color[0],
                                      color[1], color[2]);
  if (put_text_flag) {
    bmcv_point_t org = {left, top - 10};
    bmcv_color_t bmcv_color = {color[0], color[1], color[2]};
    int thickness = 2;
    float fontScale = 1.5;

    if (BM_SUCCESS != bmcv_image_put_text(handle, frame, lable.c_str(), org,
                                          bmcv_color, fontScale, thickness)) {
      std::cout << "bmcv put text error !!!" << std::endl;
    }
  }
}

static void _draw_face_rectangle_bmcv(
    bm_handle_t& handle,
    std::shared_ptr<sophon_stream::common::ObjectMetadata> results,
    bm_image& frame)  // Draw the predicted bounding box
{
  // Draw a rectangle displaying the bounding box
  bmcv_rect_t rect;
  for (size_t j = 0; j < results->mFaceObjectMetadatas.size(); j++) {
    rect.start_x = results->mFaceObjectMetadatas[j]->left;
    rect.start_y = results->mFaceObjectMetadatas[j]->top;
    rect.crop_w = results->mFaceObjectMetadatas[j]->right -
                  results->mFaceObjectMetadatas[j]->left + 1;
    rect.crop_h = results->mFaceObjectMetadatas[j]->bottom -
                  results->mFaceObjectMetadatas[j]->top + 1;

    std::cout << rect.start_x << "," << rect.start_y << "," << rect.crop_w
              << "," << rect.crop_h << std::endl;

    bmcv_image_draw_rectangle(handle, frame, 1, &rect, 3, 255, 2, 2);
  }
}

static void _draw_text_bmcv(
    bm_handle_t& handle, int left, int top, bm_image& frame,
    std::string label)  // Draw the predicted bounding box
{
  bmcv_point_t org = {left, top + 40};
  bmcv_color_t bmcv_color = {255, 0, 0};
  int thickness = 2;
  float fontScale = 1.5;
  if (BM_SUCCESS != bmcv_image_put_text(handle, frame, label.c_str(), org,
                                        bmcv_color, fontScale, thickness)) {
    std::cout << "bmcv put text error !!!" << std::endl;
  }
}

static std::vector<unsigned int> _get_pose_pairs(
    sophon_stream::common::PosedObjectMetadata::EModelType model_type) {
  switch (model_type) {
    case sophon_stream::common::PosedObjectMetadata::EModelType::BODY_25:
      return {1,  8,  1,  2,  1,  5,  2,  3,  3,  4,  5,  6,  6,
              7,  8,  9,  9,  10, 10, 11, 8,  12, 12, 13, 13, 14,
              1,  0,  0,  15, 15, 17, 0,  16, 16, 18, 2,  17, 5,
              18, 14, 19, 19, 20, 14, 21, 11, 22, 22, 23, 11, 24};
    case sophon_stream::common::PosedObjectMetadata::EModelType::COCO_18:
      return {1, 2,  1,  5,  2,  3,  3,  4,  5,  6,  6,  7, 1,
              8, 8,  9,  9,  10, 1,  11, 11, 12, 12, 13, 1, 0,
              0, 14, 14, 16, 0,  15, 15, 17, 2,  16, 5,  17};
    default:
      // COCO_18
      return {1, 2,  1,  5,  2,  3,  3,  4,  5,  6,  6,  7, 1,
              8, 8,  9,  9,  10, 1,  11, 11, 12, 12, 13, 1, 0,
              0, 14, 14, 16, 0,  15, 15, 17, 2,  16, 5,  17};
  }
}

static std::vector<unsigned int> _get_fastpose_pairs(int kp_num) {
  switch (kp_num) {
    case 17:
      return {0, 1, 0, 2,  1,  3,  2,  4,  5,  6,  5,  7,  7,  9,
              6, 8, 8, 10, 11, 12, 11, 13, 12, 14, 13, 15, 14, 16};
    case 136:
      return {
          0,   1,   0,   2,   1,   3,   2,   4,   5,   18,  6,   18,  5,   7,
          7,   9,   6,   8,   8,   10,  17,  18,  18,  19,  19,  11,  19,  12,
          11,  13,  12,  14,  13,  15,  14,  16,  20,  24,  21,  25,  23,  25,
          22,  24,  15,  24,  16,  25,  26,  27,  27,  28,  28,  29,  29,  30,
          30,  31,  31,  32,  32,  33,  33,  34,  34,  35,  35,  36,  36,  37,
          37,  38,  38,  39,  39,  40,  40,  41,  41,  42,  43,  44,  44,  45,
          45,  46,  46,  47,  48,  49,  49,  50,  50,  51,  51,  52,  53,  54,
          54,  55,  55,  56,  57,  58,  58,  59,  59,  60,  60,  61,  62,  63,
          63,  64,  64,  65,  65,  66,  66,  67,  68,  69,  69,  70,  70,  71,
          71,  72,  72,  73,  74,  75,  75,  76,  76,  77,  77,  78,  78,  79,
          79,  80,  80,  81,  81,  82,  82,  83,  83,  84,  84,  85,  85,  86,
          86,  87,  87,  88,  88,  89,  89,  90,  90,  91,  91,  92,  92,  93,
          94,  95,  95,  96,  96,  97,  97,  98,  94,  99,  99,  100, 100, 101,
          101, 102, 94,  103, 103, 104, 104, 105, 105, 106, 94,  107, 107, 108,
          108, 109, 109, 110, 94,  111, 111, 112, 112, 113, 113, 114, 115, 116,
          116, 117, 117, 118, 118, 119, 115, 120, 120, 121, 121, 122, 122, 123,
          115, 124, 124, 125, 125, 126, 126, 127, 115, 128, 128, 129, 129, 130,
          130, 131, 115, 132, 132, 133, 133, 134, 134, 135};
    case 133:
      return {
          0,   1,   0,   2,   1,   3,   2,   4,   5,   7,   7,   9,   6,   8,
          8,   10,  11,  13,  12,  14,  13,  15,  14,  16,  18,  19,  21,  22,
          20,  22,  17,  19,  15,  19,  16,  22,  23,  24,  24,  25,  25,  26,
          26,  27,  27,  28,  28,  29,  29,  30,  30,  31,  31,  32,  32,  33,
          33,  34,  34,  35,  35,  36,  36,  37,  37,  38,  38,  39,  40,  41,
          41,  42,  42,  43,  43,  44,  45,  46,  46,  47,  47,  48,  48,  49,
          50,  51,  51,  52,  52,  53,  54,  55,  55,  56,  56,  57,  57,  58,
          59,  60,  60,  61,  61,  62,  62,  63,  63,  64,  65,  66,  66,  67,
          67,  68,  68,  69,  69,  70,  71,  72,  72,  73,  73,  74,  74,  75,
          75,  76,  76,  77,  77,  78,  78,  79,  79,  80,  80,  81,  81,  82,
          82,  83,  83,  84,  84,  85,  85,  86,  86,  87,  87,  88,  88,  89,
          89,  90,  91,  92,  92,  93,  93,  94,  94,  95,  91,  96,  96,  97,
          97,  98,  98,  99,  91,  100, 100, 101, 101, 102, 102, 103, 91,  104,
          104, 105, 105, 106, 106, 107, 91,  108, 108, 109, 109, 110, 110, 111,
          112, 113, 113, 114, 114, 115, 115, 116, 112, 117, 117, 118, 118, 119,
          119, 120, 112, 121, 121, 122, 122, 123, 123, 124, 112, 125, 125, 126,
          126, 127, 127, 128, 112, 129, 129, 130, 130, 131, 131, 132};
    case 68:
      return {0,  1,  0,  2,  1,  3,  2,  4,  5,  18, 6,  18, 5,  7,  7,  9,
              6,  8,  8,  10, 17, 18, 18, 19, 19, 11, 19, 12, 11, 13, 12, 14,
              13, 15, 14, 16, 20, 24, 21, 25, 23, 25, 22, 24, 15, 24, 16, 25,
              26, 27, 27, 28, 28, 29, 29, 30, 26, 31, 31, 32, 32, 33, 33, 34,
              26, 35, 35, 36, 36, 37, 37, 38, 26, 39, 39, 40, 40, 41, 41, 42,
              26, 43, 43, 44, 44, 45, 45, 46, 47, 48, 48, 49, 49, 50, 50, 51,
              47, 52, 52, 53, 53, 54, 54, 55, 47, 56, 56, 57, 57, 58, 58, 59,
              47, 60, 60, 61, 61, 62, 62, 63, 47, 64, 64, 65, 65, 66, 66, 67};
    case 26:
      return {
          0,  1,  0,  2,  1,  3,  2,  4,  5,  18, 6,  18, 5,  7,  7,  9,
          6,  8,  8,  10, 17, 18, 18, 19, 19, 11, 19, 12, 11, 13, 12, 14,
          13, 15, 14, 16, 20, 24, 21, 25, 23, 25, 22, 24, 15, 24, 16, 25,
      };
    case 21:
      return {0,  1,  1,  2,  2,  3,  3,  4,  0,  5,  5,  6,  6,  7,  7,  8,
              0,  9,  9,  10, 10, 11, 11, 12, 0,  13, 13, 14, 14, 15, 15, 16,
              0,  17, 17, 18, 18, 19, 19, 20, 21, 22, 22, 23, 23, 24, 24, 25,
              21, 26, 26, 27, 27, 28, 28, 29, 21, 30, 30, 31, 31, 32, 32, 33,
              21, 34, 34, 35, 35, 36, 36, 37, 21, 38, 38, 39, 39, 40, 40, 41};
  }
}

static std::vector<float> _get_fastpose_p_color(int kp_num) {
  switch (kp_num) {
    case 17:
      return {0,   255, 255, 0,   191, 255, 0,   255, 102, 0,   77,
              255, 0,   255, 0,   77,  255, 255, 77,  255, 204, 77,
              204, 255, 191, 255, 77,  77,  191, 255, 191, 255, 77,
              204, 77,  255, 77,  255, 204, 191, 77,  255, 77,  255,
              191, 127, 77,  255, 77,  255, 127, 0,   255, 255};
    case 136:
      return {0,   255, 255, 0,   191, 255, 0,   255, 102, 0,   77,  255, 0,
              255, 0,   77,  255, 255, 77,  255, 204, 77,  204, 255, 191, 255,
              77,  77,  191, 255, 191, 255, 77,  204, 77,  255, 77,  255, 204,
              191, 77,  255, 77,  255, 191, 127, 77,  255, 77,  255, 127, 77,
              255, 255, 0,   255, 255, 77,  204, 255, 0,   255, 255, 0,   191,
              255, 0,   255, 102, 0,   77,  255, 0,   255, 0,   77,  255, 255};
    case 133:
      return {0,   255, 255, 0,   191, 255, 0,   255, 102, 0,   77,  255,
              0,   255, 0,   77,  255, 255, 77,  255, 204, 77,  204, 255,
              191, 255, 77,  77,  191, 255, 191, 255, 77,  204, 77,  255,
              77,  255, 204, 191, 77,  255, 77,  255, 191, 127, 77,  255,
              77,  255, 127, 0,   255, 255, 0,   191, 255, 0,   255, 102,
              0,   77,  255, 0,   255, 0,   77,  255, 255};
    case 68:
      return {0,   255, 255, 0,   191, 255, 0,   255, 102, 0,   77,  255, 0,
              255, 0,   77,  255, 255, 77,  255, 204, 77,  204, 255, 191, 255,
              77,  77,  191, 255, 191, 255, 77,  204, 77,  255, 77,  255, 204,
              191, 77,  255, 77,  255, 191, 127, 77,  255, 77,  255, 127, 77,
              255, 255, 0,   255, 255, 77,  204, 255, 0,   255, 255, 0,   191,
              255, 0,   255, 102, 0,   77,  255, 0,   255, 0,   77,  255, 255};
    case 26:
      return {0,   255, 255, 0,   191, 255, 0,   255, 102, 0,   77,  255, 0,
              255, 0,   77,  255, 255, 77,  255, 204, 77,  204, 255, 191, 255,
              77,  77,  191, 255, 191, 255, 77,  204, 77,  255, 77,  255, 204,
              191, 77,  255, 77,  255, 191, 127, 77,  255, 77,  255, 127, 77,
              255, 255, 0,   255, 255, 77,  204, 255, 0,   255, 255, 0,   191,
              255, 0,   255, 102, 0,   77,  255, 0,   255, 0,   77,  255, 255};
    case 21:
      return {255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
              255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
              255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
              255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
              255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255};
  }
}

static std::vector<float> _get_fastpose_line_color(int kp_num) {
  switch (kp_num) {
    case 17:
      return {0,   215, 255, 0,  255, 204, 0,   134, 255, 0,   255, 50,
              77,  255, 222, 77, 196, 255, 77,  135, 255, 191, 255, 77,
              77,  255, 77,  77, 222, 255, 255, 156, 127, 0,   127, 255,
              255, 127, 77,  0,  77,  255, 255, 77,  36};
    case 136:
      return {0,   215, 255, 0,   255, 204, 0,   134, 255, 0,   255, 50,
              0,   255, 102, 77,  255, 222, 77,  196, 255, 77,  135, 255,
              191, 255, 77,  77,  255, 77,  77,  191, 255, 204, 77,  255,
              77,  222, 255, 255, 156, 127, 0,   127, 255, 255, 127, 77,
              0,   77,  255, 255, 77,  36,  0,   77,  255, 0,   77,  255,
              0,   77,  255, 0,   77,  255, 255, 156, 127, 255, 156, 127};
    case 133:
      return {0,   215, 255, 0,   255, 204, 0,   134, 255, 0,   255,
              50,  0,   255, 102, 77,  255, 222, 77,  196, 255, 77,
              135, 255, 191, 255, 77,  77,  255, 77,  77,  191, 255,
              204, 77,  255, 77,  222, 255, 255, 156, 127, 0,   127,
              255, 255, 127, 77,  0,   77,  255, 255, 77,  36,  0,
              77,  255, 0,   77,  255, 0,   77,  255, 0,   77,  255};
    case 68:
      return {0,   215, 255, 0,   255, 204, 0,   134, 255, 0,   255, 50,
              0,   255, 102, 77,  255, 222, 77,  196, 255, 77,  135, 255,
              191, 255, 77,  77,  255, 77,  77,  191, 255, 204, 77,  255,
              77,  222, 255, 255, 156, 127, 0,   127, 255, 255, 127, 77,
              0,   77,  255, 255, 77,  36,  0,   77,  255, 0,   77,  255,
              0,   77,  255, 0,   77,  255, 255, 156, 127, 255, 156, 127};
    case 26:
      return {0,   215, 255, 0,   255, 204, 0,   134, 255, 0,   255, 50,
              0,   255, 102, 77,  255, 222, 77,  196, 255, 77,  135, 255,
              191, 255, 77,  77,  255, 77,  77,  191, 255, 204, 77,  255,
              77,  222, 255, 255, 156, 127, 0,   127, 255, 255, 127, 77,
              0,   77,  255, 255, 77,  36,  0,   77,  255, 0,   77,  255,
              0,   77,  255, 0,   77,  255, 255, 156, 127, 255, 156, 127};
    case 21:
      return {255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
              255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
              255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
              255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
              255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255};
  }
}

static void _render_keypoints_bmcv(bm_handle_t& handle, bm_image& frame,
                                   const std::vector<float>& keypoints,
                                   const std::vector<unsigned int>& pairs,
                                   const std::vector<float> colors,
                                   const float thicknessCircleRatio,
                                   const float thicknessLineRatioWRTCircle,
                                   const float threshold, float scale) {
  // Get frame channels
  const auto width = frame.width;
  const auto height = frame.height;
  const auto area = width * height;

  // Parameters
  const auto lineType = 8;
  const auto shift = 0;
  const auto numberColors = colors.size();
  const auto thresholdRectangle = 0.1f;

  // Keypoints

  const auto ratioAreas = 1;
  // Size-dependent variables
  const auto thicknessRatio =
      fastMax(intRound(std::sqrt(area) * thicknessCircleRatio * ratioAreas), 1);
  // Negative thickness in cv::circle means that a filled circle is to
  // be drawn.
  const auto thicknessCircle = (ratioAreas > 0.05 ? thicknessRatio : -1);
  const auto thicknessLine =
      2;  // intRound(thicknessRatio * thicknessLineRatioWRTCircle);
  const auto radius = thicknessRatio / 2;

  // Draw lines
  for (auto pair = 0u; pair < pairs.size(); pair += 2) {
    const auto index1 = (pairs[pair]) * 3;
    const auto index2 = (pairs[pair + 1]) * 3;

    if (keypoints[index1 + 2] > threshold &&
        keypoints[index2 + 2] > threshold) {
      const auto colorIndex = pairs[pair + 1] * 3;
      bmcv_color_t color = {colors[(colorIndex + 2) % numberColors],
                            colors[(colorIndex + 1) % numberColors],
                            colors[(colorIndex + 0) % numberColors]};
      bmcv_point_t start = {intRound(keypoints[index1] * scale),
                            intRound(keypoints[index1 + 1] * scale)};
      bmcv_point_t end = {intRound(keypoints[index2] * scale),
                          intRound(keypoints[index2 + 1] * scale)};

      if (BM_SUCCESS != bmcv_image_draw_lines(handle, frame, &start, &end, 1,
                                              color, thicknessLine)) {
        std::cout << "bmcv draw lines error !!!" << std::endl;
      }
    }
  }
}

static void _render_pose_keypoints_bmcv(
    bm_handle_t& handle, bm_image& frame,
    const std::vector<float>& poseKeypoints, const float renderThreshold,
    float scale,
    sophon_stream::common::PosedObjectMetadata::EModelType modelType,
    const bool blendOriginalFrame) {
  // Parameters
  const auto thicknessCircleRatio = 1.f / 75.f;
  const auto thicknessLineRatioWRTCircle = 0.75f;
  const auto& pairs = _get_pose_pairs(modelType);

  // Render keypoints
  _render_keypoints_bmcv(handle, frame, poseKeypoints, pairs,
                         POSE_COLORS_RENDER, thicknessCircleRatio,
                         thicknessLineRatioWRTCircle, renderThreshold, scale);
}

static void _render_fastpose_keypoints_bmcv(
    bm_handle_t& handle, bm_image& frame, const std::vector<float>& keypoints,
    const std::vector<float>& scores, const std::vector<unsigned int>& pairs,
    const std::vector<float> p_colors, const std::vector<float> line_colors,
    const float thicknessCircleRatio, const float thicknessLineRatioWRTCircle,
    std::string loss_type, float scale) {
  // Get frame channels
  const auto width = frame.width;
  const auto height = frame.height;
  const auto area = width * height;

  // Parameters
  const auto lineType = 8;
  const auto shift = 0;
  const auto pNumberColors = p_colors.size();
  const auto lineNumberColors = line_colors.size();
  const auto thresholdRectangle = 0.1f;
  float threshold1 = 0.4, threshold2 = 0.4;
  if (loss_type == "L1JointRegression") threshold1 = 0.05, threshold2 = 0.05;

  // Keypoints

  const auto ratioAreas = 1;
  // Size-dependent variables
  const auto thicknessRatio =
      fastMax(intRound(std::sqrt(area) * thicknessCircleRatio * ratioAreas), 1);
  // Negative thickness in cv::circle means that a filled circle is to
  // be drawn.
  const auto thicknessCircle = (ratioAreas > 0.05 ? thicknessRatio : -1);
  const auto thicknessLine =
      2;  // intRound(thicknessRatio * thicknessLineRatioWRTCircle);
  const auto radius = thicknessRatio / 2;

  // Draw lines
  for (auto pair = 0u; pair < pairs.size(); pair += 2) {
    const auto index1 = pairs[pair];
    const auto index2 = pairs[pair + 1];

    if (loss_type == "Combined" && keypoints.size() / 2 == 68) {
      if (index1 >= 26)
        threshold1 = 0.05;
      else
        threshold1 = 0.4;
      if (index2 >= 26)
        threshold2 = 0.05;
      else
        threshold2 = 0.4;
    } else if (loss_type == "Combined") {
      if (index1 >= keypoints.size() / 2 - 110)
        threshold1 = 0.05;
      else
        threshold1 = 0.4;
      if (index2 >= keypoints.size() / 2 - 110)
        threshold2 = 0.05;
      else
        threshold2 = 0.4;
    }

    if (scores[index1] > threshold1 && scores[index2] > threshold2) {
      bmcv_color_t color;
      if (pair / 2 < line_colors.size() / 3)
        color = {line_colors[pair / 2 * 3 + 2], line_colors[pair / 2 * 3 + 1],
                 line_colors[pair / 2 * 3 + 0]};
      else
        color = {255, 255, 255};
      bmcv_point_t start = {intRound(keypoints[index1 * 2] * scale),
                            intRound(keypoints[index1 * 2 + 1] * scale)};
      bmcv_point_t end = {intRound(keypoints[index2 * 2] * scale),
                          intRound(keypoints[index2 * 2 + 1] * scale)};

      if (BM_SUCCESS != bmcv_image_draw_lines(handle, frame, &start, &end, 1,
                                              color, thicknessLine)) {
        std::cout << "bmcv draw lines error !!!" << std::endl;
      }
    }
  }
}

static void _render_fastpose_keypoints(bm_handle_t& handle, bm_image& frame,
                                       const std::vector<float>& poseKeypoints,
                                       const std::vector<float>& scores,
                                       std::string loss_type, float scale) {
  // Parameters
  const auto thicknessCircleRatio = 1.f / 75.f;
  const auto thicknessLineRatioWRTCircle = 0.75f;
  const auto& pairs = _get_fastpose_pairs(poseKeypoints.size() / 2);
  const auto& p_color = _get_fastpose_p_color(poseKeypoints.size() / 2);
  const auto& line_color = _get_fastpose_line_color(poseKeypoints.size() / 2);

  // Render keypoints
  _render_fastpose_keypoints_bmcv(
      handle, frame, poseKeypoints, scores, pairs, p_color, line_color,
      thicknessCircleRatio, thicknessLineRatioWRTCircle, loss_type, scale);
}

void draw_bytetrack_results(
    std::shared_ptr<sophon_stream::common::ObjectMetadata> objectMetadata,
    bm_image& frame) {
  int colors_num = colors.size();
  for (int i = 0; i < objectMetadata->mTrackedObjectMetadatas.size(); i++) {
    // draw image
    auto trackObj = objectMetadata->mTrackedObjectMetadatas[i];
    auto detObj = objectMetadata->mDetectedObjectMetadatas[i];
    std::string label = std::to_string(trackObj->mTrackId);
    _draw_rectangle_and_text_bmcv(
        objectMetadata->mFrame->mHandle, label, detObj->mBox.mX,
        detObj->mBox.mY, detObj->mBox.mWidth, detObj->mBox.mHeight, frame,
        colors[trackObj->mTrackId % colors_num], true);
  }
}

void draw_license_plate_recognition_results_opencv(
    std::shared_ptr<sophon_stream::common::ObjectMetadata> objectMetadata,
    cv::Mat& img) {
  uni_text::UniText uniText(
      "../license_plate_recognition/data/wqy-microhei.ttc", 22);
  bm_image frame;
  cv::bmcv::toBMI(img, &frame);
  // get license plate, and draw pics
  if (objectMetadata->mSubObjectMetadatas.size() > 0) {
    for (auto subObj : objectMetadata->mSubObjectMetadatas) {
      IVS_WARN("get recognized license plate datas from yolo and lprnet");
      int subId = subObj->mSubId;
      auto reconizedObj = subObj->mRecognizedObjectMetadatas[0];
      auto detObj = objectMetadata->mDetectedObjectMetadatas[subId];

      // get license plate
      std::string oriLabel = reconizedObj->mLabelName;

      // draw pics for wide str
      _draw_rectangle_and_text_bmcv(objectMetadata->mFrame->mHandle, oriLabel,
                                    detObj->mBox.mX, detObj->mBox.mY,
                                    detObj->mBox.mWidth, detObj->mBox.mHeight,
                                    frame, colors[0], false);
      uniText.PutText(img, oriLabel,
                      cv::Point(detObj->mBox.mX, detObj->mBox.mY),
                      cv::Scalar(0, 0, 255), false);
    }
  }
}
void draw_retinaface_results(
    std::shared_ptr<sophon_stream::common::ObjectMetadata> objectMetadata,
    bm_image& frame) {
  _draw_face_rectangle_bmcv(objectMetadata->mFrame->mHandle, objectMetadata,
                            frame);
}

void draw_retinaface_distributor_resnet_faiss_converger_results(
    std::shared_ptr<sophon_stream::common::ObjectMetadata> objectMetadata,
    bm_image &frame) {
  bm_handle_t &handle = objectMetadata->mFrame->mHandle;

  if (objectMetadata->mSubObjectMetadatas.size() > 0)
  {
    for (auto &subObj : objectMetadata->mSubObjectMetadatas)
    {
      int subId = subObj->mSubId;
      auto faceObj = objectMetadata->mFaceObjectMetadatas[subId]; // 第一张脸
      auto resnetObj =
          subObj->mRecognizedObjectMetadatas[0]; // 第一张脸对应的resnet
      int class_id = subObj->mRecognizedObjectMetadatas[0]->mTopKLabels[0];
      auto label = subObj->mRecognizedObjectMetadatas[0]->mLabelName;

      bmcv_rect_t rect;
      rect.start_x = std::max(faceObj->left, 0);
      rect.start_y = std::max(faceObj->top, 0);
      rect.crop_w = std::max(faceObj->right - faceObj->left + 1, 0);
      rect.crop_h = std::max(faceObj->bottom - faceObj->top + 1, 0);

      // 1. 画人脸框：
      bmcv_image_draw_rectangle(handle, frame, 1, &rect, 2, 255, 2, 2);
      // 2. 在人脸框左上角绘制标签文字
      constexpr int TEXT_INTERNAL_OFFSET = 40;
      int call_top = rect.start_y - TEXT_INTERNAL_OFFSET;
      call_top = std::max(call_top, 0);
      _draw_text_bmcv(handle,
                      rect.start_x,
                      call_top,
                      frame,
                      label);
      // Debug 输出
      std::cout << "Draw face #" << subId
                << " rect(" << rect.start_x << "," << rect.start_y
                << "," << rect.crop_w << "," << rect.crop_h << ")"
                << " label=" << label
                << std::endl;
    }
  }
}

const std::vector<std::vector<unsigned int>> LIMB_COLORS = {
    {51, 153, 255}, {51, 153, 255}, {51, 153, 255}, {51, 153, 255},
    {255, 51, 255}, {255, 51, 255}, {255, 51, 255}, {255, 128, 0},
    {255, 128, 0},  {255, 128, 0},  {255, 128, 0},  {255, 128, 0},
    {0, 255, 0},    {0, 255, 0},    {0, 255, 0},    {0, 255, 0},
    {0, 255, 0},    {0, 255, 0},    {0, 255, 0}};
const std::vector<std::vector<unsigned int>> SKELETON = {
    {16, 14}, {14, 12}, {17, 15}, {15, 13}, {12, 13}, {6, 12}, {7, 13},
    {6, 7},   {6, 8},   {7, 9},   {8, 10},  {9, 11},  {2, 3},  {1, 2},
    {1, 3},   {2, 4},   {3, 5},   {4, 6},   {5, 7}};
const std::vector<std::vector<unsigned int>> KPS_COLORS = {
    {0, 255, 0},    {0, 255, 0},    {0, 255, 0},    {0, 255, 0},
    {0, 255, 0},    {255, 128, 0},  {255, 128, 0},  {255, 128, 0},
    {255, 128, 0},  {255, 128, 0},  {255, 128, 0},  {51, 153, 255},
    {51, 153, 255}, {51, 153, 255}, {51, 153, 255}, {51, 153, 255},
    {51, 153, 255}};

void draw_yolov8_det_pose(
    std::shared_ptr<sophon_stream::common::ObjectMetadata> objectMetadata,
    cv::Mat& img) {
  const int num_point = 17;
  int idx = 0;
  for (auto& obj : objectMetadata->mDetectedObjectMetadatas) {
    cv::rectangle(img,
                  cv::Rect{obj->mBox.mX, obj->mBox.mY, obj->mBox.mWidth,
                           obj->mBox.mHeight},
                  {0, 0, 255}, 2);

    char text[256];
    sprintf(text, "person %.1f%%", obj->mScores[0] * 100);

    int baseLine = 0;
    cv::Size label_size =
        cv::getTextSize(text, cv::FONT_HERSHEY_SIMPLEX, 0.4, 1, &baseLine);

    int x = (int)obj->mBox.mX;
    int y = (int)obj->mBox.mY + 1;

    if (y > img.rows) y = img.rows;

    cv::rectangle(
        img, cv::Rect(x, y, label_size.width, label_size.height + baseLine),
        {0, 0, 255}, -1);

    cv::putText(img, text, cv::Point(x, y + label_size.height),
                cv::FONT_HERSHEY_SIMPLEX, 0.4, {255, 255, 255}, 1);

    auto& kps = objectMetadata->mPosedObjectMetadatas[idx]->keypoints;
    for (int k = 0; k < num_point + 2; k++) {
      if (k < num_point) {
        int kps_x = std::round(kps[k * 3]);
        int kps_y = std::round(kps[k * 3 + 1]);
        float kps_s = kps[k * 3 + 2];
        if (kps_s > 0.5f) {
          cv::Scalar kps_color =
              cv::Scalar(KPS_COLORS[k][0], KPS_COLORS[k][1], KPS_COLORS[k][2]);
          cv::circle(img, {kps_x, kps_y}, 5, kps_color, -1);
        }
      }
      auto& ske = SKELETON[k];
      int pos1_x = std::round(kps[(ske[0] - 1) * 3]);
      int pos1_y = std::round(kps[(ske[0] - 1) * 3 + 1]);

      int pos2_x = std::round(kps[(ske[1] - 1) * 3]);
      int pos2_y = std::round(kps[(ske[1] - 1) * 3 + 1]);

      float pos1_s = kps[(ske[0] - 1) * 3 + 2];
      float pos2_s = kps[(ske[1] - 1) * 3 + 2];

      if (pos1_s > 0.5f && pos2_s > 0.5f) {
        cv::Scalar limb_color =
            cv::Scalar(LIMB_COLORS[k][0], LIMB_COLORS[k][1], LIMB_COLORS[k][2]);
        cv::line(img, {pos1_x, pos1_y}, {pos2_x, pos2_y}, limb_color, 2);
      }
    }
    ++idx;
  }
}

void draw_yolov5_results(
    std::shared_ptr<sophon_stream::common::ObjectMetadata> objectMetadata,
    bm_image& frame, std::vector<std::string>& class_names) {
  int colors_num = colors.size();

  for (auto detObj : objectMetadata->mDetectedObjectMetadatas) {
    int class_id = detObj->mClassify;
    std::string label =
        class_names[class_id] + ":" + cv::format("%.2f", detObj->mScores[0]);
    _draw_rectangle_and_text_bmcv(objectMetadata->mFrame->mHandle, label,
                                  detObj->mBox.mX, detObj->mBox.mY,
                                  detObj->mBox.mWidth, detObj->mBox.mHeight,
                                  frame, colors[class_id % colors_num], true);
  }
}

void draw_yolov5_bytetrack_distributor_resnet_converger_results(
    std::shared_ptr<sophon_stream::common::ObjectMetadata> objectMetadata,
    bm_image& frame, std::vector<std::string>& car_attr,
    std::vector<std::string>& person_attr) {
  if (objectMetadata->mSubObjectMetadatas.size() > 0) {
    int colors_num = colors.size();

    for (auto subObj : objectMetadata->mSubObjectMetadatas) {
      int subId = subObj->mSubId;
      auto detObj = objectMetadata->mDetectedObjectMetadatas[subId];
      auto trackObj = objectMetadata->mTrackedObjectMetadatas[subId];
      bmcv_rect_t rect;
      rect.start_x = detObj->mBox.mX;
      rect.start_y = detObj->mBox.mY;
      rect.crop_w = detObj->mBox.mWidth;
      rect.crop_h = detObj->mBox.mHeight;
      int class_id = subObj->mRecognizedObjectMetadatas[0]->mTopKLabels[0];
      std::string label = std::to_string(trackObj->mTrackId) + "-";
      if (detObj->mClassify == 2)
        label += car_attr[class_id];
      else
        label += person_attr[class_id];
      _draw_rectangle_and_text_bmcv(
          objectMetadata->mFrame->mHandle, label, rect.start_x, rect.start_y,
          rect.crop_w, rect.crop_h, frame,
          colors[trackObj->mTrackId % colors_num], true);
    }
  }
}

void draw_yolox_results(
    std::shared_ptr<sophon_stream::common::ObjectMetadata> objectMetadata,
    bm_image& frame, std::vector<std::string>& class_names) {
  int colors_num = colors.size();

  for (auto detObj : objectMetadata->mDetectedObjectMetadatas) {
    std::string label = class_names[detObj->mClassify] + ":" +
                        cv::format("%.2f", detObj->mScores[0]);
    _draw_rectangle_and_text_bmcv(
        objectMetadata->mFrame->mHandle, label, detObj->mBox.mX,
        detObj->mBox.mY, detObj->mBox.mWidth, detObj->mBox.mHeight, frame,
        colors[detObj->mClassify % colors_num], true);
  }
}

void draw_default(
    std::shared_ptr<sophon_stream::common::ObjectMetadata> objectMetadata,
    bm_image& frame) {
  return;
}

void draw_yolov5_fastpose_posec3d_results(
    std::shared_ptr<sophon_stream::common::ObjectMetadata> objectMetadata,
    bm_image& frame, std::string loss_type) {
  if (objectMetadata->mRecognizedObjectMetadatas.size() != 0)
    _draw_text_bmcv(
        objectMetadata->mFrame->mHandle, 50, 10, frame,
        objectMetadata->mRecognizedObjectMetadatas[0]->mLabelName + ":" +
            cv::format(
                "%.2f",
                objectMetadata->mRecognizedObjectMetadatas[0]->mScores[0]));
  for (auto subObj : objectMetadata->mPosedObjectMetadatas) {
    _render_fastpose_keypoints(objectMetadata->mFrame->mHandle, frame,
                               subObj->keypoints, subObj->scores, loss_type,
                               1.0);
  }
  for (auto subObj : objectMetadata->mDetectedObjectMetadatas) {
    std::string label = "person:" + cv::format("%.2f", subObj->mScores[0]);
    _draw_rectangle_and_text_bmcv(objectMetadata->mFrame->mHandle, label,
                                  subObj->mBox.mX, subObj->mBox.mY,
                                  subObj->mBox.mWidth, subObj->mBox.mHeight,
                                  frame, colors[0], true);
  }
}

void draw_ppocr_results(
    std::shared_ptr<sophon_stream::common::ObjectMetadata> objectMetadata,
    cv::Mat& img) {
  uni_text::UniText uniText("../ppocr/data/wqy-microhei.ttc", 30);

  // draw words
  if (objectMetadata->mSubObjectMetadatas.size() > 0) {
    for (auto subObj : objectMetadata->mSubObjectMetadatas) {
      IVS_WARN("get recognized words from ppocr");
      int subId = subObj->mSubId;
      auto reconizedObj = subObj->mRecognizedObjectMetadatas[0];
      auto detObj = objectMetadata->mDetectedObjectMetadatas[subId];

      std::string oriLabel = reconizedObj->mLabelName;

      uniText.PutText(img, oriLabel,
                      cv::Point(detObj->mKeyPoints[0]->mPoint.mX,
                                detObj->mKeyPoints[0]->mPoint.mY),
                      cv::Scalar(0, 255, 0), false);
    }
  }
}

void draw_text_results_online(
    std::shared_ptr<sophon_stream::common::ObjectMetadata> objectMetadata,
    bm_image& frame, std::string texts_path, std::vector<std::string> texts,
    std::vector<int> top, std::vector<int> left, int r, int g, int b) {
  uni_text::UniText uniText(texts_path.c_str(), 30);
  for (int i = 0; i < texts.size(); i++) {
    bm_image overlay_image;
    uniText.genBitMap(objectMetadata->mFrame->mHandle, texts[i], overlay_image,
                      r, g, b);
    bmcv_rect_t overlay_info = {top[i], left[i], overlay_image.height,
                                overlay_image.width};

    bmcv_image_overlay(objectMetadata->mFrame->mHandle, frame, 1, &overlay_info,
                       &overlay_image);

    bm_image_destroy(overlay_image);
  }
}
void draw_text_results(
    std::shared_ptr<sophon_stream::common::ObjectMetadata> objectMetadata,
    bm_image& frame, std::vector<bm_image>& overlay_images,
    std::vector<int>& top, std::vector<int>& left, bool draw_interval) {
  if (!objectMetadata->mFilter || draw_interval) {
    for (int i = 0; i < overlay_images.size(); i++) {
      bm_image overlay_image = overlay_images[i];
      bmcv_rect_t overlay_info = {top[i], left[i], overlay_image.height,
                                  overlay_image.width};
      bmcv_image_overlay(objectMetadata->mFrame->mHandle, frame, 1,
                         &overlay_info, &overlay_image);
    }
  }
}

void draw_opencv_obb_result(
    std::shared_ptr<common::ObjectMetadata> objectMetadata,
    std::vector<std::string>& class_names, cv::Mat& frame, bool put_text_flag,
    bool draw_interval) {
  std::shared_ptr<common::ObjectMetadata> objData;
  {
    std::lock_guard<std::mutex> lk(mLastObjectMetaDataMtx);
    objData = (objectMetadata->mFilter && draw_interval)
                  ? lastObjectMetadataMap[objectMetadata->mFrame->mChannelId]
                  : objectMetadata;
    lastObjectMetadataMap[objectMetadata->mFrame->mChannelId] = objData;
  }
  auto box_vec = objData->mObbObjectMetadatas;
  for (int n = 0; n < box_vec.size(); n++) {
    cv::Point rook_points[4];
    rook_points[0] = cv::Point(int(box_vec[n]->x1), int(box_vec[n]->y1));
    rook_points[1] = cv::Point(int(box_vec[n]->x2), int(box_vec[n]->y2));
    rook_points[2] = cv::Point(int(box_vec[n]->x3), int(box_vec[n]->y3));
    rook_points[3] = cv::Point(int(box_vec[n]->x4), int(box_vec[n]->y4));
    const cv::Point* ppt[1] = {rook_points};
    int npt[] = {4};
    std::string label = class_names[box_vec[n]->class_id] + cv::format(":%.2f", box_vec[n]->score);
    cv::Scalar color(colors[box_vec[n]->class_id][0], colors[box_vec[n]->class_id][1], colors[box_vec[n]->class_id][2]);
    cv::polylines(frame, ppt, npt, 1, 1, color, 2, 8, 0);
    if(put_text_flag){
      cv::putText(frame, label, cv::Point(int(box_vec[n]->x1), int(box_vec[n]->y1 - 5)),
                  cv::FONT_HERSHEY_SIMPLEX, 0.7, color, 2);
    }
  }
}

}  // namespace osd
}  // namespace element
}  // namespace sophon_stream

#endif  // SOPHON_STREAM_ELEMENT_OSD_DRAW_UTILS_H_