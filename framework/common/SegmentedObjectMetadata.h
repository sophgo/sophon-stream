#pragma once

#include <memory>
#include <string>
#include <vector>
#include "Frame.h"

namespace sophon_stream {
namespace common {

struct SegmentedObjectMetadata {
    SegmentedObjectMetadata(){}

    // std::string mItemName;
    // std::string mLabelName;
    std::shared_ptr<Frame> mFrame;
    // std::vector<float> mScores;
    // std::vector<int> mTopKLabels;
    // std::vector<std::shared_ptr<LabelMetadata> > mTopKLabelMetadatas;
};

} // namespace common
} // namespace sophon_stream

