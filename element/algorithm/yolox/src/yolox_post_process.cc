//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#include "yolox_post_process.h"

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
                                         std::vector<int>& picked,
                                         float nms_threshold) {
  picked.clear();
  const int n = objects.size();
  int suppressed[n];
  memset(suppressed, 0, sizeof(float) * n);

  for (int i = 0; i < n; i++) {
    if (suppressed[i] == 1) continue;
    picked.push_back(i);
    const YoloxBox& a = objects[i];
    for (int j = i + 1; j < n; j++) {
      if (suppressed[j] == 1) continue;
      const YoloxBox& b = objects[j];
      float iou = box_iou(a, b);
      if (iou > nms_threshold) suppressed[j] = 1;
    }
  }
}

YoloxPostProcess::~YoloxPostProcess() {
  delete[] m_grids_x;
  delete[] m_grids_y;
  delete[] m_expanded_strides;
}

void YoloxPostProcess::postProcess(std::shared_ptr<YoloxContext> context,
                                   common::ObjectMetadatas& objectMetadatas) {
  if (objectMetadatas.size() == 0) return;

  for (auto& obj : objectMetadatas) {
    if (obj->mFrame->mEndOfStream) break;
    int frame_width = obj->mFrame->mWidth;
    int frame_height = obj->mFrame->mHeight;
    int net_w = context->net_w;
    int net_h = context->net_h;

    float scale_w = float(net_w) / (context->roi_predefined ? context->roi.crop_w
                                                           : frame_width);
    float scale_h = float(net_h) / (context->roi_predefined ? context->roi.crop_h
                                                           : frame_height);
    float scale = 1.0 / (scale_h < scale_w ? scale_h : scale_w);

    std::vector<std::shared_ptr<BMNNTensor>> outputTensors(context->output_num);
    for (int i = 0; i < context->output_num; i++) {
      outputTensors[i] = std::make_shared<BMNNTensor>(
          obj->mOutputBMtensors->handle,
          context->bmNetwork->m_netinfo->output_names[i],
          context->bmNetwork->m_netinfo->output_scales[i],
          obj->mOutputBMtensors->tensors[i].get(), context->bmNetwork->is_soc);
    }
    float* tensor = (float*)outputTensors[0]->get_cpu_data();
    YoloxBoxVec yolobox_vec;
    int numDim3 = context->class_num + 5;

    for (size_t i = 0; i < m_box_num; ++i) {
      // 取出物体置信度
      float box_objectness = tensor[i * numDim3 + 4];
      if (box_objectness < context->thresh_conf_min) continue;
      int max_class_idx = argmax(&tensor[i * numDim3 + 5], context->class_num);
      float box_prob = box_objectness * tensor[i * numDim3 + 5 + max_class_idx];
      float cur_class_thresh =
          context->class_thresh_valid
              ? context->thresh_conf[context->class_names[max_class_idx]]
              : context->thresh_conf_min;
      if (box_prob > cur_class_thresh) {
        float center_x =
            (tensor[i * numDim3 + 0] + m_grids_x[i]) * m_expanded_strides[i];
        float center_y =
            (tensor[i * numDim3 + 1] + m_grids_y[i]) * m_expanded_strides[i];
        float w_temp = exp(tensor[i * numDim3 + 2]) * m_expanded_strides[i];
        float h_temp = exp(tensor[i * numDim3 + 3]) * m_expanded_strides[i];

        center_x *= scale;
        center_y *= scale;
        w_temp *= scale;
        h_temp *= scale;
        float left = center_x - w_temp / 2;
        float top = center_y - h_temp / 2;
        float right = center_x + w_temp / 2;
        float bottom = center_y + h_temp / 2;

        YoloxBox box;
        // 检查一下取值范围
        if (w_temp < 0 || h_temp < 0 || w_temp >= frame_width ||
            h_temp > frame_height)
          continue;
        box.left = (left >= 0) ? left : 0;
        box.top = (top >= 0) ? top : 0;
        box.right = (right < frame_width) ? right : (frame_width - 1);
        box.bottom = (bottom < frame_height) ? bottom : (frame_height - 1);
        box.width = box.right - box.left;
        box.height = box.bottom - box.top;
        if (box.width < 0 || box.height < 0) continue;
        box.score = box_prob;
        box.class_id = max_class_idx;
        if (w_temp * h_temp > m_min_box_area) yolobox_vec.push_back(box);
      }
    }

    std::sort(
        yolobox_vec.begin(), yolobox_vec.end(),
        [](const YoloxBox& a, const YoloxBox& b) { return a.score > b.score; });
    std::vector<int> picked;
    nms_sorted_bboxes(yolobox_vec, picked, context->thresh_nms);

    for (size_t i = 0; i < picked.size(); i++) {
      auto bbox = yolobox_vec[picked[i]];
      std::shared_ptr<common::DetectedObjectMetadata> detData =
          std::make_shared<common::DetectedObjectMetadata>();
      detData->mBox.mX = bbox.left;
      detData->mBox.mY = bbox.top;
      detData->mBox.mWidth = bbox.width;
      detData->mBox.mHeight = bbox.height;
      detData->mScores.push_back(bbox.score);
      detData->mClassify = bbox.class_id;
      if (context->roi_predefined) {
        detData->mBox.mX += context->roi.start_x;
        detData->mBox.mY += context->roi.start_y;
      }
      if (context->class_thresh_valid) {
        detData->mLabelName = context->class_names[detData->mClassify];
      }
      obj->mDetectedObjectMetadatas.push_back(detData);
    }
  }
}

}  // namespace yolox
}  // namespace element
}  // namespace sophon_stream