//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_COMMON_DETECTED_OBJECT_METADATA_H_
#define SOPHON_STREAM_COMMON_DETECTED_OBJECT_METADATA_H_

#include <memory>
#include <string>
#include <vector>

#include "graphics.h"

namespace sophon_stream {
namespace common {

struct PointMetadata {
  int getLabel() const {
    if (mTopKLabels.empty()) {
      return -1;
    } else {
      return mTopKLabels.front();
    }
  }

  float getScore() const {
    int label = getLabel();
    if (label < 0 || label >= mScores.size()) {
      return 0.f;
    } else {
      return mScores[label];
    }
  }

  common::Point<int> mPoint;
  std::vector<float> mScores;
  std::vector<int> mTopKLabels;
};

struct DetectedObjectMetadata {
  DetectedObjectMetadata() : mClassify(-1), mTrackIouThreshold(0.f) {}

  int getLabel() const {
    if (mTopKLabels.empty()) {
      return -1;
    } else {
      return mTopKLabels.front();
    }
  }

  float getScore() const {
    int label = getLabel();
    if (label < 0 || label >= mScores.size()) {
      return 0.f;
    } else {
      return mScores[label];
    }
  }

  common::Rectangle<int> mBox;
  std::string mItemName;
  std::string mLabelName;
  std::vector<float> mScores;
  std::vector<int> mTopKLabels;
  int mClassify;
  std::string mClassifyName;
  float mTrackIouThreshold;
  std::vector<std::shared_ptr<PointMetadata> > mKeyPoints;
};

}  // namespace common
}  // namespace sophon_stream

#endif  // SOPHON_STREAM_COMMON_DETECTED_OBJECT_METADATA_H_