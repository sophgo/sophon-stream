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

#include <mutex>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>

#include "common/logger.h"
#include "common/posed_object_metadata.h"
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
    bool put_text_flag) {
  int colors_num = colors.size();
  std::map<int, std::vector<bmcv_rect_t>> rectsMap;
  int thickness = 2;
  float fontScale = 1;
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
        const auto colorIndex =
            pairs[pair + 1] * 3;
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

}  // namespace osd
}  // namespace element
}  // namespace sophon_stream

#endif  // SOPHON_STREAM_ELEMENT_OSD_DRAW_UTILS_H_