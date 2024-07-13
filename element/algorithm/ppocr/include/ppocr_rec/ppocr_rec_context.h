//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_ELEMENT_PPOCR_REC_CONTEXT_H_
#define SOPHON_STREAM_ELEMENT_PPOCR_REC_CONTEXT_H_

#include "algorithmApi/context.h"

namespace sophon_stream {
namespace element {
namespace ppocr_rec {

#define USE_ASPECT_RATIO 1

struct RecModelSize {
  int w;
  int h;
};

class PpocrRecContext : public ::sophon_stream::element::Context {
 public:
  int deviceId;  // 设备ID

  std::shared_ptr<BMNNContext> bmContext;
  std::shared_ptr<BMNNNetwork> bmNetwork;
  bm_handle_t handle;

  int net_h, net_w, m_net_channel;
  int max_batch;
  int input_num;
  int output_num;
  bmcv_convert_to_attr converto_attr;
  bool beam_search = false;
  int beam_width = 3;
  std::vector<std::string> label_list_;

  /**
   * @brief ppocr network stage shapes.
   */
  std::vector<RecModelSize> img_size;
  /**
   * @brief ppocr network stage ratios, ratio = w / h
   */
  std::vector<float> img_ratio;
};
}  // namespace ppocr_rec
}  // namespace element
}  // namespace sophon_stream

#endif  // SOPHON_STREAM_ELEMENT_PPOCR_REC_CONTEXT_H_