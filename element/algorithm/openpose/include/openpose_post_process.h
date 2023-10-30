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
#include "group.h"
#include "openpose_context.h"
using namespace sophon_stream::common;

namespace sophon_stream {
namespace element {
namespace openpose {

typedef struct {
  unsigned long long input_data_addr;
  unsigned long long num_output_data_addr;
  unsigned long long aux_data_addr;
  int input_c;
  int input_h;
  int input_w;
  int max_peak_num;
  float nms_thresh;
} __attribute__((packed)) tpu_api_openpose_part_nms_postprocess_t;

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
class OpenposePostProcess : public ::sophon_stream::framework::PostProcess {
 public:
  void init(std::shared_ptr<OpenposeContext> context);
  /**
   * @brief 对一个batch的数据做后处理
   * @param context context指针
   * @param objectMetadatas 一个batch的数据
   */
  void postProcess(std::shared_ptr<OpenposeContext> context,
                   common::ObjectMetadatas& objectMetadatas, int dataPipeId);
  ~OpenposePostProcess() override;

 private:
  bm_device_mem_t** resize_output_map_whole_device_mem = nullptr;
  std::shared_ptr<OpenposeContext> global_context = nullptr;
  bm_device_mem_t **aux_data = nullptr, **output_num = nullptr;

  void nms(PoseBlobPtr bottom_blob, PoseBlobPtr top_blob, float threshold);
  void nmsFunc(float* ptr, float* top_ptr, int length, int h, int w,
               int max_peaks, float threshold, int plane_offset,
               int top_plane_offset);
  int kernel_part_nms(int dataPipeId, int input_h, int input_w,
                      int max_peak_num, float threshold, int* num_result,
                      float* score_out_result, int* coor_out_result,
                      PosedObjectMetadata::EModelType model_type,
                      std::shared_ptr<OpenposeContext> context);
  int resize_multi_channel(float* input, float* output,
                           bm_device_mem_t out_addr, int input_height,
                           int input_width, cv::Size outSize, bool use_memcpy,
                           int start_chan_idx, int end_chan_idx,
                           std::shared_ptr<OpenposeContext> context);
  void connectBodyPartsCpu(
      std::vector<std::shared_ptr<common::PosedObjectMetadata>>& poseKeypoints,
      const float* const heatMapPtr, const float* const peaksPtr,
      const cv::Size& heatMapSize, const int maxPeaks,
      const int interMinAboveThreshold, const float interThreshold,
      const int minSubsetCnt, const float minSubsetScore,
      const float scaleFactor, PosedObjectMetadata::EModelType model_type);
  void connectBodyPartsKernel(
      std::vector<std::shared_ptr<common::PosedObjectMetadata>>& poseKeypoints,
      const float* const heatMapPtr, const int* const num_result,
      const float* const score_out_result, const int* const coor_out_result,
      const float* const peaksPtr, const cv::Size& heatMapSize,
      const int maxPeaks, const int interMinAboveThreshold,
      const float interThreshold, const int minSubsetCnt,
      const float minSubsetScore, const float scaleFactor,
      PosedObjectMetadata::EModelType model_type);

  void getKeyPointsCPU(
      std::shared_ptr<BMNNTensor> tensorPtr, const bm_image& images,
      std::vector<std::shared_ptr<common::PosedObjectMetadata>>& body_keypoints,
      PosedObjectMetadata::EModelType model_type, float nms_threshold);

  void getKeyPointsTPUKERNEL(
      std::shared_ptr<BMNNTensor> outputTensorPtr, const bm_image& images,
      std::vector<std::shared_ptr<common::PosedObjectMetadata>>& body_keypoints,
      PosedObjectMetadata::EModelType model_type, float nms_threshold,
      std::shared_ptr<OpenposeContext> context, int dataPipeId);

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
