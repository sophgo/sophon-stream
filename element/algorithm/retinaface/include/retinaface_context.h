//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_ELEMENT_RETINAFACE_CONTEXT_H_
#define SOPHON_STREAM_ELEMENT_RETINAFACE_CONTEXT_H_

#include "algorithmApi/context.h"

namespace sophon_stream {
namespace element {
namespace retinaface {

#define USE_ASPECT_RATIO 1

class RetinafaceContext : public ::sophon_stream::element::Context {
 public:
  int deviceId;  // 设备ID

  std::shared_ptr<BMNNContext> bmContext;
  std::shared_ptr<BMNNNetwork> bmNetwork;
  bm_handle_t handle;

  float thresh_nms;                                    // nms iou阈值
  std::vector<float> mean;  // 前处理均值， 长度为3，顺序为rgb
  std::vector<float> stdd;  // 前处理方差， 长度为3，顺序为rgb
  bool bgr2rgb;             // 是否将bgr图像转成rgb推理
  /**
   * @brief
   * 类别数量，从model中读取。需要和score_threshold、max_face_count的长度做校验
   */
  int m_frame_h, m_frame_w;
  int net_h, net_w, m_net_channel;
  int max_batch;
  int input_num;
  int output_num;
  int min_dim;
  bmcv_convert_to_attr converto_attr;
  int max_face_count = 50;
  float score_threshold = 0.5f;
};
}  // namespace retinaface
}  // namespace element
}  // namespace sophon_stream

#endif  // SOPHON_STREAM_ELEMENT_RETINAFACE_CONTEXT_H_