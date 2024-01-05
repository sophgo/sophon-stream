//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_ELEMENT_PPOCR_DET_POST_PROCESSOR_H_
#define SOPHON_STREAM_ELEMENT_PPOCR_DET_POST_PROCESSOR_H_

#include <iostream>
#include <memory>
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>

namespace sophon_stream {
namespace element {
namespace ppocr_det {

struct OCRBox {
  int x1, y1, x2, y2, x3, y3, x4, y4;
  std::string rec_res;
  float score;
  void printInfo() {
    printf("Box info: (%d, %d); (%d, %d); (%d, %d); (%d, %d) \n", x1, y1, x2,
           y2, x3, y3, x4, y4);
  }
};

using OCRBoxVec = std::vector<OCRBox>;

class PostProcessor {
 public:
  std::vector<std::vector<std::vector<int>>> BoxesFromBitmap(
      const cv::Mat pred, const cv::Mat bitmap, const float& box_thresh,
      const float& det_db_unclip_ratio, const bool& use_polygon_score,
      const int& dest_width, const int& dest_height);
  std::vector<std::vector<float>> GetMiniBoxes(cv::RotatedRect box,
                                               float& ssid);
  float PolygonScoreAcc(std::vector<cv::Point> contour, cv::Mat pred);
  float BoxScoreFast(std::vector<std::vector<float>> box_array, cv::Mat pred);
  cv::RotatedRect UnClip(std::vector<std::vector<float>> box,
                         const float& unclip_ratio);
  void GetContourArea(const std::vector<std::vector<float>>& box,
                      float unclip_ratio, float& distance);
  OCRBoxVec FilterTagDetRes(std::vector<std::vector<std::vector<int>>> boxes,
                            bm_image input_bmimg);
  std::vector<std::vector<int>> OrderPointsClockwise(
      std::vector<std::vector<int>> pts);

 private:
  static bool XsortFp32(std::vector<float> a, std::vector<float> b);
  static bool XsortInt(std::vector<int> a, std::vector<int> b);
  std::vector<std::vector<float>> Mat2Vector(cv::Mat mat);
  template <class T>
  inline T clamp(T x, T min, T max) {
    if (x > max) return max;
    if (x < min) return min;
    return x;
  }

  inline float clampf(float x, float min, float max) {
    if (x > max) return max;
    if (x < min) return min;
    return x;
  }

  inline int _max(int a, int b) { return a >= b ? a : b; }

  inline int _min(int a, int b) { return a >= b ? b : a; }
};
}  // namespace ppocr_det
}  // namespace element
}  // namespace sophon_stream
#endif