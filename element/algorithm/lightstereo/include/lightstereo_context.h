//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_ELEMENT_LIGHTSTEREO_CONTEXT_H_
#define SOPHON_STREAM_ELEMENT_LIGHTSTEREO_CONTEXT_H_

#include "algorithmApi/context.h"

namespace sophon_stream {
namespace element {
namespace lightstereo {

#define USE_ASPECT_RATIO 1

class LightstereoContext : public ::sophon_stream::element::Context {
 public:
  int deviceId;  // 设备ID

  std::shared_ptr<BMNNContext> bmContext;
  std::shared_ptr<BMNNNetwork> bmNetwork;
  bm_handle_t handle;

  std::vector<float> mean = {0.485, 0.456, 0.406};  // 前处理均值， 长度为3，顺序为rgb
  std::vector<float> std = {0.229, 0.224, 0.225};  // 前处理方差， 长度为3，顺序为rgb
  bool bgr2rgb;             // 是否将bgr图像转成rgb推理

  int net_h, net_w, m_net_channel;
  int max_batch;
  int input_num;
  int output_num;
  bmcv_convert_to_attr converto_attr_left, converto_attr_right;
};
}  // namespace lightstereo
}  // namespace element
}  // namespace sophon_stream

#endif  // SOPHON_STREAM_ELEMENT_LIGHTSTEREO_CONTEXT_H_