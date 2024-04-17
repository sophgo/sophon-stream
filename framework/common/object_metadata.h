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
#include "face_object_metadata.h"
#include "frame.h"
#include "graphics.h"
#include "posed_object_metadata.h"
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
  // cpu data is used to sync dev mem and host mem
  std::vector<float*> cpu_data;
} bmTensors;

typedef struct bmSubTensors_ {
  std::vector<std::vector<std::shared_ptr<bm_tensor_t>>> tensors;
  bm_handle_t handle;
} bmSubTensors;

struct ObjectMetadata {
  ObjectMetadata()
      : mErrorCode(common::ErrorCode::SUCCESS),
        mFilter(false),
        is_main(false),
        numBranches(0) {}

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

  /**
   * @brief
   * 跳过的element，由channelTask配置，可以实现不同channel经过不同的pipeline
   */
  std::vector<int> mSkipElements;

  std::shared_ptr<bmTensors> mInputBMtensors;
  std::shared_ptr<bmTensors> mOutputBMtensors;

  std::shared_ptr<bmSubTensors> mSubInputBMtensors;
  std::shared_ptr<bmSubTensors> mSubOutputBMtensors;

  std::shared_ptr<ModelConfigureMap> mModelConfigureMap;
  std::shared_ptr<DataInformation>
      mSpDataInformation;  // 包含mTransFromFrame和原detect原recognize
  std::shared_ptr<common::Frame> mTransformFrame;
  std::vector<std::shared_ptr<ObjectMetadata>> mSubObjectMetadatas;
  int tag;

  int numBranches;
  int mSubId;
  /**
   * @brief
   * 用于posec3d插件中的推理和后处理，表示ObjectMetadata是否包含了多帧输入
   */
  bool is_main;

  /**
   * @brief 跟踪结果的vector，一个目标对应一个TrackedObjectMetadata
   */
  std::vector<std::shared_ptr<common::TrackedObjectMetadata>>
      mTrackedObjectMetadatas;
  /**
   * @brief 检测结果的vector，一个目标对应一个DetectedObjectMetadata
   */
  std::vector<std::shared_ptr<common::DetectedObjectMetadata>>
      mDetectedObjectMetadatas;
  /**
   * @brief 姿态结果的vector，一个目标对应一个PosedObjectMetadata
   */
  std::vector<std::shared_ptr<common::PosedObjectMetadata>>
      mPosedObjectMetadatas;
  /**
   * @brief
   * 识别结果的vector，一个原始ObjectMetadata通过多个模型，每个模型的结果对应一个RecognizedObjectMetadata
   */
  std::vector<std::shared_ptr<common::RecognizedObjectMetadata>>
      mRecognizedObjectMetadatas;
  /**
   * @brief
   * 分割结果的vector，一个原始ObjectMetadata通过多个模型，每个模型的结果对应一个SegmentedObjectMetadata
   */
  std::vector<std::shared_ptr<common::SegmentedObjectMetadata>>
      mSegmentedObjectMetadatas;

  /**
   * @brief 检测结果的vector，一个目标对应一个FaceObjectMetadata
   */
  std::vector<std::shared_ptr<common::FaceObjectMetadata>> mFaceObjectMetadatas;

  /**
   * @brief ocr预处理，以net_w为最长边，进行resize后的宽高
   */
  std::vector<int> resize_vector;
  std::vector<std::vector<common::Point<int>>> areas;
};

using ObjectMetadatas = std::vector<std::shared_ptr<ObjectMetadata>>;

}  // namespace common
}  // namespace sophon_stream

#endif  // SOPHON_STREAM_COMMON_OBJECT_METADATA_H_