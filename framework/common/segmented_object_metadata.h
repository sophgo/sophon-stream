//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_COMMON_SEGMENTED_OBJECT_METADATA_H_
#define SOPHON_STREAM_COMMON_SEGMENTED_OBJECT_METADATA_H_

#include <memory>
#include <string>
#include <vector>

#include "opencv2/opencv.hpp"
#include "graphics.h"

namespace sophon_stream {
namespace common {

struct SegmentedObjectMetadata {
  SegmentedObjectMetadata() : mClassify(-1) {}

  common::Rectangle<int> mBox;  // Box results
  
  int mClassify;  // class_id
  std::vector<float> mScores;  // score
  cv::Mat mask_img; // the seg mask
  
  std::string mItemName;
  std::string mLabelName;
};

}  // namespace common
}  // namespace sophon_stream

#endif  // SOPHON_STREAM_COMMON_SEGMENTED_OBJECT_METADATA_H_