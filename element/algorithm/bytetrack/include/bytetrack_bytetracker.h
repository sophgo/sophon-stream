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
#include "common/error_code.h"
#include "common/object_metadata.h"
#include "common/logger.h"
#include "element.h"

namespace sophon_stream {
namespace element {
namespace bytetrack {

struct BytetrackContext {
  float trackThresh;
  float highThresh;
  float matchThresh;
  int frameRate;
  int trackBuffer;
  int minBoxArea;
  bool correctBox;
  bool agnostic;
};

class BYTETracker {
 public:
  BYTETracker(const std::shared_ptr<BytetrackContext> mContext);
  ~BYTETracker();

  void update(std::shared_ptr<common::ObjectMetadata>& objects);

 private:
  void joint_stracks(STracks& tlista, STracks& tlistb, STracks& results);

  void sub_stracks(STracks& tlista, STracks& tlistb);

  void remove_duplicate_stracks(STracks& resa, STracks& resb, STracks& stracksa,
                                STracks& stracksb);

  void linear_assignment(std::vector<std::vector<float>>& cost_matrix,
                         int cost_matrix_size, int cost_matrix_size_size,
                         float thresh, std::vector<std::vector<int>>& matches,
                         std::vector<int>& unmatched_a,
                         std::vector<int>& unmatched_b);

  void iou_distance(const STracks& atracks, const STracks& btracks,
                    std::vector<std::vector<float>>& cost_matrix);

  void ious(std::vector<std::vector<float>>& atlbrs,
            std::vector<std::vector<float>>& btlbrs,
            std::vector<std::vector<float>>& results);

  void lapjv(const std::vector<std::vector<float>>& cost,
             std::vector<int>& rowsol, std::vector<int>& colsol,
             bool extend_cost = false, float cost_limit = LONG_MAX,
             bool return_cost = true);

 private:
  float track_thresh;
  float high_thresh;
  float match_thresh;
  int frame_rate;
  int track_buffer;
  int min_box_area;
  int frame_id;
  int max_time_lost;
  int class_offset;
  bool correct_box;
  bool agnostic;

  STracks tracked_stracks;
  STracks lost_stracks;
  STracks removed_stracks;

  std::shared_ptr<KalmanFilter> kalman_filter;
};

}  // namespace bytetrack
}  // namespace element
}  // namespace sophon_stream

#endif  // SOPHON_STREAM_ELEMENT_BYTETRACK_BYTETRACKER_H_