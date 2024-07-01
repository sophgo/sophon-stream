//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#include "ppocr_det_post_process.h"

namespace sophon_stream {
namespace element {
namespace ppocr_det {

void Ppocr_detPostProcess::init(std::shared_ptr<Ppocr_detContext> context) {}

void Ppocr_detPostProcess::postProcess(
    std::shared_ptr<Ppocr_detContext> context,
    common::ObjectMetadatas& objectMetadatas) {
  if (objectMetadatas.size() == 0) return;
  float min_score_thresh = 0.3;
  double det_db_box_thresh = 0.6;
  double det_db_unclip_ratio = 1.5;

  const double threshold = min_score_thresh * 255;
  const double maxvalue = 255;
  bool use_polygon_score = false;

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

    for (int i = 0; i < context->output_num; ++i) {
      auto output_shape = context->bmNetwork->outputTensor(i)->get_shape();
      auto output_dims = output_shape->num_dims;
      int out_net_h_ = output_shape->dims[2];
      int out_net_w_ = output_shape->dims[3];

      float* predict_batch = (float*)outputTensors[i]->get_cpu_data();
      int resize_h = obj->resize_vector[0];
      int resize_w = obj->resize_vector[1];

      int frame_height = obj->mFrame->mSpData->height;
      int frame_width = obj->mFrame->mSpData->width;

      float ratio_h = float(resize_h) / float(frame_height);
      float ratio_w = float(resize_w) / float(frame_width);

      int n = out_net_h_ * out_net_w_;
      std::vector<float> pred(n, 0.0);
      std::vector<unsigned char> cbuf(n, ' ');

      for (int j = i * n; j < (i + 1) * n; j++) {
        pred[j - i * n] = float(predict_batch[j]);
        cbuf[j - i * n] = (unsigned char)((predict_batch[j]) * 255);
      }

      cv::Mat cbuf_map_(out_net_h_, out_net_w_, CV_8UC1,
                        (unsigned char*)cbuf.data());
      cv::Mat pred_map_(out_net_h_, out_net_w_, CV_32F, (float*)pred.data());

      cv::Rect crop_region(0, 0, resize_w, resize_h);
      cv::Mat cbuf_map = cbuf_map_(crop_region);
      cv::Mat pred_map = pred_map_(crop_region);

      cv::Mat bit_map;
      cv::threshold(cbuf_map, bit_map, threshold, maxvalue, cv::THRESH_BINARY);

      std::vector<std::vector<std::vector<int>>> boxes =
          m_post_processor.BoxesFromBitmap(
              pred_map, bit_map, det_db_box_thresh, det_db_unclip_ratio,
              use_polygon_score, frame_width, frame_height);

      OCRBoxVec ocrboxes =
          m_post_processor.FilterTagDetRes(boxes, *obj->mFrame->mSpData.get());

      for (auto ocrbox : ocrboxes) {
        std::shared_ptr<common::DetectedObjectMetadata> detData =
            std::make_shared<common::DetectedObjectMetadata>();

        // four conners of the box
        std::shared_ptr<common::PointMetadata> detPoint =
            std::make_shared<common::PointMetadata>();
        detPoint->mPoint = common::Point(ocrbox.x1, ocrbox.y1);
        detData->mKeyPoints.push_back(detPoint);
        detPoint = std::make_shared<common::PointMetadata>();
        detPoint->mPoint = common::Point(ocrbox.x2, ocrbox.y2);
        detData->mKeyPoints.push_back(detPoint);
        detPoint = std::make_shared<common::PointMetadata>();
        detPoint->mPoint = common::Point(ocrbox.x3, ocrbox.y3);
        detData->mKeyPoints.push_back(detPoint);
        detPoint = std::make_shared<common::PointMetadata>();
        detPoint->mPoint = common::Point(ocrbox.x4, ocrbox.y4);
        detData->mKeyPoints.push_back(detPoint);
        // distributor需要用到
        detData->mClassify = 0;
        obj->mDetectedObjectMetadatas.push_back(detData);
      }
    }
  }
}

}  // namespace ppocr_det
}  // namespace element
}  // namespace sophon_stream