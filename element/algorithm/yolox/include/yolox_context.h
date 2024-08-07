//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_ELEMENT_YOLOX_CONTEXT_H_
#define SOPHON_STREAM_ELEMENT_YOLOX_CONTEXT_H_

#include "algorithmApi/context.h"

namespace sophon_stream {
namespace element {
namespace yolox {


class YoloxContext : public ::sophon_stream::element::Context {
 public:
  int deviceId;

  std::shared_ptr<BMNNContext> bmContext;
  std::shared_ptr<BMNNNetwork> bmNetwork;
  bm_handle_t handle;

  std::vector<float> mean;  // 前处理均值， 长度为3，顺序为rgb
  std::vector<float> stdd;  // 前处理方差， 长度为3，顺序为rgb
  bool bgr2rgb;             // 是否将bgr图像转成rgb推理

  float thresh_conf_min = -1;
  std::unordered_map<std::string, float> thresh_conf;  // 置信度阈值
  float thresh_nms;                                    // nms iou阈值
  std::vector<std::string> class_names;
  bool class_thresh_valid = false;

  int class_num = 80;
  int net_h;
  int net_w;
  int max_batch;
  int input_num;
  int output_num;
  bmcv_convert_to_attr converto_attr;

  bmcv_rect_t roi;
  bool roi_predefined = false;
};
}  // namespace yolox
}  // namespace element
}  // namespace sophon_stream

#endif  // SOPHON_STREAM_ELEMENT_YOLOX_CONTEXT_H_