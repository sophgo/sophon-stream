//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
// 参考sophon-demo中的sampls的yolov8 cpp后处理编写
//
//===----------------------------------------------------------------------===//

#include "yolov8_post_process.h"

#include <cmath>

#include "common/logger.h"

namespace sophon_stream {
namespace element {
namespace yolov8 {

void Yolov8PostProcess::init(std::shared_ptr<Yolov8Context> context) {}

Yolov8PostProcess::~Yolov8PostProcess() {}

int Yolov8PostProcess::argmax(float* data, int num) {
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

float Yolov8PostProcess::sigmoid(float x) { return 1.0 / (1 + expf(-x)); }

float Yolov8PostProcess::get_aspect_scaled_ratio(int src_w, int src_h,
                                                 int dst_w, int dst_h,
                                                 bool* pIsAligWidth) {
  float ratio;
  float r_w = (float)dst_w / src_w;
  float r_h = (float)dst_h / src_h;
  if (r_h > r_w) {
    *pIsAligWidth = true;
    ratio = r_w;
  } else {
    *pIsAligWidth = false;
    ratio = r_h;
  }
  return ratio;
}

void Yolov8PostProcess::NMS(YoloV8BoxVec& dets, float nmsConfidence) {
  int length = dets.size();
  int index = length - 1;

  std::sort(
      dets.begin(), dets.end(),
      [](const YoloV8Box& a, const YoloV8Box& b) { return a.score < b.score; });

  std::vector<float> areas(length);
  for (int i = 0; i < length; i++) {
    float width = dets[i].x2 - dets[i].x1;
    float height = dets[i].y2 - dets[i].y1;
    areas[i] = width * height;
  }

  while (index > 0) {
    int i = 0;
    while (i < index) {
      float left = std::max(dets[index].x1, dets[i].x1);
      float top = std::max(dets[index].y1, dets[i].y1);
      float right = std::min(dets[index].x2, dets[i].x2);
      float bottom = std::min(dets[index].y2, dets[i].y2);
      float overlap =
          std::max(0.0f, right - left) * std::max(0.0f, bottom - top);
      if (overlap / (areas[index] + areas[i] - overlap) > nmsConfidence) {
        areas.erase(areas.begin() + i);
        dets.erase(dets.begin() + i);
        index--;
      } else {
        i++;
      }
    }
    index--;
  }
}

void Yolov8PostProcess::postProcess(std::shared_ptr<Yolov8Context> context,
                                    common::ObjectMetadatas& objectMetadatas,
                                    int dataPipeId) {
  if (objectMetadatas.size() == 0) return;
  if (context->taskType == TaskType::Detect)
    postProcessDet(context, objectMetadatas);
  else if (context->taskType == TaskType::Pose)
    postProcessPose(context, objectMetadatas);
  else if (context->taskType == TaskType::Cls)
    postProcessCls(context, objectMetadatas);
}

void Yolov8PostProcess::clip_boxes(YoloV8BoxVec& yolobox_vec, int src_w,
                                   int src_h) {
  for (int i = 0; i < yolobox_vec.size(); i++) {
    if (yolobox_vec[i].x1 < 0)
      yolobox_vec[i].x1 = 0;
    else if (yolobox_vec[i].x1 > src_w)
      yolobox_vec[i].x1 = src_w;
    if (yolobox_vec[i].y1 < 0)
      yolobox_vec[i].y1 = 0;
    else if (yolobox_vec[i].y1 > src_h)
      yolobox_vec[i].y1 = src_h;
    if (yolobox_vec[i].x2 < 0)
      yolobox_vec[i].x2 = 0;
    else if (yolobox_vec[i].x2 > src_w)
      yolobox_vec[i].x2 = src_w;
    if (yolobox_vec[i].y2 < 0)
      yolobox_vec[i].y2 = 0;
    else if (yolobox_vec[i].y2 > src_h)
      yolobox_vec[i].y2 = src_h;
  }
}

void Yolov8PostProcess::postProcessCls(
    std::shared_ptr<Yolov8Context> context,
    common::ObjectMetadatas& objectMetadatas) {
  int idx = 0;
  for (auto obj : objectMetadatas) {
    if (obj->mFrame->mEndOfStream) break;
    std::vector<std::shared_ptr<BMNNTensor>> outputTensors(context->output_num);
    for (int i = 0; i < context->output_num; i++) {
      outputTensors[i] = std::make_shared<BMNNTensor>(
          obj->mOutputBMtensors->handle,
          context->bmNetwork->m_netinfo->output_names[i],
          context->bmNetwork->m_netinfo->output_scales[i],
          obj->mOutputBMtensors->tensors[i].get(), context->bmNetwork->is_soc);
    }
    auto out_tensor = outputTensors[0];
    int feature_num = out_tensor->get_shape()->dims[1];  // 1000

    float* output_data = nullptr;
    std::vector<float> decoded_data;

    output_data = (float*)out_tensor->get_cpu_data();

    // argmax
    float max_score = 0;
    int max_idx = 0;
    for (int i = 0; i < feature_num; ++i) {
      if (output_data[i] > max_score) {
        max_score = output_data[i];
        max_idx = i;
      }
    }

    IVS_INFO("ChannelId = {0}, FrameId = {1}, Class = {2}, Score = {3}",
             obj->mFrame->mChannelId, obj->mFrame->mFrameId, max_idx,
             max_score);

    auto clsData = std::make_shared<common::RecognizedObjectMetadata>();
    clsData->mScores.push_back(max_score);
    clsData->mTopKLabels.push_back(max_idx);
    obj->mRecognizedObjectMetadatas.push_back(clsData);
    ++idx;
  }
  return;
}

void Yolov8PostProcess::postProcessPose(
    std::shared_ptr<Yolov8Context> context,
    common::ObjectMetadatas& objectMetadatas) {
  YoloV8BoxVec yolobox_vec;
  // single class patch
  constexpr int PATCH = 10000;

  int idx = 0;
  for (auto obj : objectMetadatas) {
    if (obj->mFrame->mEndOfStream) break;
    std::vector<std::shared_ptr<BMNNTensor>> outputTensors(context->output_num);
    for (int i = 0; i < context->output_num; i++) {
      outputTensors[i] = std::make_shared<BMNNTensor>(
          obj->mOutputBMtensors->handle,
          context->bmNetwork->m_netinfo->output_names[i],
          context->bmNetwork->m_netinfo->output_scales[i],
          obj->mOutputBMtensors->tensors[i].get(), context->bmNetwork->is_soc);
    }

    yolobox_vec.clear();
    int frame_width = obj->mFrame->mSpData->width;
    int frame_height = obj->mFrame->mSpData->height;
#ifdef USE_ASPECT_RATIO
    bool isAlignWidth = false;
    float ratio =
        context->roi_predefined
            ? get_aspect_scaled_ratio(context->roi.crop_w, context->roi.crop_h,
                                      context->net_w, context->net_h,
                                      &isAlignWidth)
            : get_aspect_scaled_ratio(frame_width, frame_height, context->net_w,
                                      context->net_h, &isAlignWidth);
#endif
    int dw = context->roi_predefined
                 ? (context->net_w - (context->roi.crop_w * ratio))
                 : (context->net_w - (frame_width * ratio));
    int dh = context->roi_predefined
                 ? (context->net_h - (context->roi.crop_h * ratio))
                 : (context->net_h - (frame_height * ratio));
    dw /= 2;
    dh /= 2;

    int min_idx = 0;
    for (int i = 0; i < context->output_num; ++i) {
      auto output_shape = context->bmNetwork->outputTensor(i)->get_shape();
      auto output_dims = output_shape->num_dims;

      if (context->min_dim > output_dims) {
        min_idx = i;
        context->min_dim = output_dims;
      }
    }

    for (int tensor_idx = 0; tensor_idx < context->output_num; ++tensor_idx) {
      auto out_tensor = outputTensors[tensor_idx];
      int feature_num = out_tensor->get_shape()->dims[2];  // 8400
      int num_channels = out_tensor->get_shape()->dims[1];

      float* output_data = nullptr;
      std::vector<float> decoded_data;

      output_data = (float*)out_tensor->get_cpu_data();

      for (int i = 0; i < feature_num; i++) {
        float max_value = output_data[i + 4 * feature_num];

        float cur_class_thresh =
            context->class_thresh_valid
                ? context->thresh_conf[context->class_names[0]]
                : context->thresh_conf_min;

        if (max_value > cur_class_thresh) {
          YoloV8Box box;
          box.score = max_value;
          box.class_id = 0;
          float centerX = output_data[i + 0 * feature_num] - dw;
          float centerY = output_data[i + 1 * feature_num] - dh;
          float width = output_data[i + 2 * feature_num];
          float height = output_data[i + 3 * feature_num];

          box.x1 = (centerX - 0.5 * width) / ratio + PATCH;
          box.y1 = (centerY - 0.5 * height) / ratio + PATCH;
          box.x2 = (centerX + 0.5 * width) / ratio + PATCH;
          box.y2 = (centerY + 0.5 * height) / ratio + PATCH;

          for (int k = 0; k < 17; ++k) {
            float kps_x =
                (output_data[i + (5 + 3 * k) * feature_num] - dw) / ratio;
            float kps_y =
                (output_data[i + (5 + 3 * k + 1) * feature_num] - dh) / ratio;
            float kps_s = output_data[i + (5 + 3 * k + 2) * feature_num];
            box.kps.push_back(kps_x);
            box.kps.push_back(kps_y);
            box.kps.push_back(kps_s);
          }
          yolobox_vec.push_back(box);
        }
      }
    }

    NMS(yolobox_vec, context->thresh_nms);
    if (yolobox_vec.size() > max_det) {
      yolobox_vec.erase(yolobox_vec.begin(),
                        yolobox_vec.begin() + (yolobox_vec.size() - max_det));
    }

    for (auto bbox : yolobox_vec) {
      std::shared_ptr<common::DetectedObjectMetadata> detData =
          std::make_shared<common::DetectedObjectMetadata>();

      detData->mBox.mX = bbox.x1 - PATCH;
      detData->mBox.mY = bbox.y1 - PATCH;
      detData->mBox.mWidth = bbox.x2 - bbox.x1;
      detData->mBox.mHeight = bbox.y2 - bbox.y1;
      detData->mScores.push_back(bbox.score);
      detData->mClassify = bbox.class_id;

      std::shared_ptr<common::PosedObjectMetadata> poseData =
          std::make_shared<common::PosedObjectMetadata>();
      poseData->keypoints = bbox.kps;

      if (context->roi_predefined) {
        detData->mBox.mX += context->roi.start_x;
        detData->mBox.mY += context->roi.start_y;
      }

      if (context->class_thresh_valid) {
        detData->mLabelName = context->class_names[detData->mClassify];
      }
      obj->mDetectedObjectMetadatas.push_back(detData);
      obj->mPosedObjectMetadatas.push_back(poseData);
    }
    ++idx;
  }
}

void Yolov8PostProcess::postProcessDet(
    std::shared_ptr<Yolov8Context> context,
    common::ObjectMetadatas& objectMetadatas) {
  // Yolov8 vec
  YoloV8BoxVec yolobox_vec;

  int idx = 0;
  for (auto obj : objectMetadatas) {
    if (obj->mFrame->mEndOfStream) break;
    std::vector<std::shared_ptr<BMNNTensor>> outputTensors(context->output_num);
    for (int i = 0; i < context->output_num; i++) {
      outputTensors[i] = std::make_shared<BMNNTensor>(
          obj->mOutputBMtensors->handle,
          context->bmNetwork->m_netinfo->output_names[i],
          context->bmNetwork->m_netinfo->output_scales[i],
          obj->mOutputBMtensors->tensors[i].get(), context->bmNetwork->is_soc);
    }

    yolobox_vec.clear();
    int frame_width = obj->mFrame->mSpData->width;
    int frame_height = obj->mFrame->mSpData->height;
    int tx1 = 0, ty1 = 0;
#ifdef USE_ASPECT_RATIO
    bool isAlignWidth = false;
    float ratio =
        context->roi_predefined
            ? get_aspect_scaled_ratio(context->roi.crop_w, context->roi.crop_h,
                                      context->net_w, context->net_h,
                                      &isAlignWidth)
            : get_aspect_scaled_ratio(frame_width, frame_height, context->net_w,
                                      context->net_h, &isAlignWidth);
    if (isAlignWidth) {
      ty1 = (int)((context->net_h -
                   (int)((context->roi_predefined ? context->roi.crop_h
                                                  : frame_height) *
                         ratio)) /
                  2);
    } else {
      tx1 = (int)((context->net_w -
                   (int)((context->roi_predefined ? context->roi.crop_w
                                                  : frame_width) *
                         ratio)) /
                  2);
    }
#endif
    int min_idx = 0;
    int box_num = 0;
    for (int i = 0; i < context->output_num; ++i) {
      auto output_shape = context->bmNetwork->outputTensor(i)->get_shape();
      auto output_dims = output_shape->num_dims;
      assert(output_dims == 3 || output_dims == 5);
      if (output_dims == 5) {
        box_num += output_shape->dims[1] * output_shape->dims[2] *
                   output_shape->dims[3];
      }

      if (context->min_dim > output_dims) {
        min_idx = i;
        context->min_dim = output_dims;
      }
    }
    // mask info
    int mask_num = 0;
    auto out_tensor = outputTensors[min_idx];
    int m_class_num = out_tensor->get_shape()->dims[1] - mask_num - 4;
    int feature_num = out_tensor->get_shape()->dims[2];  // 8400
    int nout = m_class_num + mask_num + 4;
    int max_wh = 7680;  // (pixels) maximum box width and height

    float* output_data = nullptr;
    std::vector<float> decoded_data;

    if (context->min_dim == 3 && context->output_num != 1) {
      std::cout << "--> WARNING: the current bmodel has redundant outputs"
                << std::endl;
      std::cout << "             you can remove the redundant outputs to "
                   "improve performance"
                << std::endl;
      std::cout << std::endl;
    }

    assert(box_num == 0 || box_num == out_tensor->get_shape()->dims[1]);
    box_num = out_tensor->get_shape()->dims[1];
    output_data =
        (float*)out_tensor->get_cpu_data();  // 如果只有一张图片不要需修改
    float* cls_conf = output_data + 4 * feature_num;
    for (int i = 0; i < feature_num; i++) {
      // best class
      float max_value = 0.0;
      int max_index = 0;
      for (int j = 0; j < m_class_num; j++) {
        float cur_value = cls_conf[i + j * feature_num];
        if (cur_value > max_value) {
          max_value = cur_value;
          max_index = j;
        }
      }

      float cur_class_thresh =
          context->class_thresh_valid
              ? context->thresh_conf[context->class_names[max_index]]
              : context->thresh_conf_min;

      if (max_value >= cur_class_thresh) {
        YoloV8Box box;
        box.score = max_value;
        box.class_id = max_index;
        int c = box.class_id * max_wh;
        float centerX = output_data[i + 0 * feature_num];
        float centerY = output_data[i + 1 * feature_num];
        float width = (output_data[i + 2 * feature_num]);
        float height = (output_data[i + 3 * feature_num]);

        box.x1 = centerX - width / 2 + c;
        box.y1 = centerY - height / 2 + c;
        box.x2 = box.x1 + width;
        box.y2 = box.y1 + height;

        yolobox_vec.push_back(box);
      }
    }

    NMS(yolobox_vec, context->thresh_nms);
    if (yolobox_vec.size() > max_det) {
      yolobox_vec.erase(yolobox_vec.begin(),
                        yolobox_vec.begin() + (yolobox_vec.size() - max_det));
    }

    for (int i = 0; i < yolobox_vec.size(); i++) {
      int c = yolobox_vec[i].class_id * max_wh;
      yolobox_vec[i].x1 = yolobox_vec[i].x1 - c;
      yolobox_vec[i].y1 = yolobox_vec[i].y1 - c;
      yolobox_vec[i].x2 = yolobox_vec[i].x2 - c;
      yolobox_vec[i].y2 = yolobox_vec[i].y2 - c;
    }

    clip_boxes(yolobox_vec, frame_width, frame_height);

    for (auto bbox : yolobox_vec) {
      float centerx = ((bbox.x2 + bbox.x1) / 2 - tx1) / ratio;
      float centery = ((bbox.y2 + bbox.y1) / 2 - ty1) / ratio;
      float width = (bbox.x2 - bbox.x1) / ratio;
      float height = (bbox.y2 - bbox.y1) / ratio;

      std::shared_ptr<common::DetectedObjectMetadata> detData =
          std::make_shared<common::DetectedObjectMetadata>();
      detData->mBox.mX = std::max(int(centerx - width / 2), 0);
      detData->mBox.mY = std::max(int(centery - height / 2), 0);
      detData->mBox.mWidth = width;
      detData->mBox.mHeight = height;
      detData->mScores.push_back(bbox.score);
      detData->mClassify = bbox.class_id;

      if (context->roi_predefined) {
        detData->mBox.mX += context->roi.start_x;
        detData->mBox.mY += context->roi.start_y;
      }

      // check the range of box
      if (detData->mBox.mX + detData->mBox.mWidth >=
          obj->mFrame->mSpData->width) {
        detData->mBox.mWidth =
            (obj->mFrame->mSpData->width - 1 - detData->mBox.mX);
      }
      if (detData->mBox.mY + detData->mBox.mHeight >=
          obj->mFrame->mSpData->height) {
        detData->mBox.mHeight =
            (obj->mFrame->mSpData->height - 1 - detData->mBox.mY);
      }

      if (context->class_thresh_valid) {
        detData->mLabelName = context->class_names[detData->mClassify];
      }
      obj->mDetectedObjectMetadatas.push_back(detData);
    }
    ++idx;
  }
}

}  // namespace yolov8
}  // namespace element
}  // namespace sophon_stream