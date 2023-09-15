//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#include "retinaface_post_process.h"
#include <opencv2/opencv.hpp>
#include <cmath>
#include "common/logger.h"

#include <iostream>


namespace sophon_stream {
namespace element {
namespace retinaface {

void RetinafacePostProcess::init(std::shared_ptr<RetinafaceContext> context) {}

float RetinafacePostProcess::get_aspect_scaled_ratio(int src_w, int src_h, int dst_w,
                                                int dst_h, bool* pIsAligWidth) {
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

void RetinafacePostProcess::postProcess(std::shared_ptr<RetinafaceContext> context,
                                    common::ObjectMetadatas& objectMetadatas) {
  if (objectMetadatas.size() == 0) return;
  // write your post process here
  int i=0;
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

    int frame_width = obj->mFrame->mWidth;
    int frame_height = obj->mFrame->mHeight;
    int output_num=context->output_num;


    std::vector<void*> output_data;
    for (int i = 0; i < output_num; i++) {
      // Push the result of get_cpu_data() directly into the outputs_ vector
      output_data.push_back(outputTensors[i]->get_cpu_data());
    }

    //计算output_size
    vector<int>  output_sizes;
    map<string, int> output_names_map;
    for (int i = 0; i < output_num; i++) {
      output_names_map.insert(pair<string, int>(
                          context->bmNetwork->m_netinfo->output_names[i], i));
      auto &output_shape = context->bmNetwork->m_netinfo->stages[0].output_shapes[i];
      auto count = bmrt_shape_count(&output_shape);
      output_sizes.push_back(count / output_shape.dims[0]);
    }

    //将输出数据转换为float型数据
    float *preds[output_num];
    for (int j = 0; j < output_num; j++) {
        if (BM_FLOAT32 == context->bmNetwork->m_netinfo->output_dtypes[j]) {
            preds[j] = reinterpret_cast<float*>(output_data[j]) + output_sizes[j] * i;
        }
    }

    bool isAlignWidth=false;
    float ratio_ =get_aspect_scaled_ratio(frame_width, frame_height, context->net_w,
                                      context->net_h, &isAlignWidth); 

    // 应该是从json中读取的，现在先直接在这里定义
    int max_face_count=context->max_face_count;
    float score_threshold=context->score_threshold;
    std::vector<stFaceRect> results;

    vector<FaceDetectInfo> faceInfo;
    get_faceInfo(context,faceInfo,preds, output_names_map,frame_width, frame_height, ratio_,score_threshold);

    int face_num = 
       max_face_count > static_cast<int>(faceInfo.size()) ? static_cast<int>(faceInfo.size()) : max_face_count;

    for (int i = 0; i < face_num; i++) {
      std::shared_ptr<common::FaceObjectMetadata> detData =
            std::make_shared<common::FaceObjectMetadata>();
      detData->left = faceInfo[i].rect.x1;
      detData->right = faceInfo[i].rect.x2;
      detData->top = faceInfo[i].rect.y1;
      detData->bottom = faceInfo[i].rect.y2;
      detData->score = faceInfo[i].score;

      for(size_t k = 0; k < 5; k++) {
        detData->points_x[k] = faceInfo[i].pts.x[k];
        detData->points_y[k] = faceInfo[i].pts.y[k];
      }

      obj->mFaceObjectMetadata.push_back(detData);
    }
  }
}

void RetinafacePostProcess::get_faceInfo(std::shared_ptr<RetinafaceContext> context,vector<FaceDetectInfo> &faceInfo,
            float** preds, map<string, int> &output_names_map, int img_h, int img_w, float ratio_,
            float threshold, float scales){

    int hs=context->net_h;
    int ws=context->net_w;

    float * cls_data = preds[output_names_map["cls"]];
    float *land_data = preds[output_names_map["land"]];
    float *loc_data = preds[output_names_map["loc"]];
    

    const int num_layer = 3;
    const size_t steps[] = {8, 16, 32};
    const int num_anchor = 2;
    const size_t anchor_sizes[][2] = {
          {16, 32},
          {64, 128},
          {256, 512}};
    const float variances[] = {0.1, 0.2};

    size_t index = 0, min_size;
    const float *loc, *land;
    float x, y, w, h, conf;
    float anchor_w, anchor_h, anchor_x, anchor_y;


  FaceDetectInfo obj;
  for (int il = 0; il < num_layer; ++il){
    int feature_width = (ws + steps[il] - 1) / steps[il];
    int feature_height = (hs + steps[il] - 1) / steps[il];
    for (int iy = 0; iy < feature_height; ++iy){
      for (int ix = 0; ix < feature_width; ++ix){
        for (int ia = 0; ia < num_anchor; ++ia){
          conf = cls_data[index * 2 + 1];
          if (conf < threshold) goto cond;
          min_size = anchor_sizes[il][ia];
          anchor_x = (ix + 0.5) * steps[il] / ws;
          anchor_y = (iy + 0.5) * steps[il] / hs;
          anchor_w = min_size * 1. / ws;
          anchor_h = min_size * 1. / hs;
          obj.score = conf;
          loc = loc_data + index * 4;
          w = exp(loc[2] * variances[1]) * anchor_w;
          h = exp(loc[3] * variances[1]) * anchor_h;
          x = anchor_x + loc[0] * variances[0] * anchor_w;
          y = anchor_y + loc[1] * variances[0] * anchor_h;
          obj.rect.x1 = (x - w / 2) * 640 / ratio_;
          obj.rect.x2 = (x + w / 2) * 640 / ratio_;
          obj.rect.y1 = (y - h / 2) * 640 / ratio_;
          obj.rect.y2 = (y + h / 2) * 640 / ratio_;
          land = land_data + index * 10;
          for (int i = 0; i < 5; ++i){
            obj.pts.x[i] = (anchor_x +
                // land[i * 2] * variances[0] * anchor_w) * net_w_ / ratio_;
                // land[i * 2] * variances[0] * anchor_w) * img_w;
                land[i * 2] * variances[0] * anchor_w) * 640 / ratio_;
            obj.pts.y[i] = (anchor_y +
                // land[i * 2 + 1] * variances[0] * anchor_h) * net_h_ / ratio_;
                // land[i * 2 + 1] * variances[0] * anchor_h) * img_h;
                land[i * 2 + 1] * variances[0] * anchor_h) * 640 / ratio_;
          }
          faceInfo.push_back(obj);
cond:
          ++index;
        }
      }
    }
  }

  faceInfo = nms(faceInfo, threshold);

}

vector<anchor_box> RetinafacePostProcess::bbox_pred(vector<anchor_box> anchors, vector<vector<float> > regress) {
  vector<anchor_box> rects(anchors.size());
  for(size_t i = 0; i < anchors.size(); i++) {
    float width = anchors[i].x2 - anchors[i].x1 + 1;
    float height = anchors[i].y2 - anchors[i].y1 + 1;
    float ctr_x = anchors[i].x1 + 0.5 * (width - 1.0);
    float ctr_y = anchors[i].y1 + 0.5 * (height - 1.0);

    float pred_ctr_x = regress[i][0] * width + ctr_x;
    float pred_ctr_y = regress[i][1] * height + ctr_y;
    float pred_w = exp(regress[i][2]) * width;
    float pred_h = exp(regress[i][3]) * height;

    rects[i].x1 = pred_ctr_x - 0.5 * (pred_w - 1.0);
    rects[i].y1 = pred_ctr_y - 0.5 * (pred_h - 1.0);
    rects[i].x2 = pred_ctr_x + 0.5 * (pred_w - 1.0);
    rects[i].y2 = pred_ctr_y + 0.5 * (pred_h - 1.0);
  }

  return rects;
}

anchor_box RetinafacePostProcess::bbox_pred(anchor_box anchor, vector<float> regress) {
  anchor_box rect;

  float width = anchor.x2 - anchor.x1 + 1;
  float height = anchor.y2 - anchor.y1 + 1;
  float ctr_x = anchor.x1 + 0.5 * (width - 1.0);
  float ctr_y = anchor.y1 + 0.5 * (height - 1.0);

  float pred_ctr_x = regress[0] * width + ctr_x;
  float pred_ctr_y = regress[1] * height + ctr_y;
  float pred_w = exp(regress[2]) * width;
  float pred_h = exp(regress[3]) * height;

  rect.x1 = pred_ctr_x - 0.5 * (pred_w - 1.0);
  rect.y1 = pred_ctr_y - 0.5 * (pred_h - 1.0);
  rect.x2 = pred_ctr_x + 0.5 * (pred_w - 1.0);
  rect.y2 = pred_ctr_y + 0.5 * (pred_h - 1.0);

  return rect;
}

vector<FacePts> RetinafacePostProcess::landmark_pred(vector<anchor_box> anchors, vector<FacePts> facePts) {
  vector<FacePts> pts(anchors.size());
  for(size_t i = 0; i < anchors.size(); i++) {
    float width = anchors[i].x2 - anchors[i].x1 + 1;
    float height = anchors[i].y2 - anchors[i].y1 + 1;
    float ctr_x = anchors[i].x1 + 0.5 * (width - 1.0);
    float ctr_y = anchors[i].y1 + 0.5 * (height - 1.0);

    for(size_t j = 0; j < 5; j ++) {
      pts[i].x[j] = facePts[i].x[j] * width + ctr_x;
      pts[i].y[j] = facePts[i].y[j] * height + ctr_y;
    }
  }

  return pts;
}

FacePts RetinafacePostProcess::landmark_pred(anchor_box anchor, FacePts facePt) {
  FacePts pt;
  float width = anchor.x2 - anchor.x1 + 1;
  float height = anchor.y2 - anchor.y1 + 1;
  float ctr_x = anchor.x1 + 0.5 * (width - 1.0);
  float ctr_y = anchor.y1 + 0.5 * (height - 1.0);

  for(size_t j = 0; j < 5; j ++) {
    pt.x[j] = facePt.x[j] * width + ctr_x;
    pt.y[j] = facePt.y[j] * height + ctr_y;
  }

  return pt;
}

bool RetinafacePostProcess::CompareBBox(const FaceDetectInfo & a, const FaceDetectInfo & b) {
  return a.score > b.score;
}

vector<FaceDetectInfo> RetinafacePostProcess::nms(vector<FaceDetectInfo>& bboxes, float threshold) {
  vector<FaceDetectInfo> bboxes_nms;
  std::sort(bboxes.begin(), bboxes.end(), CompareBBox);

  int32_t select_idx = 0;
  int32_t num_bbox = static_cast<int32_t>(bboxes.size());
  vector<int32_t> mask_merged(num_bbox, 0);
  bool all_merged = false;

  while (!all_merged) {
    while (select_idx < num_bbox && mask_merged[select_idx] == 1)
      select_idx++;

    if (select_idx == num_bbox) {
      all_merged = true;
      continue;
    }

    bboxes_nms.push_back(bboxes[select_idx]);
    mask_merged[select_idx] = 1;

    anchor_box select_bbox = bboxes[select_idx].rect;
    float area1 = static_cast<float>((select_bbox.x2 - select_bbox.x1 + 1) * (select_bbox.y2 - select_bbox.y1 + 1));
    float x1 = static_cast<float>(select_bbox.x1);
    float y1 = static_cast<float>(select_bbox.y1);
    float x2 = static_cast<float>(select_bbox.x2);
    float y2 = static_cast<float>(select_bbox.y2);

    select_idx++;
    for (int32_t i = select_idx; i < num_bbox; i++) {
      if (mask_merged[i] == 1) {
        continue;
      }
      anchor_box& bbox_i = bboxes[i].rect;
      float x = std::max<float>(x1, static_cast<float>(bbox_i.x1));
      float y = std::max<float>(y1, static_cast<float>(bbox_i.y1));
      float w = std::min<float>(x2, static_cast<float>(bbox_i.x2)) - x + 1;
      float h = std::min<float>(y2, static_cast<float>(bbox_i.y2)) - y + 1;
      if (w <= 0 || h <= 0) {
        continue;
      }
      float area2 = static_cast<float>((bbox_i.x2 - bbox_i.x1 + 1) * (bbox_i.y2 - bbox_i.y1 + 1));
      float area_intersect = w * h;

      if (static_cast<float>(area_intersect) / (area1 + area2 - area_intersect) > threshold) {
        mask_merged[i] = 1;
      }
    }
  }
  return bboxes_nms;
}


}  // namespace retinaface
}  // namespace element
}  // namespace sophon_stream