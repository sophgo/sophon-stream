//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-DEMO is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_ELEMENT_BYTETRACK_BYTETRACKER_H_
#define SOPHON_STREAM_ELEMENT_BYTETRACK_BYTETRACKER_H_

#include <opencv2/opencv.hpp>

#include "bytetrack_lapjv.h"
#include "bytetrack_strack.h"
#include "common/ErrorCode.h"
#include "common/ObjectMetadata.h"
#include "common/logger.h"
#include "element.h"

namespace sophon_stream {
namespace element {
namespace bytetrack {

struct SaveResult {
  int frame_id;
  int track_id;
  std::vector<float> tlwh;
};

struct DeteBox {
  int x, y, width, height;
  float score;
  int class_id;
};

class BYTETracker {
 public:
  BYTETracker(int frame_rate = 30, int track_buffer = 30,
              float track_thresh = 0.6, float high_thresh = 0.7,
              float match_thresh = 0.8);
  ~BYTETracker();

  void update(std::shared_ptr<common::ObjectMetadata>& objects);
  cv::Scalar get_color(int idx);
  std::tuple<int, int, int> get_color_tuple(int idx);

 private:
  std::vector<STrack*> joint_stracks(std::vector<STrack*>& tlista,
                                     std::vector<STrack>& tlistb);
  std::vector<STrack> joint_stracks(std::vector<STrack>& tlista,
                                    std::vector<STrack>& tlistb);

  std::vector<STrack> sub_stracks(std::vector<STrack>& tlista,
                                  std::vector<STrack>& tlistb);
  void remove_duplicate_stracks(std::vector<STrack>& resa,
                                std::vector<STrack>& resb,
                                std::vector<STrack>& stracksa,
                                std::vector<STrack>& stracksb);

  void linear_assignment(std::vector<std::vector<float>>& cost_matrix,
                         int cost_matrix_size, int cost_matrix_size_size,
                         float thresh, std::vector<std::vector<int>>& matches,
                         std::vector<int>& unmatched_a,
                         std::vector<int>& unmatched_b);
  std::vector<std::vector<float>> iou_distance(std::vector<STrack*>& atracks,
                                               std::vector<STrack>& btracks,
                                               int& dist_size,
                                               int& dist_size_size);
  std::vector<std::vector<float>> iou_distance(std::vector<STrack>& atracks,
                                               std::vector<STrack>& btracks);
  std::vector<std::vector<float>> ious(std::vector<std::vector<float>>& atlbrs,
                                       std::vector<std::vector<float>>& btlbrs);

  double lapjv(const std::vector<std::vector<float>>& cost,
               std::vector<int>& rowsol, std::vector<int>& colsol,
               bool extend_cost = false, float cost_limit = LONG_MAX,
               bool return_cost = true);

 private:
  int frame_rate;
  int track_buffer;
  float track_thresh;
  float high_thresh;
  float match_thresh;
  int frame_id;
  int max_time_lost;

  std::vector<STrack> tracked_stracks;
  std::vector<STrack> lost_stracks;
  std::vector<STrack> removed_stracks;
  KalmanFilter kalman_filter;
};

}  // namespace bytetrack
}  // namespace element
}  // namespace sophon_stream

#endif  // SOPHON_STREAM_ELEMENT_BYTETRACK_BYTETRACKER_H_