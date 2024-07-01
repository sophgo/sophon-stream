//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_ELEMENT_FASTPOSE_POST_PROCESS_H_
#define SOPHON_STREAM_ELEMENT_FASTPOSE_POST_PROCESS_H_

#define USE_OPENCV 1
#include <opencv2/opencv.hpp>

#include "algorithmApi/post_process.h"
#include "fastpose_context.h"

using namespace sophon_stream::common;

namespace sophon_stream {
namespace element {
namespace fastpose {

struct PoseNMSParams {
  float delta1;
  float mu;
  float delta2;
  float gamma;
  float scoreThreds;
  float matchThreds;
  float alpha;
  float face_factor;
  float hand_factor;
  float hand_weight_score;
  float face_weight_score;
  float hand_weight_dist;
  float face_weight_dist;
};

class FastposePostProcess : public ::sophon_stream::element::PostProcess {
 public:
  void init(std::shared_ptr<FastposeContext> context);
  /**
   * @brief 对一个batch的数据做后处理
   * @param context context指针
   * @param objectMetadatas 一个batch的数据
   */
  void postProcess(std::shared_ptr<FastposeContext> context,
                   common::ObjectMetadatas& objectMetadatas);

 private:
  std::shared_ptr<PoseNMSParams> pose_nms_params;

  void getMaxPreds(float* ptr, int num_joints, int h, int w,
                   std::shared_ptr<common::PosedObjectMetadata>& poseData);
  void getAffineTransform2d(float* center, float* scale, float* output_size,
                            bool inv, cv::Mat* trans);
  void affineTransform2d(std::vector<float>& keypoints, int i, cv::Mat* trans);
  void heatmapToCoordSimple(
      float* ptr, common::Rectangle<int> bbox, int num_joints, int h, int w,
      std::shared_ptr<common::PosedObjectMetadata>& poseData);
  void poseNMS(
      std::vector<std::shared_ptr<common::DetectedObjectMetadata>>& det_data,
      int num_samples, int num_joints, float areaThres,
      std::vector<std::shared_ptr<common::PosedObjectMetadata>>& body_keypoints,
      std::vector<int>& pick_ids);
  void poseNMSBody(
      std::vector<std::shared_ptr<common::DetectedObjectMetadata>>& det_data,
      int num_samples, int num_joints, float areaThres,
      std::vector<std::shared_ptr<common::PosedObjectMetadata>>& body_keypoints,
      std::vector<int>& pick_ids);
  void getParametricDistance(
      std::vector<std::shared_ptr<common::PosedObjectMetadata>>& body_keypoints,
      std::vector<float>& dist, std::vector<int>& dist_indx, int pick_id,
      int num_joints, bool use_dist_mask, float* final_dist);
  void PCKMatch(std::vector<float>& dist, int num_joints, float ref_dist,
                int* num_match_keypoints);
  void PCKMatchFullBody(
      std::vector<float>& dist,
      std::vector<std::shared_ptr<common::PosedObjectMetadata>>& body_keypoints,
      int pick_id, int num_joints, float ref_dist, int* num_match_keypoints);
  void pMergeFast(
      std::vector<std::shared_ptr<common::PosedObjectMetadata>>& body_keypoints,
      std::vector<int>& merge_ids, std::vector<float>& dist,
      std::vector<int>& dist_indx, int pick_id, int num_joints, float ref_dist);
  void poseNMSFullBody(
      std::vector<std::shared_ptr<common::DetectedObjectMetadata>>& det_data,
      int num_samples, int num_joints, float areaThres,
      std::vector<std::shared_ptr<common::PosedObjectMetadata>>& body_keypoints,
      std::vector<int>& pick_ids);
  void getKeyPoints(
      std::vector<std::shared_ptr<BMNNTensor>> outputTensors,
      std::vector<std::shared_ptr<common::DetectedObjectMetadata>>& det_data,
      const bm_image& image,
      std::vector<std::shared_ptr<common::PosedObjectMetadata>>& body_keypoints,
      float areaThres, HeatmapLossType heatmap_loss);
};

}  // namespace fastpose
}  // namespace element
}  // namespace sophon_stream

#endif  // SOPHON_STREAM_ELEMENT_FASTPOSE_POST_PROCESS_H_
