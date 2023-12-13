//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#include <sys/stat.h>
#include <codecvt>
#include <fstream>
#include <nlohmann/json.hpp>
#include <opencv2/opencv.hpp>
#include <unordered_map>

#include "channel.h"
#include "common/clocker.h"
#include "common/common_defs.h"
#include "common/error_code.h"
#include "common/logger.h"
#include "common/object_metadata.h"
#include "common/posed_object_metadata.h"
#include "common/profiler.h"
#include "engine.h"
#include "init_engine.h"
#include "cvUniText.h"
#define POSE_COLORS_RENDER_CPU                                                 \
  255.f, 0.f, 0.f, 255.f, 85.f, 0.f, 255.f, 170.f, 0.f, 255.f, 255.f, 0.f,     \
      170.f, 255.f, 0.f, 85.f, 255.f, 0.f, 0.f, 255.f, 0.f, 0.f, 255.f, 85.f,  \
      0.f, 255.f, 170.f, 0.f, 255.f, 255.f, 0.f, 170.f, 255.f, 0.f, 85.f,      \
      255.f, 0.f, 0.f, 255.f, 85.f, 0.f, 255.f, 170.f, 0.f, 255.f, 255.f, 0.f, \
      255.f, 255.f, 0.f, 170.f, 255.f, 0.f, 85.f, 255.f, 0.f, 0.f, 255.f, 0.f, \
      255.f, 255.f, 85.f, 255.f, 255.f, 170.f, 255.f, 255.f, 255.f, 255.f,     \
      170.f, 255.f, 255.f, 85.f, 255.f, 255.f

const std::vector<float> POSE_COLORS_RENDER{POSE_COLORS_RENDER_CPU};

const std::vector<std::vector<int>> colors = {
    {0, 0, 0},    {128, 0, 0},   {0, 128, 0},    {128, 128, 0},
    {0, 0, 128},  {128, 0, 128}, {0, 128, 128},  {128, 128, 128},
    {64, 0, 0},   {192, 0, 0},   {64, 128, 0},   {192, 128, 0},
    {64, 0, 128}, {192, 0, 128}, {64, 128, 128}, {192, 128, 128},
    {0, 64, 0},   {128, 64, 0},  {0, 192, 0},    {128, 192, 0},
    {0, 64, 128}};

template <typename T>
static int intRound(const T a) {
  return int(a + 0.5f);
}

// Max/min functions
template <typename T>
static T fastMax(const T a, const T b) {
  return (a > b ? a : b);
}

static void _gen_storage_image(std::shared_ptr<sophon_stream::common::ObjectMetadata> objectMetadata, bm_image& imageStorage)
{
    int width = objectMetadata->mFrame->mWidth;
    int height = objectMetadata->mFrame->mHeight;
    bm_image image = *objectMetadata->mFrame->mSpData;
    bm_image_create(objectMetadata->mFrame->mHandle, height, width,
                    FORMAT_YUV420P, image.data_type, &imageStorage);
    bmcv_image_storage_convert(objectMetadata->mFrame->mHandle, 1, &image,
                                &imageStorage);
}

static void _draw_rectangle_and_text_bmcv(bm_handle_t& handle, std::string& lable, int left,
               int top, int width, int height, bm_image& frame, const std::vector<int>& color,
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
    bmcv_image_draw_rectangle(handle, frame, 1, &rect, 3, color[0], color[1],
                              color[2]);
    if (put_text_flag) {
        bmcv_point_t org = {left, top - 10};
        bmcv_color_t bmcv_color = {color[0], color[1], color[2]};
        int thickness = 2;
        float fontScale = 1.5;

        if (BM_SUCCESS != bmcv_image_put_text(handle, frame, lable.c_str(),
                                              org, bmcv_color, fontScale,
                                              thickness)) {
            std::cout << "bmcv put text error !!!" << std::endl;
        }
    }
}

