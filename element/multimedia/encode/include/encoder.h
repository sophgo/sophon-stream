//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_ELEMENT_ENCODER_H_
#define SOPHON_STREAM_ELEMENT_ENCODER_H_

#include <unistd.h>

#include <csignal>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <regex>

#include "bmcv_api.h"
#include "bmcv_api_ext.h"

extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libavutil/time.h>
#include <libswscale/swscale.h>
}

namespace sophon_stream {
namespace element {
namespace encode {
class Encoder {
 public:
  Encoder();
  Encoder(bm_handle_t& handle, const std::string& enc_fmt,
          const std::string& pix_fmt, const std::string& enc_params);

  ~Encoder();

  void set_output_path(const std::string& output_path);
  void set_enc_params_width(int width);
  void set_enc_params_height(int height);
  void init_writer();
  bool is_opened();
  int video_write(bm_image& image);
  void release();

 private:
  class Encoder_CC;
  class Encoder_CC* const _impl;
};
}  // namespace encode
}  // namespace element
}  // namespace sophon_stream

#endif  // SOPHON_STREAM_ELEMENT_ENCODER_H_