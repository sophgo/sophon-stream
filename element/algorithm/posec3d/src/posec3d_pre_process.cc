//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#include "posec3d_pre_process.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <fstream>

#include "common/logger.h"

namespace sophon_stream {
namespace element {
namespace posec3d {

void Posec3dPreProcess::init(std::shared_ptr<Posec3dContext> context) {}

common::ErrorCode Posec3dPreProcess::uniformSampleFrames(
    fpptr_dim3& sampled_keypoints, fpptr_dim3& sampled_keypoint_scores,
    int clip_len, int num_clips, int seed) {
  int num_frames = sampled_keypoints.size();
  std::vector<int> inds;
  srand(seed);

  // frame clip
  for (int i = 0; i < num_clips; i++) {
    if (num_frames < clip_len) {
      int start_indx = i;
      if (num_frames >= num_clips) start_indx = i * num_frames / num_clips;
      int end_indx = start_indx + clip_len;
      for (; start_indx < end_indx; start_indx++)
        inds.push_back(start_indx % num_frames);
    } else if (clip_len <= num_frames && num_frames < clip_len * 2) {
      std::vector<int> basic;
      for (int j = 0; j < clip_len + 1; j++) basic.push_back(j);
      std::random_shuffle(basic.begin(), basic.end());
      std::sort(basic.begin(), basic.begin() + num_frames - clip_len);
      int cur_offset = 0;
      for (int j = 0; j < clip_len; j++) {
        if (cur_offset < num_frames - clip_len && basic[cur_offset] == j) {
          cur_offset += 1;
          inds.push_back((j + cur_offset) % num_frames);
        } else
          inds.push_back((j + cur_offset) % num_frames);
      }
    } else {
      for (int j = 1; j <= clip_len; j++) {
        int first_num = (j - 1) * num_frames / clip_len;
        int second_num = j * num_frames / clip_len;
        inds.push_back(((rand() % (second_num - first_num)) + first_num) %
                       num_frames);
      }
    }
  }

  // filter frames
  for (int i = 0; i < inds.size(); i++) {
    sampled_keypoints.push_back(sampled_keypoints[inds[i]]);
    sampled_keypoint_scores.push_back(sampled_keypoint_scores[inds[i]]);
  }
  sampled_keypoints.erase(sampled_keypoints.begin(),
                          sampled_keypoints.begin() + num_frames);
  sampled_keypoint_scores.erase(sampled_keypoint_scores.begin(),
                                sampled_keypoint_scores.begin() + num_frames);

  return common::ErrorCode::SUCCESS;
}

common::ErrorCode Posec3dPreProcess::poseCompact(
    common::ObjectMetadatas& objectMetadatas, fpptr_dim3& keypoints,
    float padding, int threshold, std::vector<float>& hw_ratio,
    std::vector<int>& new_shape, std::vector<float>& crop_quadruple,
    bool allow_imgpad) {
  float min_x = INT_MAX, min_y = INT_MAX, max_x = INT_MIN, max_y = INT_MIN;
  // NOTE: assert frame1.h == frame2.h, frame1.w == frame2.w, ...
  int h = (*objectMetadatas[0]->mFrame->mSpData).height,
      w = (*objectMetadatas[0]->mFrame->mSpData).width;
  new_shape.clear();
  new_shape.push_back(h);
  new_shape.push_back(w);
  for (auto& obj : keypoints) {
    for (int j = 0; j < obj->size(); j++) {
      for (int i = 0; i < obj->at(j)->size(); i += 2) {
        if (obj->at(j)->at(i) < min_x) min_x = obj->at(j)->at(i);
        if (obj->at(j)->at(i + 1) < min_y) min_y = obj->at(j)->at(i + 1);
        if (obj->at(j)->at(i) > max_x) max_x = obj->at(j)->at(i);
        if (obj->at(j)->at(i + 1) > max_y) max_y = obj->at(j)->at(i + 1);
      }
    }
  }
  if (max_x - min_x < threshold || max_y - min_y < threshold)
    return common::ErrorCode::SUCCESS;

  float center_x = (max_x + min_x) / 2;
  float center_y = (max_y + min_y) / 2;
  float half_width = (max_x - min_x) / 2 * (1 + padding);
  float half_height = (max_y - min_y) / 2 * (1 + padding);

  if (hw_ratio.size() != 0) {
    half_height = std::max(hw_ratio[0] * half_width, half_height);
    half_width = std::max(1 / hw_ratio[1] * half_height, half_width);
  }

  min_x = center_x - half_width;
  max_x = center_x + half_width;
  min_y = center_y - half_height;
  max_y = center_y + half_height;

  // hot update
  if (!allow_imgpad) {
    min_x = int(std::max(0.f, min_x));
    min_y = int(std::max(0.f, min_y));
    max_x = int(std::min(float(w), max_x));
    max_y = int(std::min(float(h), max_y));
  } else {
    min_x = int(min_x);
    min_y = int(min_y);
    max_x = int(max_x);
    max_y = int(max_y);
  }

  for (auto& obj : keypoints) {
    for (int j = 0; j < obj->size(); j++) {
      for (int i = 0; i < obj->at(j)->size(); i += 2) {
        obj->at(j)->at(i) -= min_x;
        obj->at(j)->at(i + 1) -= min_y;
      }
    }
  }
  new_shape[0] = int(max_y - min_y);
  new_shape[1] = int(max_x - min_x);

  // the order is x, y, w, h (in [0, 1]), a tuple
  crop_quadruple.clear();
  crop_quadruple.push_back(min_x / w);
  crop_quadruple.push_back(min_y / h);
  crop_quadruple.push_back((max_x - min_x) / w);
  crop_quadruple.push_back((max_y - min_y) / h);

  return common::ErrorCode::SUCCESS;
}

common::ErrorCode Posec3dPreProcess::resize(fpptr_dim3& keypoints,
                                            std::vector<int>& scale,
                                            std::vector<int>& new_shape,
                                            bool keep_ratio) {
  int img_h = new_shape[0], img_w = new_shape[1];
  int new_w = scale[0], new_h = scale[1];
  if (keep_ratio) {
    int max_long_edge = std::max(scale[0], scale[1]);
    int max_short_edge = std::min(scale[0], scale[1]);
    float scale_factor =
        std::min(float(max_long_edge) / float(std::max(img_h, img_w)),
                 float(max_short_edge) / float(std::min(img_h, img_w)));
    new_w = int(img_w * scale_factor + 0.5);
    new_h = int(img_h * scale_factor + 0.5);
  }

  float scale_factor_x = float(new_w) / float(img_w);
  float scale_factor_y = float(new_h) / float(img_h);
  new_shape[0] = new_h;
  new_shape[1] = new_w;

  for (auto& obj : keypoints) {
    for (int j = 0; j < obj->size(); j++) {
      for (int i = 0; i < obj->at(j)->size(); i += 2) {
        obj->at(j)->at(i) *= scale_factor_x;
        obj->at(j)->at(i + 1) *= scale_factor_y;
      }
    }
  }
  return common::ErrorCode::SUCCESS;
}

common::ErrorCode Posec3dPreProcess::centerCrop(
    fpptr_dim3& keypoints, std::vector<int>& new_shape,
    std::vector<float>& crop_quadruple, std::vector<int>& crop_size) {
  int img_h = new_shape[0], img_w = new_shape[1];
  int crop_w = crop_size[0], crop_h = crop_size[1];
  int left = (img_w - crop_w) / 2;
  int top = (img_h - crop_h) / 2;
  int right = left + crop_w;
  int bottom = top + crop_h;
  int new_h = bottom - top, new_w = right - left;

  new_shape[0] = new_h;
  new_shape[1] = new_w;

  float x_ratio = float(left) / img_w, y_ratio = float(top) / img_h;
  float w_ratio = float(new_w) / img_w, h_ratio = float(new_h) / img_h;

  float old_x_ratio = crop_quadruple[0], old_y_ratio = crop_quadruple[1];
  float old_w_ratio = crop_quadruple[2], old_h_ratio = crop_quadruple[3];
  crop_quadruple[0] = old_x_ratio + x_ratio * old_w_ratio;
  crop_quadruple[1] = old_y_ratio + y_ratio * old_h_ratio;
  crop_quadruple[2] = w_ratio * old_w_ratio;
  crop_quadruple[3] = h_ratio * old_h_ratio;

  for (auto& obj : keypoints) {
    for (int j = 0; j < obj->size(); j++) {
      for (int i = 0; i < obj->at(j)->size(); i += 2) {
        obj->at(j)->at(i) -= left;
        obj->at(j)->at(i + 1) -= top;
      }
    }
  }
  return common::ErrorCode::SUCCESS;
}

common::ErrorCode Posec3dPreProcess::generatePoseTarget(
    std::shared_ptr<Posec3dContext> context, fpptr_dim3& keypoints,
    fpptr_dim3& sampled_keypoints, fpptr_dim3& sampled_keypoint_scores,
    std::vector<int>& new_shape, float* heatmap, int out_num, float sigma,
    float scaling, int clip_len) {
  // gen an aug
  const float eps = 1e-4;
  int img_h = new_shape[0], img_w = new_shape[1];
  // scale img_h, img_w and kps
  img_h = int(img_h * scaling + 0.5);
  img_w = int(img_w * scaling + 0.5);

  int num_c = context->m_net_keypoints;
  int num_frame = sampled_keypoints.size();
  for (auto& obj : keypoints) {
    for (int j = 0; j < obj->size(); j++) {
      for (int i = 0; i < obj->at(j)->size(); i += 2) {
        obj->at(j)->at(i) *= scaling;
        obj->at(j)->at(i + 1) *= scaling;
      }
    }
  }

  float* data = heatmap;
  memset((void*)data, 0, out_num * sizeof(float));
  int heatmap_start_indx = out_num / 2;
  for (int i = 0; i < num_frame; i++) {
    for (int j = 0; j < num_c; j++) {
      for (int person_id = 0; person_id < sampled_keypoints[i]->size();
           person_id++) {
        if (sampled_keypoint_scores[i]->at(person_id)->at(j) < eps) continue;

        float mu_x = sampled_keypoints[i]->at(person_id)->at(j * 2);
        float mu_y = sampled_keypoints[i]->at(person_id)->at(j * 2 + 1);

        int st_x = std::max(int(mu_x - 3 * sigma), 0);
        int ed_x = std::min(int(mu_x + 3 * sigma) + 1, img_w);
        int st_y = std::max(int(mu_y - 3 * sigma), 0);
        int ed_y = std::min(int(mu_y + 3 * sigma) + 1, img_h);
        if (st_x >= ed_x || st_y >= ed_y) continue;

        float* base = data + i / clip_len * num_c * clip_len * img_h * img_w +
                      j * clip_len * img_h * img_w +
                      i % clip_len * img_h * img_w;
        for (int patch_x = st_x; patch_x < ed_x; patch_x++)
          for (int patch_y = st_y; patch_y < ed_y; patch_y++) {
            float value = exp(-(std::pow(patch_x - mu_x, 2) +
                                std::pow(patch_y - mu_y, 2)) /
                              2 / std::pow(sigma, 2)) *
                          sampled_keypoint_scores[i]->at(person_id)->at(j);
            value *= context->input_scale;
            if (value > *(base + patch_y * img_w + patch_x)) {
              *(base + patch_y * img_w + patch_x) = value;
              *(base + patch_y * img_w + patch_x + heatmap_start_indx) = value;
              /*
              // original implement
              if (j == 0)
                *(base + heatmap_start_indx + patch_y * img_w + img_w - patch_x
              - 1) = value; else if (j % 2 == 0)
                *(data + heatmap_start_indx + i / clip_len * num_c * clip_len *
              img_h * img_w + (j - 1) * clip_len * img_h * img_w + i % clip_len
              * img_h * img_w + patch_y * img_w + img_w - patch_x - 1) = value;
              else if (j % 2 == 1)
                *(data + heatmap_start_indx + i / clip_len * num_c * clip_len *
              img_h * img_w + (j + 1) * clip_len * img_h * img_w + i % clip_len
              * img_h * img_w + patch_y * img_w + img_w - patch_x - 1) = value;
              */
            }
          }
      }
    }
  }

  return common::ErrorCode::SUCCESS;
}

void Posec3dPreProcess::initTensors(std::shared_ptr<Posec3dContext> context,
                                    common::ObjectMetadatas& objectMetadatas) {
  auto& obj = objectMetadatas[0];
  obj->mInputBMtensors = std::make_shared<sophon_stream::common::bmTensors>();
  int channelId = obj->mFrame->mChannelId;
  int frameId = obj->mFrame->mFrameId;
  obj->mInputBMtensors.reset(
      new sophon_stream::common::bmTensors(),
      [channelId, frameId, context](sophon_stream::common::bmTensors* p) {
        for (int i = 0; i < p->tensors.size(); ++i) {
          if (p->tensors[i]->device_mem.u.device.device_addr != 0) {
            if (context->bmNetwork->is_soc) {
              int tensor_size =
                  bm_mem_get_device_size(p->tensors[i]->device_mem);
              bm_status_t ret = bm_mem_unmap_device_mem(
                  p->handle, p->cpu_data[i], tensor_size);
              assert(BM_SUCCESS == ret);
            }
            bm_free_device(p->handle, p->tensors[i]->device_mem);
          }
        }

        delete p;
        p = nullptr;
      });
  obj->mInputBMtensors->handle = context->handle;
  obj->mInputBMtensors->tensors.resize(context->input_num);
  for (int i = 0; i < context->input_num; ++i) {
    obj->mInputBMtensors->tensors[i] = std::make_shared<bm_tensor_t>();
    obj->mInputBMtensors->tensors[i]->dtype =
        context->bmNetwork->m_netinfo->input_dtypes[i];
    obj->mInputBMtensors->tensors[i]->shape =
        context->bmNetwork->m_netinfo->stages[0].input_shapes[i];
    obj->mInputBMtensors->tensors[i]->st_mode = BM_STORE_1N;
  }
}

common::ErrorCode Posec3dPreProcess::preProcess(
    std::shared_ptr<Posec3dContext> context,
    common::ObjectMetadatas& objectMetadatas) {
  if (objectMetadatas.size() == 0) return common::ErrorCode::SUCCESS;
  initTensors(context, objectMetadatas);

  fpptr_dim3 keypoints;
  fpptr_dim3 keypoint_scores;
  fpptr_dim3 sampled_keypoints;
  fpptr_dim3 sampled_keypoint_scores;
  for (auto& obj : objectMetadatas) {
    std::shared_ptr<fpptr_dim2> single_person_keypoints =
        std::make_shared<fpptr_dim2>();
    std::shared_ptr<fpptr_dim2> single_person_keypoint_scores =
        std::make_shared<fpptr_dim2>();
    for (auto& poseObj : obj->mPosedObjectMetadatas) {
      single_person_keypoints->push_back(
          std::make_shared<std::vector<float>>(poseObj->keypoints));
      single_person_keypoint_scores->push_back(
          std::make_shared<std::vector<float>>(poseObj->scores));
    }
    keypoints.push_back(single_person_keypoints);
    keypoint_scores.push_back(single_person_keypoint_scores);
  }
  sampled_keypoints = keypoints;
  sampled_keypoint_scores = keypoint_scores;
  int clip_len = 48, num_clips = 10;

  // upsample frames to get 480 objs
  uniformSampleFrames(sampled_keypoints, sampled_keypoint_scores, clip_len,
                      num_clips, 255);
  std::vector<float> hw_ratio = {1.0, 1.0};
  std::vector<int> new_shape;
  std::vector<float> crop_quadruple = {0, 0, 1, 1};

  // rescale anf shift keypoints
  poseCompact(objectMetadatas, keypoints, 0.25, 10, hw_ratio, new_shape,
              crop_quadruple, true);
  std::vector<int> scale = {INT_MAX, 64};
  resize(keypoints, scale, new_shape, true);
  std::vector<int> crop_size = {64, 64};
  centerCrop(keypoints, new_shape, crop_quadruple, crop_size);
  int out_num = context->m_net_crops_clips * context->m_net_channel *
                context->m_net_keypoints * context->m_net_h * context->m_net_w;
  int size_byte = out_num * sizeof(float);
  // input data set into obj0 mem
  auto ret = bm_malloc_device_byte_heap(
      context->handle,
      &objectMetadatas[0]->mInputBMtensors->tensors[0]->device_mem, 0,
      size_byte);
  STREAM_CHECK(ret == 0, "Alloc Device Memory Failed! Program Terminated.")

  // generate heatmap input
  float* heatmap = nullptr;
  if (context->bmNetwork->is_soc) {
    unsigned long long addr;
    assert(BM_SUCCESS ==
           bm_mem_mmap_device_mem(
               context->handle,
               &objectMetadatas[0]->mInputBMtensors->tensors[0]->device_mem,
               &addr));
    objectMetadatas[0]->mInputBMtensors->cpu_data.resize(1);
    objectMetadatas[0]->mInputBMtensors->cpu_data[0] = (float*)addr;
    heatmap = objectMetadatas[0]->mInputBMtensors->cpu_data[0];
  } else
    heatmap = new float[out_num];
  generatePoseTarget(context, keypoints, sampled_keypoints,
                     sampled_keypoint_scores, new_shape, heatmap, out_num, 0.6,
                     1.0, clip_len);

  if (context->bmNetwork->is_soc)
    assert(BM_SUCCESS ==
           bm_mem_flush_device_mem(
               context->handle,
               &objectMetadatas[0]->mInputBMtensors->tensors[0]->device_mem));
  else {
    assert(BM_SUCCESS ==
           bm_memcpy_s2d(
               context->handle,
               objectMetadatas[0]->mInputBMtensors->tensors[0]->device_mem,
               (void*)heatmap));
    delete[] heatmap;
  }

  objectMetadatas[0]->is_main = true;

  return common::ErrorCode::SUCCESS;
}

}  // namespace posec3d
}  // namespace element
}  // namespace sophon_stream