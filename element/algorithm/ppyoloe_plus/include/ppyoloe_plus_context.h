//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_ELEMENT_PPYOLOE_PLUS_CONTEXT_H_
#define SOPHON_STREAM_ELEMENT_PPYOLOE_PLUS_CONTEXT_H_

#include "algorithmApi/context.h"

namespace sophon_stream {
namespace element {
namespace ppyoloe_plus {

#define USE_ASPECT_RATIO 1

class Ppyoloe_plusContext : public ::sophon_stream::element::Context {
 public:
  int deviceId;  // 设备ID

  std::shared_ptr<BMNNContext> bmContext;
  std::shared_ptr<BMNNNetwork> bmNetwork;
  bm_handle_t handle;

  std::vector<float> mean;  // 前处理均值， 长度为3，顺序为rgb
  std::vector<float> stdd;  // 前处理方差， 长度为3，顺序为rgb
  bool bgr2rgb;             // 是否将bgr图像转成rgb推理
  bmcv_convert_to_attr converto_attr; // 图片做归一化，标准化预处理使用

  /**
   * @brief 最小的置信度阈值。详细说明请参考README
   */
  float thresh_conf_min = 0.5;
  /**
   * @brief 置信度阈值，key：类名，value：阈值
   * 该参数支持对不同的类别设置不同的阈值
   */
  std::unordered_map<std::string, float> thresh_conf;
  /**
   * @brief NMS IOU阈值
   */
  float thresh_nms;
  std::vector<std::string> class_names;
  /**
   * @brief 决定是否启用类别阈值
   */
  bool class_thresh_valid = false;

  /**
   * @brief
   * 类别数量，从model中读取。需要和thresh_conf、class_names的长度做校验
   */
  int class_num = 1;
  int m_frame_h, m_frame_w;
  int net_h, net_w, m_net_channel;
  int max_batch;
  int input_num;
  int output_num;
  int min_dim;

};
}  // namespace ppyoloe_plus
}  // namespace element
}  // namespace sophon_stream

#endif  // SOPHON_STREAM_ELEMENT_PPYOLOE_PLUS_CONTEXT_H_