//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#include "fastpose_post_process.h"

#include <cmath>

#include "common/logger.h"

namespace sophon_stream {
namespace element {
namespace fastpose {

template <typename T>
inline int sign(T val) {
  return (T(0) < val) - (val < T(0));
}

void FastposePostProcess::init(std::shared_ptr<FastposeContext> context) {
  pose_nms_params = std::make_shared<PoseNMSParams>();
  pose_nms_params->face_factor = 1.9;
  pose_nms_params->hand_factor = 0.55;
  pose_nms_params->hand_weight_score = 0.1;
  pose_nms_params->face_weight_score = 1.0;
  pose_nms_params->hand_weight_dist = 1.5;
  pose_nms_params->face_weight_dist = 1.0;
  if ((context->m_net_channel == 136 || context->m_net_channel == 133) &&
      !(context->heatmap_loss == HeatmapLossType::MSELoss)) {
    pose_nms_params->delta1 = 1.0;
    pose_nms_params->mu = 1.65;
    pose_nms_params->delta2 = 8.0;
    pose_nms_params->gamma = 3.6;
    pose_nms_params->scoreThreds = 0.01;
    pose_nms_params->matchThreds = 3.0;
    pose_nms_params->alpha = 0.15;
  } else {
    pose_nms_params->delta1 = 1.0;
    pose_nms_params->mu = 1.7;
    pose_nms_params->delta2 = 2.65;
    pose_nms_params->gamma = 22.48;
    pose_nms_params->scoreThreds = 0.3;
    pose_nms_params->matchThreds = 5.0;
    pose_nms_params->alpha = 0.1;
  }
}

void FastposePostProcess::postProcess(
    std::shared_ptr<FastposeContext> context,
    common::ObjectMetadatas& objectMetadatas) {
  if (objectMetadatas.size() == 0) return;
  for (auto obj : objectMetadatas) {
    if (obj->mFrame->mEndOfStream) break;
    if (obj->mSubOutputBMtensors->tensors.size() == 0) continue;
    // hm_data
    std::vector<std::shared_ptr<BMNNTensor>> outputTensors(
        obj->mSubOutputBMtensors->tensors.size());
    for (int i = 0; i < obj->mSubOutputBMtensors->tensors.size(); i++) {
      outputTensors[i] = std::make_shared<BMNNTensor>(
          obj->mSubOutputBMtensors->handle,
          context->bmNetwork->m_netinfo->output_names[0],
          context->bmNetwork->m_netinfo->output_scales[0],
          obj->mSubOutputBMtensors->tensors[i][0].get(),
          context->bmNetwork->is_soc);
    }

    // boxes scores cropped_boxes
    std::vector<std::shared_ptr<common::DetectedObjectMetadata>> det_data =
        obj->mDetectedObjectMetadatas;

    getKeyPoints(outputTensors, det_data, *obj->mFrame->mSpData,
                 obj->mPosedObjectMetadatas, context->area_thresh,
                 context->heatmap_loss);
  }
}

void FastposePostProcess::getMaxPreds(
    float* ptr, int num_joints, int h, int w,
    std::shared_ptr<common::PosedObjectMetadata>& poseData) {
  const int joint_offset = h * w;
  for (int c = 0; c < num_joints; c++) {
    float* base = ptr + (c * joint_offset);
    int max_idx = 0;
    for (int i = 0; i < joint_offset; i++) {
      if ((*(base + i)) > (*(base + max_idx))) max_idx = i;
    }
    poseData->keypoints.push_back(max_idx % w);
    poseData->keypoints.push_back(int(max_idx / w));
    poseData->scores.push_back(*(base + max_idx));
    if (poseData->scores[c] == 0) poseData->scores[c] = 1e-5;
  }
}

void FastposePostProcess::getAffineTransform2d(float* center, float* scale,
                                               float* output_size, bool inv,
                                               cv::Mat* trans) {
  float src_dir[] = {0, scale[0] * (-0.5)};
  float dst_dir[] = {0, output_size[0] * (-0.5)};

  cv::Point2f src[3], dst[3];
  src[0] = cv::Point2f(center[0], center[1]);
  src[1] = cv::Point2f(center[0] + src_dir[0], center[1] + src_dir[1]);
  dst[0] = cv::Point2f(output_size[0] * 0.5, output_size[1] * 0.5);
  dst[1] = cv::Point2f(output_size[0] * 0.5 + dst_dir[0],
                       output_size[1] * 0.5 + dst_dir[1]);
  src[2] = cv::Point2f(center[0] + src_dir[0] + src_dir[1],
                       center[1] + src_dir[1] - src_dir[0]);
  dst[2] = cv::Point2f(output_size[0] * 0.5 + dst_dir[0] + dst_dir[1],
                       output_size[1] * 0.5 + dst_dir[1] - dst_dir[0]);

  if (inv)
    *trans = cv::getAffineTransform(dst, src);
  else
    *trans = cv::getAffineTransform(src, dst);
}

void FastposePostProcess::affineTransform2d(std::vector<float>& keypoints,
                                            int i, cv::Mat* trans) {
  float x = keypoints[i], y = keypoints[i + 1];
  keypoints[i] = trans->at<double>(0, 0) * x + trans->at<double>(0, 1) * y +
                 trans->at<double>(0, 2);
  keypoints[i + 1] = trans->at<double>(1, 0) * x + trans->at<double>(1, 1) * y +
                     trans->at<double>(1, 2);
}

void FastposePostProcess::heatmapToCoordSimple(
    float* ptr, common::Rectangle<int> bbox, int num_joints, int h, int w,
    std::shared_ptr<common::PosedObjectMetadata>& poseData) {
  getMaxPreds(ptr, num_joints, h, w, poseData);
  const int joint_offset = h * w;
  float bbox_scale[2] = {bbox.mWidth, bbox.mHeight};
  float bbox_center[2] = {bbox.mX + bbox_scale[0] * 0.5,
                          bbox.mY + bbox_scale[1] * 0.5};
  float output_size[2] = {w, h};
  cv::Mat trans;
  getAffineTransform2d(bbox_center, bbox_scale, output_size, 1, &trans);
  for (int c = 0; c < num_joints; c++) {
    float* base = ptr + c * joint_offset;
    int px = int(poseData->keypoints[c * 2]);
    int py = int(poseData->keypoints[c * 2 + 1]);
    if (1 < px && px < w - 1 && 1 < py && py < h - 1) {
      poseData->keypoints[c * 2] += 0.25 * sign((*(base + py * w + px + 1)) -
                                                (*(base + py * w + px - 1)));
      poseData->keypoints[c * 2 + 1] +=
          0.25 *
          sign((*(base + (py + 1) * w + px)) - (*(base + (py - 1) * w + px)));
    }
    affineTransform2d(poseData->keypoints, c * 2, &trans);
  }
}

void FastposePostProcess::poseNMS(
    std::vector<std::shared_ptr<common::DetectedObjectMetadata>>& det_data,
    int num_samples, int num_joints, float area_thresh,
    std::vector<std::shared_ptr<common::PosedObjectMetadata>>& body_keypoints,
    std::vector<int>& pick_ids) {
  if (num_joints == 136 || num_joints == 133)
    poseNMSFullBody(det_data, num_samples, num_joints, area_thresh,
                    body_keypoints, pick_ids);
  else
    poseNMSBody(det_data, num_samples, num_joints, area_thresh, body_keypoints,
                pick_ids);
}

void FastposePostProcess::poseNMSBody(
    std::vector<std::shared_ptr<common::DetectedObjectMetadata>>& det_data,
    int num_samples, int num_joints, float area_thresh,
    std::vector<std::shared_ptr<common::PosedObjectMetadata>>& body_keypoints,
    std::vector<int>& pick_ids) {
  float* ref_dists = new float[num_samples];
  float* human_scores = new float[num_samples];
  bool* mask = new bool[num_samples];
  int num_valid_samples = num_samples;
  for (int i = 0; i < num_samples; i++) {
    float width = det_data[i]->mBox.mWidth, height = det_data[i]->mBox.mHeight;
    ref_dists[i] = pose_nms_params->alpha * std::max(width, height);
    float joints_score_sum = 0;
    for (int j = 0; j < num_joints; j++) {
      joints_score_sum += body_keypoints[i]->scores[j];
    }
    human_scores[i] = joints_score_sum / num_joints;
    mask[i] = true;
  }

  while (num_valid_samples > 0) {
    int pick_id;
    for (int i = 0; i < num_samples; i++) {
      if (mask[i]) {
        pick_id = i;
        break;
      }
    }
    int relative_pick_id = 0;
    for (int i = pick_id + 1; i < num_samples; i++) {
      if (mask[i] && human_scores[i] > human_scores[pick_id]) {
        pick_id = i;
      }
    }

    // find overlap index
    std::vector<float> dist;
    std::vector<int> dist_indx;
    for (int i = 0; i < num_samples; i++) {
      if (i == pick_id) relative_pick_id = dist.size() / num_joints;
      if (mask[i]) {
        for (int j = 0; j < num_joints; j++) {
          dist.push_back(
              sqrt(pow(body_keypoints[i]->keypoints[j * 2] -
                           body_keypoints[pick_id]->keypoints[j * 2],
                       2) +
                   pow(body_keypoints[i]->keypoints[j * 2 + 1] -
                           body_keypoints[pick_id]->keypoints[j * 2 + 1],
                       2)));
          dist_indx.push_back(i * num_joints + j);
        }
      }
    }

    int keep_num = dist.size() / num_joints;
    float* final_dist = new float[keep_num];
    int* num_match_keypoints = new int[keep_num];
    getParametricDistance(body_keypoints, dist, dist_indx, pick_id, num_joints,
                          false, final_dist);
    PCKMatch(dist, num_joints, ref_dists[pick_id], num_match_keypoints);
    std::vector<int> delete_ids;
    for (int i = 0; i < keep_num; i++) {
      if (final_dist[i] > pose_nms_params->gamma ||
          num_match_keypoints[i] >= pose_nms_params->matchThreds) {
        int delete_id = dist_indx[i * num_joints] / num_joints;
        delete_ids.push_back(i);
        mask[delete_id] = false;
        num_valid_samples -= 1;
      }
    }
    delete[] final_dist;
    delete[] num_match_keypoints;
    if (delete_ids.size() == 0) {
      delete_ids.push_back(relative_pick_id);
      mask[pick_id] = false;
      num_valid_samples -= 1;
    }

    float pick_max_score = 0;
    for (int j = 0; j < num_joints; j++)
      if (body_keypoints[pick_id]->scores[j] > pick_max_score)
        pick_max_score = body_keypoints[pick_id]->scores[j];
    if (pick_max_score < pose_nms_params->scoreThreds) continue;
    pMergeFast(body_keypoints, delete_ids, dist, dist_indx, pick_id, num_joints,
               ref_dists[pick_id]);
    float merge_max_score = 0;
    for (int j = 0; j < num_joints; j++)
      if (body_keypoints[pick_id]->scores[j] > merge_max_score)
        merge_max_score = body_keypoints[pick_id]->scores[j];
    if (merge_max_score < pose_nms_params->scoreThreds) continue;
    float xmax = body_keypoints[pick_id]->keypoints[0];
    float xmin = xmax;
    float ymax = body_keypoints[pick_id]->keypoints[1];
    float ymin = ymax;
    for (int j = 1; j < num_joints; j++) {
      float cur_x = body_keypoints[pick_id]->keypoints[j * 2];
      float cur_y = body_keypoints[pick_id]->keypoints[j * 2 + 1];
      if (cur_x > xmax) xmax = cur_x;
      if (cur_x < xmin) xmin = cur_x;
      if (cur_y > ymax) ymax = cur_y;
      if (cur_y < ymin) ymin = cur_y;
    }
    if (1.5 * 1.5 * (xmax - xmin) * (ymax - ymin) < area_thresh) continue;
    pick_ids.push_back(pick_id);
  }

  delete[] ref_dists;
  delete[] human_scores;
  delete[] mask;
}

void FastposePostProcess::getParametricDistance(
    std::vector<std::shared_ptr<common::PosedObjectMetadata>>& body_keypoints,
    std::vector<float>& dist, std::vector<int>& dist_indx, int pick_id,
    int num_joints, bool use_dist_mask, float* final_dist) {
  for (int i = 0; i < dist.size(); i++) {
    int joint_idx = i % num_joints;
    if (joint_idx == 0) final_dist[i / num_joints] = 0;

    bool mask = dist[i] <= 1;
    bool dist_mask;
    if (use_dist_mask) {
      dist_mask = body_keypoints[dist_indx[i] / num_joints]
                      ->scores[dist_indx[i] % num_joints] <
                  pose_nms_params->scoreThreds;
      mask &= dist_mask;
    }

    float score_dist = 0;
    if (mask)
      score_dist = tanh(body_keypoints[pick_id]->scores[joint_idx] /
                        pose_nms_params->delta1) *
                   tanh(body_keypoints[dist_indx[i] / num_joints]
                            ->scores[dist_indx[i] % num_joints] /
                        pose_nms_params->delta1);

    if (use_dist_mask) {
      float point_dist;
      if (joint_idx < num_joints - 110) {
        point_dist = exp((-1) * dist[i] / pose_nms_params->delta2);
        final_dist[i / num_joints] +=
            ((dist_mask ? score_dist
                        : (score_dist + pose_nms_params->mu * point_dist)) /
             (num_joints - 110));
      } else if (joint_idx < num_joints - 42) {
        point_dist =
            exp((-1) * dist[i] /
                (pose_nms_params->delta2 * pose_nms_params->face_factor));
        final_dist[i / num_joints] +=
            ((dist_mask ? score_dist * pose_nms_params->face_weight_score
                        : (score_dist * pose_nms_params->face_weight_score +
                           pose_nms_params->mu * point_dist *
                               pose_nms_params->face_weight_dist)) /
             68);
      } else {
        point_dist =
            exp((-1) * dist[i] /
                (pose_nms_params->delta2 * pose_nms_params->hand_factor));
        final_dist[i / num_joints] +=
            ((dist_mask ? score_dist * pose_nms_params->hand_weight_score
                        : (score_dist * pose_nms_params->hand_weight_score +
                           pose_nms_params->mu * point_dist *
                               pose_nms_params->hand_weight_dist)) /
             42);
      }
    } else {
      float point_dist = exp((-1) * dist[i] / pose_nms_params->delta2);
      final_dist[i / num_joints] +=
          (score_dist + pose_nms_params->mu * point_dist);
    }
  }
}

void FastposePostProcess::PCKMatch(std::vector<float>& dist, int num_joints,
                                   float ref_dist, int* num_match_keypoints) {
  ref_dist = std::min(ref_dist, 7.f);
  for (int i = 0; i < dist.size(); i++) {
    int joint_idx = i % num_joints;
    if (joint_idx == 0) num_match_keypoints[i / num_joints] = 0;

    if (dist[i] / ref_dist <= 1) num_match_keypoints[i / num_joints] += 1;
  }
}

void FastposePostProcess::PCKMatchFullBody(
    std::vector<float>& dist,
    std::vector<std::shared_ptr<common::PosedObjectMetadata>>& body_keypoints,
    int pick_id, int num_joints, float ref_dist, int* num_match_keypoints) {
  int mask_num = 0;
  int valid_num = dist.size() / num_joints;
  for (int j = 0; j < num_joints; j++)
    if (body_keypoints[pick_id]->scores[j] > pose_nms_params->scoreThreds / 2)
      mask_num++;
  if (mask_num * 2 * valid_num < 2) {
    for (int i = 0; i < valid_num; i++) num_match_keypoints[i] = 0;
    return;
  }

  int add_num = 1 / mask_num / 2 * num_joints;
  ref_dist = std::min(ref_dist, 7.f);
  for (int i = 0; i < dist.size(); i++) {
    int joint_idx = i % num_joints;
    if (joint_idx == 0) num_match_keypoints[i / num_joints] = 0;

    if (joint_idx < 26 && dist[i] / ref_dist <= 1)
      num_match_keypoints[i / num_joints] += add_num;
    else if (26 <= joint_idx && joint_idx < 94 &&
             dist[i] / ref_dist <= pose_nms_params->face_factor)
      num_match_keypoints[i / num_joints] += add_num;
    else if (94 <= joint_idx &&
             dist[i] / ref_dist <= pose_nms_params->hand_factor)
      num_match_keypoints[i / num_joints] += add_num;
  }
}

void FastposePostProcess::pMergeFast(
    std::vector<std::shared_ptr<common::PosedObjectMetadata>>& body_keypoints,
    std::vector<int>& merge_ids, std::vector<float>& dist,
    std::vector<int>& dist_indx, int pick_id, int num_joints, float ref_dist) {
  ref_dist = std::min(ref_dist, 15.f);
  float* normed_scores = new float[merge_ids.size() * num_joints];
  float* sum_score = new float[num_joints];
  for (int j = 0; j < num_joints; j++) sum_score[j] = 0;
  for (int i = 0; i < merge_ids.size(); i++) {
    for (int j = 0; j < num_joints; j++) {
      if (dist[merge_ids[i] * num_joints + j] <= ref_dist) {
        int offset = dist_indx[merge_ids[i] * num_joints + j];
        normed_scores[i * num_joints + j] =
            body_keypoints[offset / num_joints]->scores[offset % num_joints];
        sum_score[j] += normed_scores[i * num_joints + j];
      } else
        normed_scores[i * num_joints + j] = 0;
    }
  }

  for (int i = 0; i < merge_ids.size(); i++) {
    for (int j = 0; j < num_joints; j++) {
      normed_scores[i * num_joints + j] /= sum_score[j];
    }
  }

  for (int j = 0; j < num_joints; j++) {
    float final_pose1 = 0, final_pose2 = 0, final_score = 0;
    for (int i = 0; i < merge_ids.size(); i++) {
      final_pose1 +=
          (body_keypoints[dist_indx[merge_ids[i] * num_joints + j] / num_joints]
               ->keypoints[j * 2] *
           normed_scores[i * num_joints + j]);
      final_pose2 +=
          (body_keypoints[dist_indx[merge_ids[i] * num_joints + j] / num_joints]
               ->keypoints[j * 2 + 1] *
           normed_scores[i * num_joints + j]);
      final_score += (pow(normed_scores[i * num_joints + j], 2) * sum_score[j]);
    }
    body_keypoints[pick_id]->keypoints[j * 2] = final_pose1;
    body_keypoints[pick_id]->keypoints[j * 2 + 1] = final_pose2;
    body_keypoints[pick_id]->scores[j] = final_score;
  }
  delete[] normed_scores;
  delete[] sum_score;
}

void FastposePostProcess::poseNMSFullBody(
    std::vector<std::shared_ptr<common::DetectedObjectMetadata>>& det_data,
    int num_samples, int num_joints, float area_thresh,
    std::vector<std::shared_ptr<common::PosedObjectMetadata>>& body_keypoints,
    std::vector<int>& pick_ids) {
  float* ref_dists = new float[num_samples];
  float* human_scores = new float[num_samples];
  bool* mask = new bool[num_samples];
  int num_valid_samples = num_samples;
  for (int i = 0; i < num_samples; i++) {
    float width = det_data[i]->mBox.mWidth, height = det_data[i]->mBox.mHeight;
    ref_dists[i] = pose_nms_params->alpha * std::max(width, height);
    float joints_score_sum = 0;
    for (int j = 0; j < num_joints; j++) {
      joints_score_sum += body_keypoints[i]->scores[j];
    }
    human_scores[i] = joints_score_sum / num_joints;
    mask[i] = true;
  }

  while (num_valid_samples > 0) {
    int pick_id;
    for (int i = 0; i < num_samples; i++) {
      if (mask[i]) {
        pick_id = i;
        break;
      }
    }
    int relative_pick_id = 0;
    for (int i = pick_id + 1; i < num_samples; i++) {
      if (mask[i] && human_scores[i] > human_scores[pick_id]) {
        pick_id = i;
      }
    }

    // find overlap index
    std::vector<float> dist;
    std::vector<int> dist_indx;
    for (int i = 0; i < num_samples; i++) {
      if (i == pick_id) relative_pick_id = dist.size() / num_joints;
      if (mask[i]) {
        for (int j = 0; j < num_joints; j++) {
          dist.push_back(
              sqrt(pow(body_keypoints[i]->keypoints[j * 2] -
                           body_keypoints[pick_id]->keypoints[j * 2],
                       2) +
                   pow(body_keypoints[i]->keypoints[j * 2 + 1] -
                           body_keypoints[pick_id]->keypoints[j * 2 + 1],
                       2.0)));
          dist_indx.push_back(i * num_joints + j);
        }
      }
    }

    int keep_num = dist.size() / num_joints;
    float* final_dist = new float[keep_num];
    int* num_match_keypoints = new int[keep_num];
    getParametricDistance(body_keypoints, dist, dist_indx, pick_id, num_joints,
                          true, final_dist);
    PCKMatchFullBody(dist, body_keypoints, pick_id, num_joints,
                     ref_dists[pick_id], num_match_keypoints);
    std::vector<int> delete_ids;
    for (int i = 0; i < keep_num; i++) {
      if (final_dist[i] > pose_nms_params->gamma ||
          num_match_keypoints[i] >= pose_nms_params->matchThreds) {
        int delete_id = dist_indx[i * num_joints] / num_joints;
        delete_ids.push_back(i);
        mask[delete_id] = false;
        num_valid_samples -= 1;
      }
    }
    delete[] final_dist;
    delete[] num_match_keypoints;
    if (delete_ids.size() == 0) {
      delete_ids.push_back(relative_pick_id);
      mask[pick_id] = false;
      num_valid_samples -= 1;
    }

    float pick_max_score = 0;
    for (int j = 0; j < num_joints; j++)
      if (body_keypoints[pick_id]->scores[j] > pick_max_score)
        pick_max_score = body_keypoints[pick_id]->scores[j];
    if (pick_max_score < pose_nms_params->scoreThreds) continue;
    pMergeFast(body_keypoints, delete_ids, dist, dist_indx, pick_id, num_joints,
               ref_dists[pick_id]);
    float merge_max_score = 0;
    for (int j = 0; j < num_joints; j++)
      if (body_keypoints[pick_id]->scores[j] > merge_max_score)
        merge_max_score = body_keypoints[pick_id]->scores[j];
    if (merge_max_score < pose_nms_params->scoreThreds) continue;
    float xmax = body_keypoints[pick_id]->keypoints[0];
    float xmin = xmax;
    float ymax = body_keypoints[pick_id]->keypoints[1];
    float ymin = ymax;
    for (int j = 1; j < num_joints; j++) {
      float cur_x = body_keypoints[pick_id]->keypoints[j * 2];
      float cur_y = body_keypoints[pick_id]->keypoints[j * 2 + 1];
      if (cur_x > xmax) xmax = cur_x;
      if (cur_x < xmin) xmin = cur_x;
      if (cur_y > ymax) ymax = cur_y;
      if (cur_y < ymin) ymin = cur_y;
    }
    if (1.5 * 1.5 * (xmax - xmin) * (ymax - ymin) < area_thresh) continue;
    pick_ids.push_back(pick_id);
  }

  delete[] ref_dists;
  delete[] human_scores;
  delete[] mask;
}

void FastposePostProcess::getKeyPoints(
    std::vector<std::shared_ptr<BMNNTensor>> outputTensors,
    std::vector<std::shared_ptr<common::DetectedObjectMetadata>>& det_data,
    const bm_image& image,
    std::vector<std::shared_ptr<common::PosedObjectMetadata>>& body_keypoints,
    float area_thresh, HeatmapLossType heatmap_loss) {
  int num_samples = det_data.size();
  int num_joints = outputTensors[0]->get_shape()->dims[1];

  std::vector<int> pick_ids;
  std::vector<std::shared_ptr<common::PosedObjectMetadata>> old_body_keypoints;
  for (int i = 0; i < num_samples; i++) {
    // TODO add other heatmapToCoord func
    std::shared_ptr<common::PosedObjectMetadata> poseData =
        std::make_shared<common::PosedObjectMetadata>();
    old_body_keypoints.push_back(poseData);
    if (heatmap_loss == HeatmapLossType::MSELoss)
      heatmapToCoordSimple((float*)outputTensors[i]->get_cpu_data(),
                           det_data[i]->mCroppedBox, num_joints,
                           outputTensors[i]->get_shape()->dims[2],
                           outputTensors[i]->get_shape()->dims[3], poseData);
    else {
      IVS_CRITICAL("heatmap_loss not implement");
      abort();
    }
  }
  poseNMS(det_data, num_samples, num_joints, area_thresh, old_body_keypoints,
          pick_ids);

  for (int i = 0; i < pick_ids.size(); i++) {
    body_keypoints.push_back(old_body_keypoints[pick_ids[i]]);
  }
}

}  // namespace fastpose
}  // namespace element
}  // namespace sophon_stream
