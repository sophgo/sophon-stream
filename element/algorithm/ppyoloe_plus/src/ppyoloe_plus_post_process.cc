//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#include "ppyoloe_plus_post_process.h"

namespace sophon_stream {
namespace element {
namespace ppyoloe_plus {

void Ppyoloe_plusPostProcess::init(std::shared_ptr<Ppyoloe_plusContext> context) {}

void Ppyoloe_plusPostProcess::postProcess(std::shared_ptr<Ppyoloe_plusContext> context,
                                    common::ObjectMetadatas& objectMetadatas) {
  if (objectMetadatas.size() == 0) return;
  // write your post process here

  ppYoloePlusBoxVec yolobox_vec;
  for (auto obj : objectMetadatas) {
    // 每帧处理逻辑
    if (obj->mFrame->mEndOfStream) break;

    // ----收集所有输出结果, ppyoloe有两个输出结果
    std::vector<std::shared_ptr<BMNNTensor>> outputTensors(context->output_num);
    for (int i = 0; i < context->output_num; i++) {
      outputTensors[i] = std::make_shared<BMNNTensor>(
          obj->mOutputBMtensors->handle,
          context->bmNetwork->m_netinfo->output_names[i],
          context->bmNetwork->m_netinfo->output_scales[i],
          obj->mOutputBMtensors->tensors[i].get(), context->bmNetwork->is_soc);
    }

    // ---置信度过滤
    yolobox_vec.clear();
    auto out_score = outputTensors[1];
    auto out_coordinate = outputTensors[0];
    int m_class_num = out_score->get_shape()->dims[1];

    // float* output_data = nullptr;
    float *out_score_ptr = nullptr;
    float *out_coordinate_ptr = nullptr;

    // box_num = 8400
    int box_num = out_coordinate->get_shape()->dims[1];
    out_score_ptr = (float*)out_score->get_cpu_data();
    out_coordinate_ptr = (float*)out_coordinate->get_cpu_data();


    // 计算缩放比例，dw、dh
    bm_image frame = *obj->mFrame->mSpData;
    int frame_width = frame.width;
    int frame_height = frame.height;
    int dw = 0, dh = 0;
    bool is_align_width = false;
    float ratio = get_aspect_scaled_ratio(frame.width, frame.height, context->net_w, context->net_h, &is_align_width);
    if (is_align_width) {
        dh = (int)((context->net_h - (int)(frame_height*ratio)) / 2);
    }else{
        dw = (int)((context->net_w - (int)(frame_width*ratio)) / 2);
    }
    
    // 每个框做类别解码，置信度过滤，结果保存到yolobox_vec中
    for (int i = 0; i < box_num; i++) {

        float *ptr0 = out_score_ptr + i;
        float *ptr1 = out_coordinate_ptr + i*4;

        int class_id = argmax_interval(&ptr0[0],m_class_num,box_num);

        float confidence = ptr0[class_id*box_num];

        if (confidence > context->thresh_conf_min) {

            ppYoloePlusBox box;
            box.x = std::max(int(round(ptr1[0])-int(dw/ratio)), 0);
            box.y = std::max(int(round(ptr1[1])-int(dh/ratio)), 0);
            int x2 = std::min(int(round(ptr1[2])-int(dw/ratio)), frame_width);
            int y2 = std::min(int(round(ptr1[3])-int(dh/ratio)), frame.height);
            box.width = x2 -box.x ;
            box.height = y2 - box.y;

            box.class_id = class_id;
            box.score = confidence;
            yolobox_vec.push_back(box);
        }
    }

    // --- NMS----
    NMS(yolobox_vec, context->thresh_nms);

    // ----结果保存到 mDetectedObjectMetadatas
    for (auto bbox : yolobox_vec) {
      std::shared_ptr<common::DetectedObjectMetadata> detData =
          std::make_shared<common::DetectedObjectMetadata>();
      detData->mBox.mX = bbox.x;
      detData->mBox.mY = bbox.y;
      detData->mBox.mWidth = bbox.width;
      detData->mBox.mHeight = bbox.height;
      detData->mScores.push_back(bbox.score);
      detData->mTopKLabels.push_back(0);
      detData->mClassify = bbox.class_id;
      detData->mClassifyName = context->class_names[bbox.class_id];

      if (context->class_thresh_valid) {
        detData->mLabelName = context->class_names[detData->mClassify];
      }
      
      obj->mDetectedObjectMetadatas.push_back(detData);
    }
  }   
}


int Ppyoloe_plusPostProcess::argmax_interval(float *data, int class_num, int box_num){
    float max_value = 0.0;
    int max_index = 0;
    for (int i = 0; i < class_num; ++i) {
        float value = data[i*box_num];
        if (value > max_value) {
            max_value = value;
            max_index = i;
        }
    }
    return max_index;
}

void Ppyoloe_plusPostProcess::NMS(ppYoloePlusBoxVec& dets, float nmsConfidence) {
    int length = dets.size();
    int index = length - 1;

    std::sort(dets.begin(), dets.end(), [](const ppYoloePlusBox& a, const ppYoloePlusBox& b) { return a.score < b.score; });

    std::vector<float> areas(length);
    for (int i = 0; i < length; i++) {
        areas[i] = dets[i].width * dets[i].height;
    }

    while (index > 0) {
        int i = 0;
        while (i < index) {
            float left = std::max(dets[index].x, dets[i].x);
            float top = std::max(dets[index].y, dets[i].y);
            float right = std::min(dets[index].x + dets[index].width, dets[i].x + dets[i].width);
            float bottom = std::min(dets[index].y + dets[index].height, dets[i].y + dets[i].height);
            float overlap = std::max(0.0f, right - left) * std::max(0.0f, bottom - top);
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

}  // namespace ppyoloe_plus
}  // namespace element
}  // namespace sophon_stream