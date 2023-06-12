//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_COMMON_OBJECT_METADATA_H_
#define SOPHON_STREAM_COMMON_OBJECT_METADATA_H_

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "common/bmnn_utils.h"
#include "detected_object_metadata.h"
#include "error_code.h"
#include "frame.h"
#include "graphics.h"
#include "recognized_object_metadata.h"
#include "segmented_object_metadata.h"
#include "tracked_object_metadata.h"

namespace sophon_stream {
namespace common {

using ModelConfigureMap = std::map<std::string, bool>;

struct PointData {
  common::Point<int> mPoint;
  float mScore;
};

struct DataInformation {
  common::Rectangle<int> mBox;  // 位置信息
  std::string mItemName;        //
  std::string mLabelName;       // label名称
  float mScore;                 // 分数
  std::string mLabel;           // label属性值
  int mClassify;                // 粗分类
  std::string mClassifyName;    // 粗分类的名称
  std::map<int, PointData>
      mKeyPoints;  // 关键点坐标 key是索引 ,
                   // value是坐标信息，比如用于肢体检测，人脸就是5点坐标
};

typedef struct bmTensors_ {
  std::vector<std::shared_ptr<bm_tensor_t>> tensors;
  bm_handle_t handle;
} bmTensors;

struct ObjectMetadata {
  ObjectMetadata() : mErrorCode(common::ErrorCode::SUCCESS), mFilter(false) {}

  int getChannelId() const {
    if (mFrame) {
      return mFrame->mChannelId;
    } else {
      return -1;
    }
  }

  std::int64_t getFrameId() const {
    if (mFrame) {
      return mFrame->mFrameId;
    } else {
      return -1;
    }
  }

  std::int64_t getTimestamp() const {
    if (mFrame) {
      return mFrame->mTimestamp;
    } else {
      return 0;
    }
  }

  bool getEndofStream() const {
    if (mFrame) {
      return mFrame->mEndOfStream;
    } else {
      return false;
    }
  }

  common::ErrorCode mErrorCode;

  std::shared_ptr<common::Frame> mFrame;

  bool mFilter;

  std::shared_ptr<bmTensors> mInputBMtensors;
  std::shared_ptr<bmTensors> mOutputBMtensors;

  std::shared_ptr<ModelConfigureMap> mModelConfigureMap;
  std::shared_ptr<DataInformation>
      mSpDataInformation;  // 包含mTransFromFrame和原detect原recognize
  std::shared_ptr<common::Frame> mTransformFrame;
  std::shared_ptr<common::TrackedObjectMetadata> mTrackedObjectMetadata;
  std::vector<std::shared_ptr<ObjectMetadata>> mSubObjectMetadatas;

  std::shared_ptr<common::DetectedObjectMetadata> mDetectedObjectMetadata;
  std::vector<std::shared_ptr<common::RecognizedObjectMetadata>>
      mRecognizedObjectMetadatas;

  std::vector<std::shared_ptr<common::SegmentedObjectMetadata>>
      mSegmentedObjectMetadatas;
};

using ObjectMetadatas = std::vector<std::shared_ptr<ObjectMetadata>>;

}  // namespace common
}  // namespace sophon_stream

#endif  // SOPHON_STREAM_COMMON_OBJECT_METADATA_H_