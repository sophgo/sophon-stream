//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_ELEMENT_LPRNET_CONTEXT_H_
#define SOPHON_STREAM_ELEMENT_LPRNET_CONTEXT_H_

#include <memory>
#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>
#include <vector>

#include "bmcv_api.h"
#include "bmcv_api_ext.h"
#include "bmlib_runtime.h"
#include "bmruntime_interface.h"
#include "common/bmnn_utils.h"
#include "common/error_code.h"

namespace sophon_stream {
namespace element {
namespace lprnet {

#define USE_ASPECT_RATIO 1
#define FFALIGN(x, a) (((x) + (a)-1) & ~((a)-1))

  struct LprnetContext {
    int deviceId;  // 设备ID

    std::shared_ptr<BMNNContext> bmContext;
    std::shared_ptr<BMNNNetwork> bmNetwork;
    bm_handle_t handle;

    int m_frame_h, m_frame_w;
    int net_h, net_w, m_net_channel;
    int max_batch;
    int input_num;
    int output_num;
    int min_dim;

    int clas_char;
    int len_char;

    bmcv_convert_to_attr converto_attr;

    bool roi_predefined = false;
  };
}  // namespace lprnet
}  // namespace element
}  // namespace sophon_stream

#endif  // SOPHON_STREAM_ELEMENT_LPRNET_CONTEXT_H_