static void _draw_face_rectangle_bmcv(bm_handle_t& handle,
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

static void _draw_text_bmcv(bm_handle_t& handle, int left, int top, bm_image& frame,
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
      return {0, 1, 0,  2,  1,  3,  2,  4,  5,  6,  5,  7,  7,  9,  6,
             8, 8, 10, 11, 12, 11, 13, 12, 14, 13, 15, 14, 16};
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

static void _render_pose_keypoints_bmcv(bm_handle_t& handle, bm_image& frame,
                             const std::vector<float>& poseKeypoints,
                             const float renderThreshold, float scale,
                             sophon_stream::common::PosedObjectMetadata::EModelType modelType,
                             const bool blendOriginalFrame) {
  // Parameters
  const auto thicknessCircleRatio = 1.f / 75.f;
  const auto thicknessLineRatioWRTCircle = 0.75f;
  const auto& pairs = _get_pose_pairs(modelType);

  // Render keypoints
  _render_keypoints_bmcv(handle, frame, poseKeypoints, pairs, POSE_COLORS_RENDER,
                      thicknessCircleRatio, thicknessLineRatioWRTCircle,
                      renderThreshold, scale);
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
    _render_fastpose_keypoints_bmcv(handle, frame, poseKeypoints, scores, pairs, p_color,
                        line_color, thicknessCircleRatio,
                        thicknessLineRatioWRTCircle, loss_type, scale);
}

void draw_bytetrack_results(std::shared_ptr<sophon_stream::common::ObjectMetadata> objectMetadata, std::string& out_dir)
{
    bm_image imageStorage;
    _gen_storage_image(objectMetadata, imageStorage);
    // for (auto subObj : objectMetadata->mSubObjectMetadatas) {
    //   // draw image
    //   draw_tracker_bmcv(objectMetadata->mFrame->mHandle,
    //                     subObj->mTrackedObjectMetadata->mTrackId,
    //                     subObj->mDetectedObjectMetadata->mBox.mX,
    //                     subObj->mDetectedObjectMetadata->mBox.mY,
    //                     subObj->mDetectedObjectMetadata->mBox.mWidth,
    //                     subObj->mDetectedObjectMetadata->mBox.mHeight,
    //                     imageStorage, true);
    // }

    int colors_num = colors.size();
    for (int i = 0; i < objectMetadata->mTrackedObjectMetadatas.size(); i++) {
        // draw image
        auto trackObj = objectMetadata->mTrackedObjectMetadatas[i];
        auto detObj = objectMetadata->mDetectedObjectMetadatas[i];
        std::string label = std::to_string(trackObj->mTrackId);
        _draw_rectangle_and_text_bmcv(objectMetadata->mFrame->mHandle, label,
                            detObj->mBox.mX, detObj->mBox.mY, detObj->mBox.mWidth,
                            detObj->mBox.mHeight, imageStorage, colors[trackObj->mTrackId % colors_num], true);
    }

    // save image
    void* jpeg_data = NULL;
    size_t out_size = 0;
    int ret = bmcv_image_jpeg_enc(objectMetadata->mFrame->mHandle, 1,
                                &imageStorage, &jpeg_data, &out_size);
    if (ret == BM_SUCCESS) {
        std::string img_file =
            out_dir + "/" + std::to_string(objectMetadata->mFrame->mChannelId) +
            "_" + std::to_string(objectMetadata->mFrame->mFrameId) + ".jpg";
        FILE* fp = fopen(img_file.c_str(), "wb");
        fwrite(jpeg_data, out_size, 1, fp);
        fclose(fp);
    }
    free(jpeg_data);
    bm_image_destroy(imageStorage);
}

void draw_license_plate_recognition_results(std::shared_ptr<sophon_stream::common::ObjectMetadata> objectMetadata, std::string& out_dir)
{
    bm_image imageStorage;
    _gen_storage_image(objectMetadata, imageStorage);
    uni_text::UniText uniText("../license_plate_recognition/data/wqy-microhei.ttc", 22);
    cv::Mat img;
    cv::bmcv::toMAT(&imageStorage, img);
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
            _draw_rectangle_and_text_bmcv(objectMetadata->mFrame->mHandle, oriLabel, detObj->mBox.mX,
                    detObj->mBox.mY, detObj->mBox.mWidth, detObj->mBox.mHeight,
                    imageStorage, colors[0], false);

            uniText.PutText(img, oriLabel,
                          cv::Point(detObj->mBox.mX, detObj->mBox.mY),
                          cv::Scalar(0, 0, 255), false);
        }
        std::string img_file =
            out_dir + "/" + std::to_string(objectMetadata->mFrame->mChannelId) +
            "_" + std::to_string(objectMetadata->mFrame->mFrameId) + ".jpg";
        cv::imwrite(img_file, img);
  }
  bm_image_destroy(imageStorage);
}

