//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#include "yolox_post_process.h"

#include <algorithm>
#include <cmath>
#include "common/logger.h"

namespace sophon_stream {
namespace element {
namespace yolox {

void YoloxPostProcess::init(std::shared_ptr<YoloxContext> context) {

  m_box_num = 0;
  int net_w = context->net_w;
  int net_h = context->net_h;
  std::vector<int> strides{8, 16, 32};
  for (int i = 0; i < strides.size(); ++i) {
    int layer_w = net_w / strides[i];
    int layer_h = net_h / strides[i];
    m_box_num += layer_w * layer_h;
  }
  m_grids_x = new int[m_box_num];
  m_grids_y = new int[m_box_num];
  m_expanded_strides = new int[m_box_num];

  int channel_len = 0;
  for (int i = 0; i < strides.size(); ++i) {
    int layer_w = net_w / strides[i];
    int layer_h = net_h / strides[i];
    for (int m = 0; m < layer_h; ++m) {
      for (int n = 0; n < layer_w; ++n) {
        m_grids_x[channel_len + m * layer_w + n] = n;
        m_grids_y[channel_len + m * layer_w + n] = m;
        m_expanded_strides[channel_len + m * layer_w + n] = strides[i];
      }
    }
    channel_len += layer_w * layer_h;
  }
}

int YoloxPostProcess::argmax(float* data, int num) {
  float max_value = 0.0;
  int max_index = 0;
  for (int i = 0; i < num; ++i) {
    float value = data[i];
    if (value > max_value) {
      max_value = value;
      max_index = i;
    }
  }
  return max_index;
}

float YoloxPostProcess::sigmoid(float x) { return 1.0 / (1 + expf(-x)); }

float YoloxPostProcess::box_iou(const YoloxBox& a, const YoloxBox& b) {
  float x = std::min(a.right, b.right) - std::max(a.left, b.left);
  float y = std::min(a.bottom, b.bottom) - std::max(a.top, b.top);
  float w = x > 0 ? x : 0;
  float h = y > 0 ? y : 0;
  float i = w * h;
  float u = a.width * a.height + b.width * b.height - i;
  return i / (u);
}

void YoloxPostProcess::nms_sorted_bboxes(const std::vector<YoloxBox>& objects,
                              std::vector<int>& picked, float nms_threshold) {
  picked.clear();
  const int n = objects.size();

  for (int i = 0; i < n; i++) {
    const YoloxBox& a = objects[i];
    int keep = 1;
    for (int j = 0; j < (int)picked.size(); j++) {
      const YoloxBox& b = objects[picked[j]];

      float iou = box_iou(a, b);
      if (iou > nms_threshold) keep = 0;
    }
    if (keep) picked.push_back(i);
  }
}

YoloxPostProcess::~YoloxPostProcess() {
  delete[] m_grids_x;
  delete[] m_grids_y;
  delete[] m_expanded_strides;
}

void YoloxPostProcess::postProcess3output(std::shared_ptr<YoloxContext> context,
                                   common::ObjectMetadatas& objectMetadatas) {
  if (objectMetadatas.size() == 0) return;
  if (objectMetadatas[0]->mFrame->mEndOfStream) return;
  std::vector<std::shared_ptr<BMNNTensor>> outputTensors(context->output_num);
  for (int i = 0; i < context->output_num; i++) {
    outputTensors[i] = std::make_shared<BMNNTensor>(
        objectMetadatas[0]->mOutputBMtensors->handle,
        context->bmNetwork->m_netinfo->output_names[i],
        context->bmNetwork->m_netinfo->output_scales[i],
        objectMetadatas[0]->mOutputBMtensors->tensors[i].get(),
        context->bmNetwork->is_soc);
  }

  int frame_width = objectMetadatas[0]->mFrame->mWidth;
  int frame_height = objectMetadatas[0]->mFrame->mHeight;
  int net_w = context->net_w;
  int net_h = context->net_h;

  for (int batch_idx = 0; batch_idx < context->max_batch; ++batch_idx) {
    int tx1 = 0, ty1 = 0;

    float scale_w = float(net_w) / frame_width;
    float scale_h = float(net_h) / frame_height;
    float scale_min = scale_h < scale_w ? scale_h : scale_w;

    float scale_x = 1.0 / scale_min;
    float scale_y = 1.0 / scale_min;

    YoloxBoxVec yolobox_vec;
    float* objectTensor = nullptr;
    float* boxTensor = nullptr;
    float* classTensor = nullptr;
    int boxOffset = 0;
    int classOffset = 0;
    int objectOffset = 0;
    float* tensor = nullptr;
    int numDim3 = 0;
    int batchOffset = 0;
    if (context->output_num==3) {
      objectOffset = batch_idx * m_box_num * 1;
      boxOffset = batch_idx * m_box_num * 4;
      classOffset = batch_idx * m_box_num * context->class_num;
      objectTensor = (float*)outputTensors[0]->get_cpu_data();
      boxTensor = (float*)outputTensors[2]->get_cpu_data();
      classTensor = (float*)outputTensors[1]->get_cpu_data();

    } else {
      tensor = (float*)outputTensors[0]->get_cpu_data();
      numDim3 = context->class_num + 5;
      batchOffset = batch_idx * m_box_num * numDim3;
    }

    for (size_t i = 0; i < m_box_num; ++i) {
      // 取出物体置信度
      float box_objectness = 0;
      if (context->output_num==3)
        box_objectness = objectTensor[objectOffset + i];
      else
        box_objectness = tensor[batchOffset + i * numDim3 + 4];

      if (box_objectness < context->thresh_conf) continue;
      // 进入解码阶段
      float center_x = 0;
      float center_y = 0;
      float w_temp = 0;
      float h_temp = 0;
      if (context->output_num==3) {
        center_x = (boxTensor[boxOffset + i * 4 + 0] + m_grids_x[i]) *
                   m_expanded_strides[i];
        center_y = (boxTensor[boxOffset + i * 4 + 1] + m_grids_y[i]) *
                   m_expanded_strides[i];
        w_temp = exp(boxTensor[boxOffset + i * 4 + 2]) * m_expanded_strides[i];
        h_temp = exp(boxTensor[boxOffset + i * 4 + 3]) * m_expanded_strides[i];
      } else {
        center_x = (tensor[batchOffset + i * numDim3 + 0] + m_grids_x[i]) *
                   m_expanded_strides[i];
        center_y = (tensor[batchOffset + i * numDim3 + 1] + m_grids_y[i]) *
                   m_expanded_strides[i];
        w_temp =
            exp(tensor[batchOffset + i * numDim3 + 2]) * m_expanded_strides[i];
        h_temp =
            exp(tensor[batchOffset + i * numDim3 + 3]) * m_expanded_strides[i];
      }

      center_x *= scale_x;
      center_y *= scale_y;
      w_temp *= scale_x;
      h_temp *= scale_y;
      float left = center_x - w_temp / 2;
      float top = center_y - h_temp / 2;
      float right = center_x + w_temp / 2;
      float bottom = center_y + h_temp / 2;

      for (int class_idx = 0; class_idx < context->class_num; ++class_idx) {
        float box_cls_score = 0;
        if (context->output_num==3)
          box_cls_score =
              classTensor[classOffset + i * context->class_num + class_idx];
        else
          box_cls_score = tensor[batchOffset + i * numDim3 + 5 + class_idx];
        float box_prob = box_objectness * box_cls_score;
        if (box_prob > context->thresh_conf) {
          YoloxBox box;
          box.width = w_temp;
          box.height = h_temp;
          box.left = left;
          box.top = top;
          box.right = right;
          box.bottom = bottom;
          box.score = box_prob;
          box.class_id = class_idx;
          yolobox_vec.push_back(box);
        }
      }
    }

    // nms
    std::sort(
        yolobox_vec.begin(), yolobox_vec.end(),
        [](const YoloxBox& a, const YoloxBox& b) { return a.score > b.score; });
    std::vector<int> picked;
    nms_sorted_bboxes(yolobox_vec, picked, context->thresh_nms);

    for (size_t i = 0; i < picked.size(); i++) {
      auto bbox = yolobox_vec[picked[i]];
      std::shared_ptr<common::ObjectMetadata> spObjData =
          std::make_shared<common::ObjectMetadata>();
      spObjData->mDetectedObjectMetadata =
          std::make_shared<common::DetectedObjectMetadata>();
      spObjData->mDetectedObjectMetadata->mBox.mX = bbox.left;
      spObjData->mDetectedObjectMetadata->mBox.mY = bbox.top;
      spObjData->mDetectedObjectMetadata->mBox.mWidth = bbox.width;
      spObjData->mDetectedObjectMetadata->mBox.mHeight = bbox.height;
      spObjData->mDetectedObjectMetadata->mScores.push_back(bbox.score);
      spObjData->mDetectedObjectMetadata->mClassify = bbox.class_id;
      objectMetadatas[batch_idx]->mSubObjectMetadatas.push_back(spObjData);
    }
  }
                                   }

void YoloxPostProcess::postProcess1output(std::shared_ptr<YoloxContext> context,
                                   common::ObjectMetadatas& objectMetadatas) {
  if (objectMetadatas.size() == 0) return;
  if (objectMetadatas[0]->mFrame->mEndOfStream) return;
  std::vector<std::shared_ptr<BMNNTensor>> outputTensors(context->output_num);
  for (int i = 0; i < context->output_num; i++) {
    outputTensors[i] = std::make_shared<BMNNTensor>(
        objectMetadatas[0]->mOutputBMtensors->handle,
        context->bmNetwork->m_netinfo->output_names[i],
        context->bmNetwork->m_netinfo->output_scales[i],
        objectMetadatas[0]->mOutputBMtensors->tensors[i].get(),
        context->bmNetwork->is_soc);
  }

  int frame_width = objectMetadatas[0]->mFrame->mWidth;
  int frame_height = objectMetadatas[0]->mFrame->mHeight;
  int net_w = context->net_w;
  int net_h = context->net_h;

  float scale_w = float(net_w) / frame_width;
  float scale_h = float(net_h) / frame_height;
  float scale = 1.0 / (scale_h < scale_w ? scale_h : scale_w);

  float* tensor = (float*)outputTensors[0]->get_cpu_data();
  for (int batch_idx = 0; batch_idx < context->max_batch; ++batch_idx) {


    YoloxBoxVec yolobox_vec;

    int numDim3 = context->class_num + 5;
    int batchOffset = batch_idx * m_box_num * numDim3;

    for (size_t i = 0; i < m_box_num; ++i) {
      // 取出物体置信度
      float box_objectness = tensor[batchOffset + i * numDim3 + 4];
      if (box_objectness < context->thresh_conf) continue;
      int max_class_idx = argmax(&tensor[batchOffset + i * numDim3 + 5], context->class_num);
      float box_prob = box_objectness * tensor[batchOffset+i*numDim3+5+max_class_idx];
      if(box_prob > context->thresh_conf) {
        float center_x = (tensor[batchOffset + i * numDim3 + 0] + m_grids_x[i]) *
                     m_expanded_strides[i];
          float center_y = (tensor[batchOffset + i * numDim3 + 1] + m_grids_y[i]) *
                     m_expanded_strides[i];
          float w_temp =
              exp(tensor[batchOffset + i * numDim3 + 2]) * m_expanded_strides[i];
          float h_temp =
              exp(tensor[batchOffset + i * numDim3 + 3]) * m_expanded_strides[i];

          center_x *= scale;
          center_y *= scale;
          w_temp *= scale;
          h_temp *= scale;
          float left = center_x - w_temp / 2;
          float top = center_y - h_temp / 2;
          float right = center_x + w_temp / 2;
          float bottom = center_y + h_temp / 2;

          YoloxBox box;
          box.width = w_temp;
          box.height = h_temp;
          box.left = left;
          box.top = top;
          box.right = right;
          box.bottom = bottom;
          box.score = box_prob;
          box.class_id = max_class_idx;
          yolobox_vec.push_back(box);
      }
    }

    // nms
    std::sort(
        yolobox_vec.begin(), yolobox_vec.end(),
        [](const YoloxBox& a, const YoloxBox& b) { return a.score > b.score; });
    std::vector<int> picked;
    nms_sorted_bboxes(yolobox_vec, picked, context->thresh_nms);

    for (size_t i = 0; i < picked.size(); i++) {
      auto bbox = yolobox_vec[picked[i]];
      std::shared_ptr<common::ObjectMetadata> spObjData =
          std::make_shared<common::ObjectMetadata>();
      spObjData->mDetectedObjectMetadata =
          std::make_shared<common::DetectedObjectMetadata>();
      spObjData->mDetectedObjectMetadata->mBox.mX = bbox.left;
      spObjData->mDetectedObjectMetadata->mBox.mY = bbox.top;
      spObjData->mDetectedObjectMetadata->mBox.mWidth = bbox.width;
      spObjData->mDetectedObjectMetadata->mBox.mHeight = bbox.height;
      spObjData->mDetectedObjectMetadata->mScores.push_back(bbox.score);
      spObjData->mDetectedObjectMetadata->mClassify = bbox.class_id;
      objectMetadatas[batch_idx]->mSubObjectMetadatas.push_back(spObjData);
    }
  }
                                   }

void YoloxPostProcess::postProcess(std::shared_ptr<YoloxContext> context,
                                   common::ObjectMetadatas& objectMetadatas) {
    if (context->output_num==3) postProcess3output(context, objectMetadatas);
    else postProcess1output(context, objectMetadatas);
}

}  // namespace yolox
}  // namespace element
}  // namespace sophon_stream