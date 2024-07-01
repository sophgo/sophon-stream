//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_ELEMENT_ALGORITHMAPI_POSTPROCESS_H_
#define SOPHON_STREAM_ELEMENT_ALGORITHMAPI_POSTPROCESS_H_

#include "context.h"
namespace sophon_stream {
namespace element {

class PostProcess {
 public:
  PostProcess() = default;
  virtual ~PostProcess() = default;

  float get_aspect_scaled_ratio(int src_w, int src_h, int dst_w, int dst_h,
                                bool* pIsAligWidth) {
    float ratio;
    float r_w = (float)dst_w / src_w;
    float r_h = (float)dst_h / src_h;
    if (r_h > r_w) {
      *pIsAligWidth = true;
      ratio = r_w;
    } else {
      *pIsAligWidth = false;
      ratio = r_h;
    }
    return ratio;
  }
};

}  // namespace element
}  // namespace sophon_stream

#endif