void draw_openpose_results(std::shared_ptr<sophon_stream::common::ObjectMetadata> objectMetadata, std::string& out_dir)
{
    int width = objectMetadata->mFrame->mWidth;
    int height = objectMetadata->mFrame->mHeight;
    bm_image image = *objectMetadata->mFrame->mSpData;
    bm_image imageStorage;

    if (image.image_format == 11) {
        bm_image image_aligned;
        bool need_copy = image.width & (64 - 1);
        if (need_copy) {
            int stride1[3], stride2[3];
            bm_image_get_stride(image, stride1);
            stride2[0] = FFALIGN(stride1[0], 64);
            stride2[1] = FFALIGN(stride1[1], 64);
            stride2[2] = FFALIGN(stride1[2], 64);

            bm_image_create(objectMetadata->mFrame->mHandle, image.height,
                            image.width, image.image_format, image.data_type,
                            &image_aligned, stride2);
            bm_image_alloc_dev_mem(image_aligned, BMCV_IMAGE_FOR_IN);
            bmcv_copy_to_atrr_t copyToAttr;
            memset(&copyToAttr, 0, sizeof(copyToAttr));
            copyToAttr.start_x = 0;
            copyToAttr.start_y = 0;
            copyToAttr.if_padding = 1;
            bmcv_image_copy_to(objectMetadata->mFrame->mHandle, copyToAttr, image,
                                image_aligned);
        } else {
            image_aligned = image;
        }

        bm_image_create(objectMetadata->mFrame->mHandle, image.height,
                        image.width, FORMAT_YUV420P, image.data_type,
                        &imageStorage);
        bmcv_rect_t crop_rect = {0, 0, image.width, image.height};
        bmcv_image_vpp_convert(objectMetadata->mFrame->mHandle, 1,
                                image_aligned, &imageStorage, &crop_rect);
        if (need_copy) bm_image_destroy(image_aligned);
    }
    for (auto subObj : objectMetadata->mPosedObjectMetadatas) {
        _render_pose_keypoints_bmcv(objectMetadata->mFrame->mHandle, imageStorage,
                              subObj->keypoints, 0.05, 1.0, subObj->modeltype,
                              0);
    }
    void* jpeg_data = NULL;
    size_t out_size = 0;
    int ret = bmcv_image_jpeg_enc(objectMetadata->mFrame->mHandle, 1,
                                  &imageStorage, &jpeg_data, &out_size);

    if (ret == BM_SUCCESS) {
        std::string img_file =
            out_dir + "/" +
            std::to_string(objectMetadata->mFrame->mChannelId) + "_" +
            std::to_string(objectMetadata->mFrame->mFrameId) + ".jpg";
        FILE* fp = fopen(img_file.c_str(), "wb");
        fwrite(jpeg_data, out_size, 1, fp);
        fclose(fp);
    }
    free(jpeg_data);
    bm_image_destroy(imageStorage);
}

void draw_retinaface_results(std::shared_ptr<sophon_stream::common::ObjectMetadata> objectMetadata, std::string& out_dir)
{
    bm_image imageStorage;
    _gen_storage_image(objectMetadata, imageStorage);
    _draw_face_rectangle_bmcv(objectMetadata->mFrame->mHandle, objectMetadata, imageStorage);

    // save image
    void* jpeg_data = NULL;
    size_t out_size = 0;
    int ret = bmcv_image_jpeg_enc(objectMetadata->mFrame->mHandle, 1,
                                &imageStorage, &jpeg_data, &out_size);
    if (ret == BM_SUCCESS) {
        std::string img_file =
            out_dir + "/" + std::to_string(objectMetadata->mFrame->mChannelId) +
            "_" + std::to_string(objectMetadata->mFrame->mFrameId) + ".jpg";
        FILE* fp = fopen(img_file.c_str(), "wb");
        fwrite(jpeg_data, out_size, 1, fp);
        fclose(fp);
    }
    free(jpeg_data);
    bm_image_destroy(imageStorage);
}

