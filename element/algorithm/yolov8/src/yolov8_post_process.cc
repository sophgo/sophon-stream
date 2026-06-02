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
  if (context->taskType == TaskType::Detect) {
    if (context->use_post_opt)
      postProcessDetOpt(context, objectMetadatas);
    else
      postProcessDet(context, objectMetadatas);
  } else if (context->taskType == TaskType::Pose)
    postProcessPose(context, objectMetadatas);
  else if (context->taskType == TaskType::Cls)
    postProcessCls(context, objectMetadatas);
  else if (context->taskType == TaskType::Seg)
    postProcessSeg(context, objectMetadatas);
  else if (context->taskType == TaskType::SegFuse)
    postProcessSegFuse(context, objectMetadatas);
  else if (context->taskType == TaskType::Obb)
    postProcessObb(context, objectMetadatas);
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

void Yolov8PostProcess::postProcessDetOpt(
    std::shared_ptr<Yolov8Context> context,
    common::ObjectMetadatas& objectMetadatas) {
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
    int max_wh = 7680;
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
    auto out_tensor = outputTensors[min_idx];
    int mask_num = 0;
    int m_class_num = out_tensor->get_shape()->dims[2] - mask_num - 4;
    int feat_num = out_tensor->get_shape()->dims[1];
    int nout = m_class_num + mask_num + 4;
    float* output_data = nullptr;
    std::vector<float> decoded_data;

    assert(box_num == 0 || box_num == out_tensor->get_shape()->dims[2]);
    box_num = out_tensor->get_shape()->dims[2];
    output_data = (float*)out_tensor->get_cpu_data();

    // Candidates
    float* cls_conf = output_data + 4;
    for (int i = 0; i < feat_num; i++) {
      // best class
      float max_value = 0.0;
      int max_index = 0;
      for (int j = 0; j < m_class_num; j++) {
        float cur_value = cls_conf[i * nout + j];
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
        float centerX = output_data[i * nout];
        float centerY = output_data[i * nout + 1];
        float width = output_data[i * nout + 2];
        float height = output_data[i * nout + 3];

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

    for (int i = 0; i < yolobox_vec.size(); i++) {
      float centerx =
          ((yolobox_vec[i].x2 + yolobox_vec[i].x1) / 2 - tx1) / ratio;
      float centery =
          ((yolobox_vec[i].y2 + yolobox_vec[i].y1) / 2 - ty1) / ratio;
      float width = (yolobox_vec[i].x2 - yolobox_vec[i].x1) / ratio;
      float height = (yolobox_vec[i].y2 - yolobox_vec[i].y1) / ratio;

      std::shared_ptr<common::DetectedObjectMetadata> detData =
          std::make_shared<common::DetectedObjectMetadata>();
      detData->mBox.mX = std::max(int(centerx - width / 2), 0);
      detData->mBox.mY = std::max(int(centery - height / 2), 0);
      detData->mBox.mWidth = width;
      detData->mBox.mHeight = height;
      detData->mScores.push_back(yolobox_vec[i].score);
      detData->mClassify = yolobox_vec[i].class_id;

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
      IVS_WARN("WARNING: the current bmodel has redundant outputs");
      IVS_WARN("you can remove the redundant outputs to improve performance");
      IVS_WARN("");
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

    for (int i = 0; i < yolobox_vec.size(); i++) {
      float centerx =
          ((yolobox_vec[i].x2 + yolobox_vec[i].x1) / 2 - tx1) / ratio;
      float centery =
          ((yolobox_vec[i].y2 + yolobox_vec[i].y1) / 2 - ty1) / ratio;
      float width = (yolobox_vec[i].x2 - yolobox_vec[i].x1) / ratio;
      float height = (yolobox_vec[i].y2 - yolobox_vec[i].y1) / ratio;
      yolobox_vec[i].x1 = centerx - width / 2;
      yolobox_vec[i].y1 = centery - height / 2;
      yolobox_vec[i].x2 = centerx + width / 2;
      yolobox_vec[i].y2 = centery + height / 2;
    }

    clip_boxes(yolobox_vec, frame_width, frame_height);

    for (auto bbox : yolobox_vec) {
      std::shared_ptr<common::DetectedObjectMetadata> detData =
          std::make_shared<common::DetectedObjectMetadata>();
      detData->mBox.mX = std::max(int(bbox.x1), 0);
      detData->mBox.mY = std::max(int(bbox.y1), 0);
      detData->mBox.mWidth = bbox.x2 - bbox.x1;
      detData->mBox.mHeight = bbox.y2 - bbox.y1;
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

void Yolov8PostProcess::postProcessSeg(
    std::shared_ptr<Yolov8Context> context,
    common::ObjectMetadatas& objectMetadatas) {
  // YoloV8BoxVec
  YoloV8BoxVec yolobox_vec;
  int idx = 0;

  for (auto obj : objectMetadatas) {
    if (obj->mFrame->mEndOfStream) break;

    // get the outputs from obj->mOutputBMtensors
    std::vector<std::shared_ptr<BMNNTensor>> outputTensors(
        context->output_num);  // yolov8_seg has 2 outputs
    for (int i = 0; i < context->output_num; i++) {
      outputTensors[i] = std::make_shared<BMNNTensor>(
          obj->mOutputBMtensors->handle,
          context->bmNetwork->m_netinfo->output_names[i],
          context->bmNetwork->m_netinfo->output_scales[i],
          obj->mOutputBMtensors->tensors[i].get(), context->bmNetwork->is_soc);
    }

    yolobox_vec.clear();

    // the image or frame width and height
    int frame_width = obj->mFrame->mSpData->width;
    int frame_height = obj->mFrame->mSpData->height;
    int tx1 = 0, ty1 = 0;

    // USE_ASPECT_RATIO or Not
#if USE_ASPECT_RATIO

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

    ImageInfo para = {
        cv::Size(frame_width, frame_height),
        {ratio, ratio, static_cast<double>(tx1), static_cast<double>(ty1)}};

#else
    ImageInfo para = {cv::Size(frame_width, frame_height),
                      {context->net_w / (float)frame_width,
                       context->net_h / (float)frame_height, (double)tx1, (double)ty1}};

#endif
    int min_idx = 0;
    int box_num = 0;

    // mask info
    auto detection_out_tensor = outputTensors[min_idx];
    auto segmentation_out_tensor = outputTensors[1];

    auto detection_out_shape =
        context->bmNetwork->outputTensor(min_idx)->get_shape();
    auto segmentation_out_shape =
        context->bmNetwork->outputTensor(1)->get_shape();

    int mask_len = segmentation_out_shape->dims[1];
    int per_feat_size = detection_out_shape->dims[1];
    // Handle transposed output: [1, 8400, 116] vs [1, 116, 8400]
    int feat_num, m_class_num;
    bool transposed = false;
    if (detection_out_shape->dims[1] > detection_out_shape->dims[2]) {
      // Transposed format: [1, feat_num, per_feat_size]
      feat_num = detection_out_shape->dims[1];
      per_feat_size = detection_out_shape->dims[2];
      transposed = true;
    } else {
      // Original format: [1, per_feat_size, feat_num]
      feat_num = detection_out_shape->dims[2];
    }
    m_class_num = per_feat_size - mask_len - 4;  // 116 - 32 - 4 = 80

    int max_wh = 7680;  // (pixels) maximum box width and height
    int max_det = 300;

    // post1: get output one batch
    float* detection_data = nullptr;
    detection_data = (float*)detection_out_tensor->get_cpu_data();

    float* segmentation_data = nullptr;
    segmentation_data = segmentation_out_tensor->get_cpu_data();

    // post2:  get detections matrix nx6 (xyxy, score, class_id, mask)
    for (int i = 0; i < feat_num; i++) {
      // best class
      float max_value = 0.0;
      int max_index = 0;

      for (int j = 0; j < m_class_num; j++) {
        float cur_value;
        if (transposed) {
          cur_value = detection_data[i * per_feat_size + 4 + j];
        } else {
          cur_value = detection_data[i + (4 + j) * feat_num];
        }
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
        float centerX, centerY, width, height;
        if (transposed) {
          centerX = detection_data[i * per_feat_size + 0];
          centerY = detection_data[i * per_feat_size + 1];
          width = detection_data[i * per_feat_size + 2];
          height = detection_data[i * per_feat_size + 3];
        } else {
          centerX = detection_data[i + 0 * feat_num];
          centerY = detection_data[i + 1 * feat_num];
          width = detection_data[i + 2 * feat_num];
          height = detection_data[i + 3 * feat_num];
        }

        box.x1 = centerX - width / 2 + c;
        box.y1 = centerY - height / 2 + c;
        box.x2 = box.x1 + width;
        box.y2 = box.y1 + height;

        for (int k = 0; k < mask_len; k++) {
          if (transposed) {
            box.mask.emplace_back(
                detection_data[i * per_feat_size + (per_feat_size - mask_len + k)]);
          } else {
            box.mask.emplace_back(
                detection_data[i + (per_feat_size - mask_len + k) * feat_num]);
          }
        }

        yolobox_vec.push_back(box);
      }
    }

    // post3: nms
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

    // Scale bbox from model space to image space (matching sophon-demo)
    // para.trans = {ratio_x, ratio_y, tx1, ty1} where ratio = net_size / frame_size
    float inv_ratio_x = 1.0 / para.trans[0];  // frame_width / net_w
    float inv_ratio_y = 1.0 / para.trans[1];  // frame_height / net_h
    for (int i = 0; i < yolobox_vec.size(); i++) {
      yolobox_vec[i].x1 = std::round((yolobox_vec[i].x1 - tx1) * inv_ratio_x);
      yolobox_vec[i].y1 = std::round((yolobox_vec[i].y1 - ty1) * inv_ratio_y);
      yolobox_vec[i].x2 = std::round((yolobox_vec[i].x2 - tx1) * inv_ratio_x);
      yolobox_vec[i].y2 = std::round((yolobox_vec[i].y2 - ty1) * inv_ratio_y);
    }

    clip_boxes(yolobox_vec, frame_width, frame_height);

    // post 4: get mask (CPU)
    YoloV8BoxVec yolobox_vec_final;

    int dims = 4;
    int sizes[] = {
        segmentation_out_shape->dims[0], segmentation_out_shape->dims[1],
        segmentation_out_shape->dims[2], segmentation_out_shape->dims[3]};
    cv::Mat segmentation_out_data(dims, sizes, CV_32F, segmentation_data);

    for (int i = 0; i < yolobox_vec.size(); i++) {
      if (yolobox_vec[i].x2 > yolobox_vec[i].x1 + 1 &&
          yolobox_vec[i].y2 > yolobox_vec[i].y1 + 1) {
        get_mask(context, cv::Mat(yolobox_vec[i].mask).t(),
                 segmentation_out_data, para,
                 cv::Rect{yolobox_vec[i].x1, yolobox_vec[i].y1,
                          yolobox_vec[i].x2 - yolobox_vec[i].x1,
                          yolobox_vec[i].y2 - yolobox_vec[i].y1},
                 yolobox_vec[i].mask_img);

        yolobox_vec_final.emplace_back(yolobox_vec[i]);
      }
    }

    // 5. get final results
    for (auto bbox : yolobox_vec_final) {
      std::shared_ptr<common::SegmentedObjectMetadata> segData =
          std::make_shared<common::SegmentedObjectMetadata>();

      segData->mBox.mX = std::max(int(bbox.x1), 0);
      segData->mBox.mY = std::max(int(bbox.y1), 0);
      segData->mBox.mWidth = bbox.x2 - bbox.x1;
      segData->mBox.mHeight = bbox.y2 - bbox.y1;
      segData->mScores.push_back(bbox.score);
      segData->mClassify = bbox.class_id;
      segData->mask_img = bbox.mask_img;

      if (context->roi_predefined) {
        segData->mBox.mX += context->roi.start_x;
        segData->mBox.mY += context->roi.start_y;
      }

      if (context->class_thresh_valid) {
        segData->mLabelName = context->class_names[segData->mClassify];
      }

      obj->mSegmentedObjectMetadatas.push_back(segData);

      // Also populate mDetectedObjectMetadatas for OSD rendering
      std::shared_ptr<common::DetectedObjectMetadata> detData =
          std::make_shared<common::DetectedObjectMetadata>();
      detData->mBox.mX = segData->mBox.mX;
      detData->mBox.mY = segData->mBox.mY;
      detData->mBox.mWidth = segData->mBox.mWidth;
      detData->mBox.mHeight = segData->mBox.mHeight;
      detData->mScores.push_back(bbox.score);
      detData->mClassify = bbox.class_id;
      if (context->class_thresh_valid) {
        detData->mLabelName = context->class_names[detData->mClassify];
      }
      obj->mDetectedObjectMetadatas.push_back(detData);
    }

    ++idx;
  }
}

void Yolov8PostProcess::postProcessSegFuse(
    std::shared_ptr<Yolov8Context> context,
    common::ObjectMetadatas& objectMetadatas) {
  YoloV8BoxVec yolobox_vec;

  for (auto obj : objectMetadatas) {
    if (obj->mFrame->mEndOfStream) break;

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

    // Identify box output (2D) and mask output (3D)
    int box_tensor_idx = -1;
    int mask_tensor_idx = -1;
    for (int i = 0; i < context->output_num; i++) {
      auto shape = context->bmNetwork->outputTensor(i)->get_shape();
      if (shape->num_dims == 3) {
        mask_tensor_idx = i;
      } else if (shape->num_dims == 2) {
        box_tensor_idx = i;
      }
    }

    auto box_shape = context->bmNetwork->outputTensor(box_tensor_idx)->get_shape();
    auto mask_shape = context->bmNetwork->outputTensor(mask_tensor_idx)->get_shape();
    int box_num = box_shape->dims[0];
    int mask_h = mask_shape->dims[1];
    int mask_w = mask_shape->dims[2];

    // Get box data via BMNNTensor (float output)
    std::shared_ptr<BMNNTensor> box_out_tensor = std::make_shared<BMNNTensor>(
        obj->mOutputBMtensors->handle,
        context->bmNetwork->m_netinfo->output_names[box_tensor_idx],
        context->bmNetwork->m_netinfo->output_scales[box_tensor_idx],
        obj->mOutputBMtensors->tensors[box_tensor_idx].get(),
        context->bmNetwork->is_soc);
    float* box_data = (float*)box_out_tensor->get_cpu_data();

    // Get mask data directly from device memory (UINT8, not supported by BMNNTensor)
    auto& mask_tensor = obj->mOutputBMtensors->tensors[mask_tensor_idx];
    int mask_total = box_num * mask_h * mask_w;
    uint8_t* mask_data = new uint8_t[mask_total];
    bm_memcpy_d2s_partial(context->handle, mask_data,
                          mask_tensor->device_mem, mask_total);

    yolobox_vec.clear();

    // Parse boxes: [x1, y1, x2, y2, score, class_id]
    // Skip the last entry (sentinel) to match sophon-demo reference behavior
    for (int i = 0; i < box_num; i++) {
      YoloV8Box box;
      int box_offset = i * 6;
      box.x1 = box_data[box_offset];
      box.y1 = box_data[box_offset + 1];
      box.x2 = box_data[box_offset + 2];
      box.y2 = box_data[box_offset + 3];
      box.score = box_data[box_offset + 4];
      box.class_id = box_data[box_offset + 5];
      yolobox_vec.push_back(box);
    }

    // Scale bbox from model space to image space
    float inv_ratio = 1.0 / ratio;
    for (int i = 0; i < yolobox_vec.size(); i++) {
      yolobox_vec[i].x1 = std::round((yolobox_vec[i].x1 - tx1) * inv_ratio);
      yolobox_vec[i].y1 = std::round((yolobox_vec[i].y1 - ty1) * inv_ratio);
      yolobox_vec[i].x2 = std::round((yolobox_vec[i].x2 - tx1) * inv_ratio);
      yolobox_vec[i].y2 = std::round((yolobox_vec[i].y2 - ty1) * inv_ratio);
    }
    clip_boxes(yolobox_vec, frame_width, frame_height);

    // Process mask for each valid box
    ImageInfo para = {cv::Size(frame_width, frame_height),
                      {static_cast<double>(ratio), static_cast<double>(ratio),
                       static_cast<double>(tx1), static_cast<double>(ty1)}};

    YoloV8BoxVec yolobox_vec_tmp;
    for (int i = 0; i < yolobox_vec.size(); i++) {
      if (yolobox_vec[i].x2 > yolobox_vec[i].x1 + 1 &&
          yolobox_vec[i].y2 > yolobox_vec[i].y1 + 1) {
        // Extract per-box mask from the fused mask tensor
        cv::Mat mask_slice(mask_h, mask_w, CV_8UC1,
                          mask_data + i * mask_h * mask_w);
        // Process mask: crop valid region, resize, threshold, crop to bbox
        get_mask(context, mask_slice,
                 cv::Rect{yolobox_vec[i].x1, yolobox_vec[i].y1,
                          yolobox_vec[i].x2 - yolobox_vec[i].x1,
                          yolobox_vec[i].y2 - yolobox_vec[i].y1},
                 para, yolobox_vec[i].mask_img);
        yolobox_vec_tmp.push_back(yolobox_vec[i]);
      }
    }
    delete[] mask_data;

    // Populate results
    for (auto bbox : yolobox_vec_tmp) {
      std::shared_ptr<common::SegmentedObjectMetadata> segData =
          std::make_shared<common::SegmentedObjectMetadata>();

      segData->mBox.mX = std::max(int(bbox.x1), 0);
      segData->mBox.mY = std::max(int(bbox.y1), 0);
      segData->mBox.mWidth = bbox.x2 - bbox.x1;
      segData->mBox.mHeight = bbox.y2 - bbox.y1;
      segData->mScores.push_back(bbox.score);
      segData->mClassify = bbox.class_id;
      segData->mask_img = bbox.mask_img;

      if (context->roi_predefined) {
        segData->mBox.mX += context->roi.start_x;
        segData->mBox.mY += context->roi.start_y;
      }

      if (context->class_thresh_valid) {
        segData->mLabelName = context->class_names[segData->mClassify];
      }

      obj->mSegmentedObjectMetadatas.push_back(segData);

      std::shared_ptr<common::DetectedObjectMetadata> detData =
          std::make_shared<common::DetectedObjectMetadata>();
      detData->mBox.mX = segData->mBox.mX;
      detData->mBox.mY = segData->mBox.mY;
      detData->mBox.mWidth = segData->mBox.mWidth;
      detData->mBox.mHeight = segData->mBox.mHeight;
      detData->mScores.push_back(bbox.score);
      detData->mClassify = bbox.class_id;
      if (context->class_thresh_valid) {
        detData->mLabelName = context->class_names[detData->mClassify];
      }
      obj->mDetectedObjectMetadatas.push_back(detData);
    }
  }
}

void Yolov8PostProcess::get_mask(std::shared_ptr<Yolov8Context> context,
                                 const cv::Mat& mask_info,
                                 const cv::Mat& mask_data,
                                 const ImageInfo& para, cv::Rect bound,
                                 cv::Mat& mask_out) {
  // r r tx1 ty1
  cv::Vec4f trans = para.trans;

  int r_x = floor(trans[2] / context->net_w * (context->net_w / 4));
  int r_y = floor(trans[3] / context->net_h * (context->net_h / 4));

  int r_w = (context->net_w / 4) - 2 * r_x;
  int r_h = (context->net_h / 4) - 2 * r_y;

  r_w = MAX(r_w, 1);
  r_h = MAX(r_h, 1);

  std::vector<cv::Range> roi_rangs = {cv::Range(0, 1), cv::Range::all(),
                                      cv::Range(r_y, r_h + r_y),
                                      cv::Range(r_x, r_w + r_x)};

  cv::Mat temp_mask = mask_data(roi_rangs).clone();  // crop
  cv::Mat protos = temp_mask.reshape(0, {32, r_w * r_h});

  cv::Mat matmul_res = (mask_info * protos);
  cv::Mat masks_feature = matmul_res.reshape(1, {r_h, r_w});

  int left = bound.x;
  int top = bound.y;
  int width = bound.width;
  int height = bound.height;

  cv::Mat mask;
  resize(masks_feature, mask,
         cv::Size(para.raw_size.width, para.raw_size.height));
  mask_out = mask(bound) > 0.5f;
}

void Yolov8PostProcess::get_mask(std::shared_ptr<Yolov8Context> context,
                                 const cv::Mat& mask_slice, cv::Rect bound,
                                 const ImageInfo& para, cv::Mat& mask_out) {
  cv::Vec4f trans = para.trans;
  int r_x = floor(trans[2] / context->net_w * (context->net_w / 4));
  int r_y = floor(trans[3] / context->net_h * (context->net_h / 4));
  int r_w = (context->net_w / 4) - 2 * r_x;
  int r_h = (context->net_h / 4) - 2 * r_y;
  r_w = MAX(r_w, 1);
  r_h = MAX(r_h, 1);

  // Crop valid region from per-box mask and resize to full image
  cv::Mat valid_region = mask_slice(cv::Rect(r_x, r_y, r_w, r_h)).clone();
  cv::Mat resized_mask;
  cv::resize(valid_region, resized_mask,
             cv::Size(para.raw_size.width, para.raw_size.height));
  mask_out = resized_mask(bound) > 127;
}

void Yolov8PostProcess::postProcessObb(
    std::shared_ptr<Yolov8Context> context,
    common::ObjectMetadatas& objectMetadatas) {
  // Yolov8 obb vec
  obbBoxVec yolobox_vec;

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

    int frame_width = obj->mFrame->mSpData->width;
    int frame_height = obj->mFrame->mSpData->height;
    int tx1 = 0, ty1 = 0;
    float ratio = 1.0;
#ifdef USE_ASPECT_RATIO
    bool isAlignWidth = false;
    ratio =
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
    int m_class_num = out_tensor->get_shape()->dims[2] - mask_num - 5;
    int feature_num = out_tensor->get_shape()->dims[1];  // 8400
    int nout = m_class_num + mask_num + 5;
    int max_wh = 7680;  // (pixels) maximum box width and height
    bool agnostic = false;

    float* output_data = nullptr;
    std::vector<float> decoded_data;

    if (context->min_dim == 3 && context->output_num != 1) {
      IVS_WARN("WARNING: the current bmodel has redundant outputs");
      IVS_WARN("you can remove the redundant outputs to improve performance");
      IVS_WARN("");
    }

    assert(box_num == 0 || box_num == out_tensor->get_shape()->dims[1]);
    box_num = out_tensor->get_shape()->dims[1];
    output_data =
        (float*)out_tensor->get_cpu_data();  // 如果只有一张图片不要需修改

    // Candidates
    float* cls_conf = output_data + 4; //output_tensor's last dim: [x, y, w, h, cls_conf0, ..., cls_conf14, rotate_angle]
    for (int i = 0; i < box_num; i++) {
      // multilabel
      for (int j = 0; j < m_class_num; j++) {
        float cur_conf = cls_conf[i * nout + j];
        float cur_class_thresh =
          context->class_thresh_valid
              ? context->thresh_conf[context->class_names[j]]
              : context->thresh_conf_min;
        if (cur_conf > cur_class_thresh) {
          obbBox box;
          box.score = cur_conf;
          box.class_id = j;
          int c = agnostic ? 0 : box.class_id * max_wh;
          box.x = output_data[i * nout + 0] + c;
          box.y = output_data[i * nout + 1] + c;
          box.w = output_data[i * nout + 2];
          box.h = output_data[i * nout + 3];
          box.angle = output_data[(i + 1) * nout - 1];
          yolobox_vec.push_back(box);
        }
      }
    }
    nms_rotated(yolobox_vec, context->thresh_nms);
    if (yolobox_vec.size() > max_det) {
        yolobox_vec.erase(yolobox_vec.begin(), yolobox_vec.begin() + (yolobox_vec.size() - max_det));
    }
    if(!agnostic){
        for (int i = 0; i < yolobox_vec.size(); i++) {
            int c = yolobox_vec[i].class_id * max_wh;
            yolobox_vec[i].x = yolobox_vec[i].x - c;
            yolobox_vec[i].y = yolobox_vec[i].y - c;
        }
    }
    regularize_rbox(yolobox_vec);
    float inv_ratio = 1.0 / ratio;
    for (int i = 0; i < yolobox_vec.size(); i++) {
        yolobox_vec[i].x = std::round((yolobox_vec[i].x - tx1) * inv_ratio);
        yolobox_vec[i].y = std::round((yolobox_vec[i].y - ty1) * inv_ratio);
        yolobox_vec[i].w = std::round(yolobox_vec[i].w * inv_ratio);
        yolobox_vec[i].h = std::round(yolobox_vec[i].h * inv_ratio);
    }

    for (auto bbox : yolobox_vec) {
      std::shared_ptr<common::ObbObjectMetadata> obbData =
          std::make_shared<common::ObbObjectMetadata>(xywhr2xyxyxyxy(bbox));

      if (context->roi_predefined) {
        obbData->add_offset(context->roi.start_x, context->roi.start_y);
      }
      obj->mObbObjectMetadatas.push_back(obbData);
    }
    ++idx;
  }
}


void Yolov8PostProcess::regularize_rbox(obbBoxVec& obbVec){
  for(auto& obb : obbVec){
    if(obb.h > obb.w){
      std::swap(obb.w, obb.h);
      obb.angle = obb.angle + M_PI / 2;
    }
    obb.angle = std::fmod(obb.angle, M_PI);
    if(obb.angle < 0){
      obb.angle += M_PI;
    }
  }
}

std::tuple<float, float, float> Yolov8PostProcess::convariance_matrix(const obbBox& obb){
  float w = obb.w;
  float h = obb.h;
  float r = obb.angle;
  float a = w * w / 12.0;
  float b = h * h / 12.0;
  float cos_r = std::cos(r);
  float sin_r = std::sin(r);
  float a_val = a * cos_r * cos_r + b * sin_r * sin_r;
  float b_val = a * sin_r * sin_r + b * cos_r * cos_r;
  float c_val = (a - b) * cos_r * sin_r;
  return std::make_tuple(a_val, b_val, c_val);
}

float Yolov8PostProcess::probiou(const obbBox& obb1, const obbBox& obb2, float eps){
  // Calculate the prob iou between oriented bounding boxes, https://arxiv.org/pdf/2106.06072v1.pdf.
  float a1, b1, c1, a2, b2, c2;
  std::tie(a1, b1, c1) = convariance_matrix(obb1);
  std::tie(a2, b2, c2) = convariance_matrix(obb2);
  float x1 = obb1.x, y1 = obb1.y;
  float x2 = obb2.x, y2 = obb2.y;
  float t1 = ((a1 + a2) * std::pow(y1 - y2, 2) + (b1 + b2) * std::pow(x1 - x2, 2)) / ((a1 + a2) * (b1 + b2) - std::pow(c1 + c2, 2) + eps);
  float t2 = ((c1 + c2) * (x2 - x1) * (y1 - y2)) / ((a1 + a2) * (b1 + b2) - std::pow(c1 + c2, 2) + eps);
  float t3 = std::log(((a1 + a2) * (b1 + b2) - std::pow(c1 + c2, 2)) / (4 * std::sqrt(std::max(a1 * b1 - c1 * c1, 0.0f)) * std::sqrt(std::max(a2 * b2 - c2 * c2, 0.0f)) + eps) + eps);
  float bd = 0.25 * t1 + 0.5 * t2 + 0.5 * t3;
  bd = std::max(std::min(bd, 100.0f), eps);
  float hd = std::sqrt(1.0 - std::exp(-bd) + eps);
  return 1 - hd;
}


void Yolov8PostProcess::nms_rotated(obbBoxVec& dets, float nmsConfidence) {
    int length = dets.size();
    int index = length - 1;

    std::sort(dets.begin(), dets.end(), [](const obbBox& a, const obbBox& b) { return a.score < b.score; });

    while (index > 0) {
        int i = 0;
        while (i < index) {
            float iou = probiou(dets[index], dets[i]);
            if (iou >= nmsConfidence) {
                dets.erase(dets.begin() + i);
                index--;
            } else {
                i++;
            }
        }
        index--;
    }
}

common::ObbObjectMetadata Yolov8PostProcess::xywhr2xyxyxyxy(const obbBox& obb){
  common::ObbObjectMetadata obb_;
  float cos_value = std::cos(obb.angle);
  float sin_value = std::sin(obb.angle);

  // Calculate half-dimensions rotated
  float dx1 = (obb.w / 2) * cos_value;
  float dy1 = (obb.w / 2) * sin_value;
  float dx2 = (obb.h / 2) * sin_value;
  float dy2 = (obb.h / 2) * cos_value;

  // Calculate corners
  obb_.class_id = obb.class_id;
  obb_.score = obb.score;
  obb_.x1 = std::round(obb.x + dx1 + dx2);
  obb_.y1 = std::round(obb.y + dy1 - dy2);
  obb_.x2 = std::round(obb.x + dx1 - dx2);
  obb_.y2 = std::round(obb.y + dy1 + dy2);
  obb_.x3 = std::round(obb.x - dx1 - dx2);
  obb_.y3 = std::round(obb.y - dy1 + dy2);
  obb_.x4 = std::round(obb.x - dx1 + dx2);
  obb_.y4 = std::round(obb.y - dy1 - dy2);
  return obb_;
}

}  // namespace yolov8
}  // namespace element
}  // namespace sophon_stream