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

#include <memory>
#include <string>
#include <vector>

#include "common/ErrorCode.h"
#include "common/bm_wrapper.hpp"
#include "common/bmnn_utils.h"

#include <nlohmann/json.hpp>

namespace sophon_stream {
namespace element {
namespace yolox {

#define FFALIGN(x, a) (((x)+(a)-1)&~((a)-1))

struct YoloxContext {

  bm_handle_t handle;
  int deviceId;
  std::shared_ptr<BMNNContext> bmContext;
  std::shared_ptr<BMNNNetwork> bmNetwork;
  int class_num; 
  int net_h;
  int net_w;
  int input_num;
  int output_num;
  int max_batch;

  std::vector<bm_image> resized_imgs;
  std::vector<bm_image> converto_imgs;
  bmcv_convert_to_attr converto_attr;

  float thresh_conf;  // 置信度阈值
  float thresh_nms;   // nms iou阈值
};
}  // namespace yolox
}  // namespace element
}  // namespace sophon_stream

#endif  // SOPHON_STREAM_ELEMENT_YOLOX_CONTEXT_H_