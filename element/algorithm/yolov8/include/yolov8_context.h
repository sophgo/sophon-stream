//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_ELEMENT_YOLOV8_CONTEXT_H_
#define SOPHON_STREAM_ELEMENT_YOLOV8_CONTEXT_H_

#include <memory>
#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>
#include <vector>

// for bmcv_api_ext.h
#include "bmcv_api_ext.h"
#include "bmlib_runtime.h"
#include "bmruntime_interface.h"
#include "common/bmnn_utils.h"
#include "common/error_code.h"
#include "group.h"

namespace sophon_stream {
namespace element {
namespace yolov8 {

#define USE_ASPECT_RATIO 1
#define FFALIGN(x, a) (((x) + (a)-1) & ~((a)-1))

class Yolov8Context : public ::sophon_stream::framework::Context {
 public:
  int deviceId;  // 设备ID

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

  int class_num = 80;  // default is coco names
  int m_frame_h, m_frame_w;
  int net_h, net_w, m_net_channel;
  int max_batch;
  int input_num;
  int output_num;  //
  int min_dim;
  bmcv_convert_to_attr converto_attr;

  bmcv_rect_t roi;
  bool roi_predefined = false;
  int thread_number;
};
}  // namespace yolov8
}  // namespace element
}  // namespace sophon_stream

#endif  // SOPHON_STREAM_ELEMENT_YOLOV8_CONTEXT_H_