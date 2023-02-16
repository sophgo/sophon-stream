#pragma once

#include <memory>
#include <string>
#include <vector>

namespace sophon_stream {
namespace common {

struct LabelMetadata {
    std::string mFeature;
};

struct RecognizedObjectMetadata {
    RecognizedObjectMetadata()
        : mTrackFeature(false), 
          mTrackCosineSimilarityThreshold(0.f) {
    }
    
    int getLabel() const {
        if (mTopKLabels.empty()) {
            return -1;
        } else {
            return mTopKLabels.front();
        }
    }

    float getScore() const {
        int label = getLabel();
        if (label < 0
                || label >= mScores.size()) {
            return 0.f;
        } else {
            return mScores[label];
        }
    }

    bool mTrackFeature;
    float mTrackCosineSimilarityThreshold;
    std::string mItemName;
    std::string mLabelName;
    std::vector<float> mScores;
    std::vector<int> mTopKLabels;
    std::vector<std::shared_ptr<LabelMetadata> > mTopKLabelMetadatas;
};

} // namespace common
} // namespace sophon_stream