void draw_retinaface_distributor_resnet_faiss_converger_results(std::shared_ptr<sophon_stream::common::ObjectMetadata> objectMetadata, std::string& out_dir)
{
    if (objectMetadata->mSubObjectMetadatas.size() > 0) {
        bm_image imageStorage;
        _gen_storage_image(objectMetadata, imageStorage);
        for (auto subObj : objectMetadata->mSubObjectMetadatas) {
            int subId = subObj->mSubId;
            auto faceObj =
                objectMetadata->mFaceObjectMetadatas[subId];  // 第一张脸
            auto resnetObj =
                subObj->mRecognizedObjectMetadatas[0];  // 第一张脸对应的resnet
            int class_id = subObj->mRecognizedObjectMetadatas[0]->mTopKLabels[0];
            auto label = subObj->mRecognizedObjectMetadatas[0]->mLabelName;

            bmcv_rect_t rect;
            rect.start_x = std::max(faceObj->left, 0);
            rect.start_y = std::max(faceObj->top, 0);
            rect.crop_w = std::max(faceObj->right - faceObj->left + 1, 0);
            rect.crop_h = std::max(faceObj->bottom - faceObj->top + 1, 0);

            _draw_text_bmcv(objectMetadata->mFrame->mHandle, rect.start_x,
                        rect.start_y, imageStorage,
                        label);

            std::cout << "label:" << label << std::endl;
        }

        std::string filename =
            out_dir + "/" + std::to_string(objectMetadata->mFrame->mChannelId) +
            "-" + std::to_string(objectMetadata->mFrame->mFrameId) + ".bmp";
        bm_image_write_to_bmp(imageStorage, filename.c_str());
        bm_image_destroy(imageStorage);
    }
}

void draw_yolov5_results(std::shared_ptr<sophon_stream::common::ObjectMetadata> objectMetadata, std::string& out_dir, std::vector<std::string>& class_names)
{
    bm_image imageStorage;
    _gen_storage_image(objectMetadata, imageStorage);
    int colors_num = colors.size();

    for (auto detObj : objectMetadata->mDetectedObjectMetadatas) {
        int class_id = detObj->mClassify;
        std::string label = class_names[class_id] + ":" + cv::format("%.2f", detObj->mScores[0]);
        _draw_rectangle_and_text_bmcv(objectMetadata->mFrame->mHandle, label,
                  detObj->mBox.mX,
                  detObj->mBox.mY, detObj->mBox.mWidth, detObj->mBox.mHeight,
                  imageStorage, colors[class_id % colors_num], true);
    }
    // save image
    void* jpeg_data = NULL;
    size_t out_size = 0;
    int ret = bmcv_image_jpeg_enc(objectMetadata->mFrame->mHandle, 1,
                                &imageStorage, &jpeg_data, &out_size);
    if (ret == BM_SUCCESS) {
        std::string img_file =
            out_dir + "/" + std::to_string(objectMetadata->mFrame->mChannelId) +
            "_" + std::to_string(objectMetadata->mFrame->mFrameId) + ".jpg";
        FILE* fp = fopen(img_file.c_str(), "wb");
        fwrite(jpeg_data, out_size, 1, fp);
        fclose(fp);
    }
    free(jpeg_data);
    bm_image_destroy(imageStorage);
}

void draw_yolov5_bytetrack_distributor_resnet_converger_results(std::shared_ptr<sophon_stream::common::ObjectMetadata> objectMetadata, std::string& out_dir, std::vector<std::string>& car_attr, std::vector<std::string>& person_attr)
{
    if (objectMetadata->mSubObjectMetadatas.size() > 0) {
        bm_image imageStorage;
        _gen_storage_image(objectMetadata, imageStorage);
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
            _draw_rectangle_and_text_bmcv(objectMetadata->mFrame->mHandle, label,
                        rect.start_x, rect.start_y, rect.crop_w, rect.crop_h,
                        imageStorage, colors[trackObj->mTrackId % colors_num], true);
        }
        std::string filename =
            out_dir + "/" + std::to_string(objectMetadata->mFrame->mChannelId) +
            "-" + std::to_string(objectMetadata->mFrame->mFrameId) + ".bmp";
        bm_image_write_to_bmp(imageStorage, filename.c_str());
        bm_image_destroy(imageStorage);
    }
}

