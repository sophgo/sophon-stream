#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "ErrorCode.h"
#include "Frame.h"
#include "Packet.h"
#include "Graphics.hpp"
#include "TrackedObjectMetadata.h"
#include "DetectedObjectMetadata.h"
#include "RecognizedObjectMetadata.h"
#include "SegmentedObjectMetadata.h"
#include "common/bmnn_utils.h"

namespace sophon_stream {
namespace common {

using ModelConfigureMap = std::map<std::string, bool>;

struct PointData{
    common::Point<int> mPoint;
    float mScore;
};

struct DataInformation {
    common::Rectangle<int> mBox;//位置信息
    std::string mItemName;  //
    std::string mLabelName; //label名称
    float mScore;           //分数
    std::string mLabel;     //label属性值
    int mClassify;          //粗分类
    std::string mClassifyName; //粗分类的名称
    std::map<int,PointData> mKeyPoints; //关键点坐标 key是索引 , value是坐标信息，比如用于肢体检测，人脸就是5点坐标
};


struct ObjectMetadata {
    ObjectMetadata()
        : mErrorCode(common::ErrorCode::SUCCESS), 
          mFilter(false) {
    }

    int getChannelId() const {
        if (mFrame) {
            return mFrame->mChannelId;
        } else if (mPacket) {
            return mPacket->mChannelId;
        } else {
            return -1;
        }
    }

    std::int64_t getFrameId() const {
        if (mFrame) {
            return mFrame->mFrameId;
        } else if (mPacket) {
            return mPacket->mPacketId;
        } else {
            return -1;
        }
    }

    std::int64_t getTimestamp() const {
        if (mFrame) {
            return mFrame->mTimestamp;
        } else if (mPacket) {
            return mPacket->mTimestamp;
        } else {
            return 0;
        }
    }

    bool getEndofStream() const {
        if (mFrame) {
            return mFrame->mEndOfStream;
        } else if (mPacket) {
            return mPacket->mEndOfStream;
        } else {
            return false;
        }
    }

    common::ErrorCode mErrorCode;

    std::shared_ptr<common::Packet> mPacket;
    std::shared_ptr<common::Frame> mFrame;

    std::shared_ptr<bm_handle_t> mAlgorithmHandle;

    bool mFilter;

    std::vector<std::shared_ptr<BMNNTensor>> mOutputTensors;

    std::shared_ptr<ModelConfigureMap> mModelConfigureMap;
    std::shared_ptr<DataInformation> mSpDataInformation;//包含mTransFromFrame和原detect原recognize
    std::shared_ptr<common::Frame> mTransformFrame;
    std::shared_ptr<common::TrackedObjectMetadata> mTrackedObjectMetadata;
    std::vector<std::shared_ptr<ObjectMetadata> > mSubObjectMetadatas;

    std::shared_ptr<common::DetectedObjectMetadata> mDetectedObjectMetadata;
    std::vector<std::shared_ptr<common::RecognizedObjectMetadata>> mRecognizedObjectMetadatas;

    std::vector<std::shared_ptr<common::SegmentedObjectMetadata>> mSegmentedObjectMetadatas;

};

using ObjectMetadatas = std::vector<std::shared_ptr<ObjectMetadata> >;

} // namespace common
} // namespace sophon_stream

