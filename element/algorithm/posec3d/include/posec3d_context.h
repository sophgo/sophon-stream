//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_ELEMENT_POSEC3D_CONTEXT_H_
#define SOPHON_STREAM_ELEMENT_POSEC3D_CONTEXT_H_

#include "algorithmApi/context.h"

namespace sophon_stream {
namespace element {
namespace posec3d {

#define USE_ASPECT_RATIO 1

class Posec3dContext : public ::sophon_stream::element::Context {
 public:
  int deviceId;  // 设备ID

  std::shared_ptr<BMNNContext> bmContext;
  std::shared_ptr<BMNNNetwork> bmNetwork;
  bm_handle_t handle;

  int m_frame_h, m_frame_w;
  int m_net_crops_clips, m_net_channel, m_net_keypoints, net_h, net_w;
  int max_batch;
  int input_num;
  int output_num;

  std::vector<std::string> class_names;
  float input_scale;
};
}  // namespace posec3d
}  // namespace element
}  // namespace sophon_stream

#endif  // SOPHON_STREAM_ELEMENT_POSEC3D_CONTEXT_H_