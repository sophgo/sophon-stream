//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_ELEMENT_OPENPOSE_POST_PROCESS_H_
#define SOPHON_STREAM_ELEMENT_OPENPOSE_POST_PROCESS_H_

#include <memory>
#include <string>
#include <vector>
#define USE_OPENCV 1
#include <opencv2/opencv.hpp>

#include "common/bmnn_utils.h"
#include "common/error_code.h"
#include "common/object_metadata.h"
#include "openpose_context.h"
using namespace sophon_stream::common;

namespace sophon_stream {
namespace element {
namespace openpose {

class PoseBlob : public NoCopyable,
                 public std::enable_shared_from_this<PoseBlob> {
  int m_count;
  int m_n, m_c, m_h, m_w;
  float* m_data;

 public:
  PoseBlob(int n, int c, int h, int w) : m_n(n), m_c(c), m_h(h), m_w(w) {
    m_count = n * c * w * h;
    m_data = new float_t[m_count];
  }

  ~PoseBlob() { delete[] m_data; }

  std::shared_ptr<PoseBlob> getPtr() { return shared_from_this(); }

  int height() { return m_h; }
  int width() { return m_w; }
  int channels() { return m_c; }
  int num() { return m_n; }

  float* data() { return m_data; }
};
using PoseBlobPtr = std::shared_ptr<PoseBlob>;
class OpenposePostProcess {
 public:
  void init(std::shared_ptr<OpenposeContext> context);
  /**
   * @brief 对一个batch的数据做后处理
   * @param context context指针
   * @param objectMetadatas 一个batch的数据
   */
  void postProcess(std::shared_ptr<OpenposeContext> context,
                   common::ObjectMetadatas& objectMetadatas);

 private:
  int nms(PoseBlobPtr bottom_blob, PoseBlobPtr top_blob, float threshold);

  void connectBodyPartsCpu(
      std::vector<std::shared_ptr<common::PosedObjectMetadata>>& poseKeypoints,
      const float* const heatMapPtr, const float* const peaksPtr,
      const cv::Size& heatMapSize, const int maxPeaks,
      const int interMinAboveThreshold, const float interThreshold,
      const int minSubsetCnt, const float minSubsetScore,
      const float scaleFactor, PosedObjectMetadata::EModelType model_type);

  int getKeyPoints(
      std::shared_ptr<BMNNTensor> tensorPtr, const bm_image& images,
      std::vector<std::shared_ptr<common::PosedObjectMetadata>>& body_keypoints,
      PosedObjectMetadata::EModelType model_type, float nms_threshold);

  std::vector<unsigned int> getPosePairs(
      PosedObjectMetadata::EModelType model_type);

  std::vector<unsigned int> getPoseMapIdx(
      PosedObjectMetadata::EModelType model_type);

  int getNumberBodyParts(PosedObjectMetadata::EModelType model_type);
};

}  // namespace openpose
}  // namespace element
}  // namespace sophon_stream

#endif  // SOPHON_STREAM_ELEMENT_OPENPOSE_POST_PROCESS_H_
