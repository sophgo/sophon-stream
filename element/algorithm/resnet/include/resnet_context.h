//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_ELEMENT_RESNET_CONTEXT_H_
#define SOPHON_STREAM_ELEMENT_RESNET_CONTEXT_H_

#include "algorithmApi/context.h"

namespace sophon_stream {
namespace element {
namespace resnet {

#define USE_ASPECT_RATIO 1

#define MAX_BATCH 16

// resnet TaskType枚举
enum class TaskType { SingleLabel, FeatureExtract, MultiLabel };

class ResNetContext : public sophon_stream::element::Context {
 public:
  int deviceId;  // 设备ID

  std::shared_ptr<BMNNContext> bmContext;
  std::shared_ptr<BMNNNetwork> bmNetwork;
  bm_handle_t handle;

  std::vector<float> mean;  // 前处理均值， 长度为3，顺序为rgb
  std::vector<float> stdd;  // 前处理方差， 长度为3，顺序为rgb
  bool bgr2rgb;             // 是否将bgr图像转成rgb推理
  bool bgr2gray;            // 是否将bgr图像转成gray推理
  // bool extract_feature = false;  // 是否直接提取特征

  TaskType taskType = TaskType::SingleLabel;
  std::vector<float> class_thresh;

  int m_frame_h, m_frame_w;
  int net_h, net_w, m_net_channel;
  int max_batch;
  int input_num;
  int output_num;
  int min_dim;
  int class_num;
  bmcv_convert_to_attr converto_attr;

  bmcv_rect_t roi;
  bool roi_predefined = false;
};
}  // namespace resnet
}  // namespace element
}  // namespace sophon_stream

#endif  // SOPHON_STREAM_ELEMENT_RESNET_CONTEXT_H_