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
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

#include "bmcv_api.h"
#include "bmcv_api_ext.h"
#include "bmlib_runtime.h"
#include "bmruntime_interface.h"
#include "common/ErrorCode.h"
#include "common/bmnn_utils.h"

namespace sophon_stream {
namespace element {
namespace yolox {

#define FFALIGN(x, a) (((x) + (a)-1) & ~((a)-1))

struct YoloxContext {
  int deviceId;

  std::shared_ptr<BMNNContext> bmContext;
  std::shared_ptr<BMNNNetwork> bmNetwork;
  bm_handle_t handle;

  float thresh_conf;  // 置信度阈值
  float thresh_nms;   // nms iou阈值

  int class_num = 80;
  int net_h;
  int net_w;
  int max_batch;
  int input_num;
  int output_num;
  bmcv_convert_to_attr converto_attr;
};
}  // namespace yolox
}  // namespace element
}  // namespace sophon_stream

#endif  // SOPHON_STREAM_ELEMENT_YOLOX_CONTEXT_H_