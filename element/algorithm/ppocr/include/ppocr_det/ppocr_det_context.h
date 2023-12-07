//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_ELEMENT_PPOCR_DET_CONTEXT_H_
#define SOPHON_STREAM_ELEMENT_PPOCR_DET_CONTEXT_H_

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
namespace ppocr_det {

#define USE_ASPECT_RATIO 1
#define FFALIGN(x, a) (((x) + (a)-1) & ~((a)-1))

class Ppocr_detContext : public ::sophon_stream::framework::Context {
 public:
  int deviceId;  // 设备ID

  std::shared_ptr<BMNNContext> bmContext;
  std::shared_ptr<BMNNNetwork> bmNetwork;
  bm_handle_t handle;

  std::vector<float> mean;  // 前处理均值， 长度为3，顺序为rgb
  std::vector<float> stdd;  // 前处理方差， 长度为3，顺序为rgb
  bool bgr2rgb;             // 是否将bgr图像转成rgb推理

  /**
   * @brief 最小的置信度阈值。详细说明请参考README
   */
  float thresh_conf_min = -1;
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
  int class_num = 80;
  int m_frame_h, m_frame_w;
  int net_h, net_w, m_net_channel;
  int max_batch;
  int input_num;
  int output_num;
  int min_dim;
  bmcv_convert_to_attr converto_attr;

  /**
   * @brief json文件中定义的ROI，若此项生效，则只对ROI划定的区域做算法
   */
  bmcv_rect_t roi;
  bool roi_predefined = false;
};
}  // namespace ppocr_det
}  // namespace element
}  // namespace sophon_stream

#endif  // SOPHON_STREAM_ELEMENT_PPOCR_DET_CONTEXT_H_