//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-DEMO is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_ELEMENT_BYTETRACK_KALMANFILTER_H_
#define SOPHON_STREAM_ELEMENT_BYTETRACK_KALMANFILTER_H_

#include <cstddef>
#include <opencv2/opencv.hpp>

namespace sophon_stream {
namespace element {
namespace bytetrack {

class KalmanFilter {
 public:
  static const double chi2inv95[10];
  KalmanFilter();
  ~KalmanFilter();
  std::pair<cv::Mat, cv::Mat> initiate(const cv::Mat& measurement);
  std::pair<cv::Mat, cv::Mat> predict(const cv::Mat& mean,
                                      const cv::Mat& covariance);
  std::pair<cv::Mat, cv::Mat> update(const cv::Mat& mean,
                                     const cv::Mat& covariance,
                                     const cv::Mat& measurement);
  cv::Mat gating_distance(const cv::Mat& mean, const cv::Mat& covariance,
                          const std::vector<cv::Mat>& measurements,
                          bool only_position = false);

 private:
  std::unique_ptr<cv::KalmanFilter> opencv_kf;
  float _std_weight_position;
  float _std_weight_velocity;
};

}  // namespace bytetrack
}  // namespace element
}  // namespace sophon_stream

#endif  // SOPHON_STREAM_ELEMENT_BYTETRACK_KALMANFILTER_H_