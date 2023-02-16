#pragma once

#include <string>
#include <vector>

namespace sophon_stream {
namespace common {

enum TrackFlag {
    TrNormal,
    TrLast
};

struct TrackedObjectMetadata {
    TrackedObjectMetadata()
        : mPerferScore(0.f),
          mCoverArea(0) {
    }

    std::string mUuid;
    float mPerferScore;
    int mCoverArea;

    //新加的
    std::string mName;
    bool mTrackerFilter = false;
    long long mTrackId = -1; // 跟踪id
    int mTrackFlag = TrNormal;
    float mQualityScore = 0.0;
    std::string mCaptureTime;
    std::string mImagePath;
};

} // namespace common
} // namespace sophon_stream

