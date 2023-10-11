//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_COMMON_RECOGNIZED_OBJECT_METADATA_H_
#define SOPHON_STREAM_COMMON_RECOGNIZED_OBJECT_METADATA_H_

#include <memory>
#include <string>
#include <vector>

namespace sophon_stream {
namespace common {

struct LabelMetadata {
  std::string mFeature;
};

struct RecognizedObjectMetadata {
  RecognizedObjectMetadata() {}

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

  std::string mItemName;
  std::string mLabelName;
  std::vector<float> mScores;
  std::vector<int> mTopKLabels;
  std::vector<std::shared_ptr<LabelMetadata> > mTopKLabelMetadatas;
  std::shared_ptr<float> feature_vector;
};

}  // namespace common
}  // namespace sophon_stream

#endif  // SOPHON_STREAM_COMMON_RECOGNIZED_OBJECT_METADATA_H_