void draw_yolox_results(std::shared_ptr<sophon_stream::common::ObjectMetadata> objectMetadata, std::string& out_dir, std::vector<std::string>& class_names)
{
    bm_image imageStorage;
    _gen_storage_image(objectMetadata, imageStorage);
    int colors_num = colors.size();

    for (auto detObj : objectMetadata->mDetectedObjectMetadatas) {
        std::string label = class_names[detObj->mClassify] + ":" + cv::format("%.2f", detObj->mScores[0]);
        _draw_rectangle_and_text_bmcv(objectMetadata->mFrame->mHandle, label,
                  detObj->mBox.mX,
                  detObj->mBox.mY, detObj->mBox.mWidth, detObj->mBox.mHeight,
                  imageStorage, colors[detObj->mClassify % colors_num], true);
    }
    // save image
    void* jpeg_data = NULL;
    size_t out_size = 0;
    int ret = bmcv_image_jpeg_enc(objectMetadata->mFrame->mHandle, 1,
                                    &imageStorage, &jpeg_data, &out_size);
    if (ret == BM_SUCCESS) {
        std::string img_file =
            out_dir + "/" + std::to_string(objectMetadata->mFrame->mChannelId) +
            "_" + std::to_string(objectMetadata->mFrame->mFrameId) + ".jpg";
        FILE* fp = fopen(img_file.c_str(), "wb");
        fwrite(jpeg_data, out_size, 1, fp);
        fclose(fp);
    }
    free(jpeg_data);
    bm_image_destroy(imageStorage);
}

void save_only(std::shared_ptr<sophon_stream::common::ObjectMetadata> objectMetadata, std::string& out_dir) {
    std::string filename =
            out_dir + "/" + std::to_string(objectMetadata->mFrame->mChannelId) +
            "-" + std::to_string(objectMetadata->mFrame->mFrameId) + ".bmp";
    bm_image_write_to_bmp(*objectMetadata->mFrame->mSpData, filename.c_str());
    return;
}

void draw_default(std::shared_ptr<sophon_stream::common::ObjectMetadata> objectMetadata) {
    return;
}

void draw_yolov5_fastpose_posec3d_results(std::shared_ptr<sophon_stream::common::ObjectMetadata> objectMetadata, std::string& out_dir, std::string loss_type) {
    bm_image imageStorage;
    _gen_storage_image(objectMetadata, imageStorage);
    if (objectMetadata->mRecognizedObjectMetadatas.size() != 0)
        _draw_text_bmcv(objectMetadata->mFrame->mHandle, 50, 10, imageStorage, 
            objectMetadata->mRecognizedObjectMetadatas[0]->mLabelName + ":" + cv::format("%.2f", objectMetadata->mRecognizedObjectMetadatas[0]->mScores[0]));
    for (auto subObj : objectMetadata->mPosedObjectMetadatas) {
        _render_fastpose_keypoints(objectMetadata->mFrame->mHandle, imageStorage,
                                subObj->keypoints, subObj->scores,
                                loss_type, 1.0);
    }
    for (auto subObj : objectMetadata->mDetectedObjectMetadatas) {
        std::string label = "person:" + cv::format("%.2f", subObj->mScores[0]);
        _draw_rectangle_and_text_bmcv(objectMetadata->mFrame->mHandle, label,
                  subObj->mBox.mX, subObj->mBox.mY, subObj->mBox.mWidth,
                  subObj->mBox.mHeight, imageStorage, colors[0], true);
    }
    void* jpeg_data = NULL;
    size_t out_size = 0;
    int ret = bmcv_image_jpeg_enc(objectMetadata->mFrame->mHandle, 1,
                                    &imageStorage, &jpeg_data, &out_size);

    if (ret == BM_SUCCESS) {
        std::string img_file =
            out_dir + "/" +
            std::to_string(objectMetadata->mFrame->mChannelId) + "_" +
            std::to_string(objectMetadata->mFrame->mFrameId) + ".jpg";
        FILE* fp = fopen(img_file.c_str(), "wb");
        fwrite(jpeg_data, out_size, 1, fp);
        fclose(fp);
    
    }
    free(jpeg_data);
    bm_image_destroy(imageStorage);
}