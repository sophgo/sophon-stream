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
    float ratio = 1;
    ImageInfo para = {cv::Size(frame_width, frame_height),
                      {context->net_w / frame_width,
                       context->net_h / frame_height, tx1, ty1}};

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
    int m_class_num =
        detection_out_shape->dims[1] - mask_len - 4;  // 116 - 32 - 4

    int feat_num = detection_out_shape->dims[2];       // 8400
    int per_feat_size = detection_out_shape->dims[1];  // 116

    int max_wh = 7680;  // (pixels) maximum box width and height
    int max_det = 300;

    // post1: get output one batch
    float* detection_data = nullptr;
    detection_data = (float*)detection_out_tensor->get_cpu_data();

    float* segmentation_data = nullptr;
    segmentation_data = segmentation_out_tensor->get_cpu_data();

    // post2:  get detections matrix nx6 (xyxy, score, class_id, mask)
    float* cls_conf = detection_data + 4 * feat_num;

    for (int i = 0; i < feat_num; i++) {
      // best class
      float max_value = 0.0;
      int max_index = 0;

      for (int j = 0; j < m_class_num; j++) {
        float cur_value = cls_conf[i + j * feat_num];
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
        float centerX = detection_data[i + 0 * feat_num];
        float centerY = detection_data[i + 1 * feat_num];
        float width = detection_data[i + 2 * feat_num];
        float height = detection_data[i + 3 * feat_num];

        box.x1 = centerX - width / 2 + c;
        box.y1 = centerY - height / 2 + c;
        box.x2 = box.x1 + width;
        box.y2 = box.y1 + height;

        for (int k = 0; k < mask_len; k++) {
          box.mask.emplace_back(
              detection_data[i + (per_feat_size - mask_len + k) * feat_num]);
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

    // post 4: get mask
    YoloV8BoxVec yolobox_vec_final;

    if (context->seg_tpu_opt) {
      cv::Vec4f trans = para.trans;
      int r_x =
          floor(trans[2] / context->net_w * segmentation_out_shape->dims[3]);
      int r_y =
          floor(trans[3] / context->net_h * segmentation_out_shape->dims[2]);

      int r_w = segmentation_out_shape->dims[3] - 2 * r_x;
      int r_h = segmentation_out_shape->dims[2] - 2 * r_y;

      r_w = MAX(r_w, 1);
      r_h = MAX(r_h, 1);

      struct Paras paras = {
          r_x, r_y, r_w, r_h, para.raw_size.width, para.raw_size.height};

      YoloV8BoxVec yolobox_valid_vec;
      for (int i = 0; i < yolobox_vec.size(); i++) {
        if (yolobox_vec[i].x2 > yolobox_vec[i].x1 + 1 &&
            yolobox_vec[i].y2 > yolobox_vec[i].y1 + 1) {
          yolobox_valid_vec.emplace_back(yolobox_vec[i]);
        }
      }

      bm_tensor_t segmentation_tensor;
      bool ok = bmrt_tensor(&segmentation_tensor, context->bmrt,
                            context->netinfo->input_dtypes[1],
                            context->netinfo->stages[0].input_shapes[1]);
      if (!ok) {
        IVS_ERROR("postProcessSeg bmrt_tensor error");
      }

      int ret = bm_memcpy_s2d_partial(
          context->tpu_mask_handle, segmentation_tensor.device_mem,
          reinterpret_cast<void*>(segmentation_data),
          bmrt_tensor_bytesize(&segmentation_tensor));
      if (ret != BM_SUCCESS) {
        IVS_ERROR("postProcessSeg bm_memcpy_s2d_partial error");
      }

      if (yolobox_valid_vec.size() > 0) {
        int mask_times =
            (yolobox_valid_vec.size() + context->tpu_mask_num - 1) /
            context->tpu_mask_num;

        for (int i = 0; i < mask_times; i++) {
          int start = i * context->tpu_mask_num;
          getmask_tpu(context, yolobox_valid_vec, start, segmentation_tensor,
                      paras, yolobox_vec_final, context->thresh_conf_min);
        }
      }

      bm_free_device(context->tpu_mask_handle, segmentation_tensor.device_mem);
    } else {
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
    }

    ++idx;
  }
}

void Yolov8PostProcess::getmask_tpu(std::shared_ptr<Yolov8Context> context,
                                    YoloV8BoxVec& yolov8box_input, int start,
                                    const bm_tensor_t& segmentation_tensor,
                                    Paras& paras,
                                    YoloV8BoxVec& yolov8box_output,
                                    float confThreshold) {
  int mask_height = context->m_tpumask_net_h;
  int mask_width = context->m_tpumask_net_w;
  int actual_mask_num =
      MIN(context->tpu_mask_num, yolov8box_input.size() - start);

  context->netinfo->stages[0].input_shapes[0].dims[0] = 1;
  context->netinfo->stages[0].input_shapes[0].dims[1] = actual_mask_num;
  context->netinfo->stages[0].input_shapes[0].dims[2] = context->mask_len;

  // 1. prepare bmodel inputs
  bm_tensor_t detection_tensor;
  bool ok = bmrt_tensor(&detection_tensor, context->bmrt,
                        context->netinfo->input_dtypes[0],
                        context->netinfo->stages[0].input_shapes[0]);
  if (!ok) {
    IVS_ERROR("getmask_tpu bmrt_tensor error");
  }

  for (size_t i = start; i < start + actual_mask_num; i++) {
    int ret = bm_memcpy_s2d_partial_offset(
        context->tpu_mask_handle, detection_tensor.device_mem,
        reinterpret_cast<void*>(yolov8box_input[i].mask.data()), 32 * 4,
        32 * 4 * (i - start));
    if (ret != BM_SUCCESS) {
      IVS_ERROR("getmask_tpu bm_memcpy_s2d_partial_offset error");
    }
  }

  std::vector<bm_tensor_t> input_tensors = {detection_tensor,
                                            segmentation_tensor};
  std::vector<bm_tensor_t> output_tensors;

  // 2. run bmodel
  output_tensors.resize(context->netinfo->output_num);
  ok = bmrt_launch_tensor(context->bmrt, context->netinfo->name,
                          input_tensors.data(), context->netinfo->input_num,
                          output_tensors.data(), context->netinfo->output_num);
  if (!ok) {
    IVS_ERROR("getmask_tpu bmrt_launch_tensor error");
  }

  int ret = bm_thread_sync(context->tpu_mask_handle);
  if (ret != BM_SUCCESS) {
    IVS_ERROR("getmask_tpu bm_thread_sync error");
  }

  bm_free_device(context->tpu_mask_handle, input_tensors[0].device_mem);

  // 3. get outputs
  bm_tensor_t output_tensor = output_tensors[0];
  float output0[1 * actual_mask_num * mask_height * mask_width];
  ret = bm_memcpy_d2s_partial(context->tpu_mask_handle, output0,
                              output_tensor.device_mem,
                              bmrt_tensor_bytesize(&output_tensor));
  if (ret != BM_SUCCESS) {
    IVS_ERROR("getmask_tpu bm_memcpy_d2s_partial error");
  }
  for (int i = 0; i < output_tensors.size(); i++) {
    bm_free_device(context->tpu_mask_handle, output_tensors[i].device_mem);
  }

  // 4. crop + mask
  for (int i = 0; i < actual_mask_num; i++) {
    int yi = start + i;
    cv::Mat temp_mask(mask_height, mask_width, CV_32FC1,
                      output0 + i * mask_height * mask_width);
    cv::Mat masks_feature =
        temp_mask(cv::Rect(paras.r_x, paras.r_y, paras.r_w, paras.r_h));
    cv::Mat mask;
    cv::resize(masks_feature, mask, cv::Size(paras.width, paras.height));

    // crop + mask
    cv::Rect bound = cv::Rect{yolov8box_input[yi].x1, yolov8box_input[yi].y1,
                              yolov8box_input[yi].x2 - yolov8box_input[yi].x1,
                              yolov8box_input[yi].y2 - yolov8box_input[yi].y1};
    yolov8box_input[yi].mask_img = mask(bound) > confThreshold;
    yolov8box_output.push_back(yolov8box_input[yi]);
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
  mask_out = mask(bound) > context->thresh_nms;
}

}  // namespace yolov8
}  // namespace element
}  // namespace sophon_stream