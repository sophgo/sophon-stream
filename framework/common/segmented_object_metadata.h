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

#include "frame.h"

namespace sophon_stream {
namespace common {

struct SegmentedObjectMetadata {
  SegmentedObjectMetadata() {}

  // std::string mItemName;
  // std::string mLabelName;
  std::shared_ptr<Frame> mFrame;
  // std::vector<float> mScores;
  // std::vector<int> mTopKLabels;
  // std::vector<std::shared_ptr<LabelMetadata> > mTopKLabelMetadatas;
};

}  // namespace common
}  // namespace sophon_stream

#endif  // SOPHON_STREAM_COMMON_SEGMENTED_OBJECT_METADATA_H_