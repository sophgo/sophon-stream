//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_ELEMENT_RESIZE_H_
#define SOPHON_STREAM_ELEMENT_RESIZE_H_
#include <algorithm>

#include "common/object_metadata.h"
#include "common/profiler.h"
#include "element.h"

namespace sophon_stream {
namespace element {
namespace resize {

#define FFALIGN(x, a) (((x) + (a)-1) & ~((a)-1))

class Resize : public ::sophon_stream::framework::Element {
 public:
  Resize();
  ~Resize() override;

  common::ErrorCode initInternal(const std::string& json) override;

  common::ErrorCode doWork(int dataPipeId) override;

  common::ErrorCode resize_work(std::shared_ptr<common::ObjectMetadata> resObj);

  float get_aspect_scaled_ratio(int src_w, int src_h, int dst_w, int dst_h,
                                bool* pIsAligWidth);

  static constexpr const char* CONFIG_INTERNAL_DST_H_FILED = "dst_h";
  static constexpr const char* CONFIG_INTERNAL_DST_W_FILED = "dst_w";
  static constexpr const char* CONFIG_INTERNAL_RATIO_FILED = "ratio";

  int dst_h, dst_w;
  float ratio;

 private:
  ::sophon_stream::common::FpsProfiler mFpsProfiler;
};

}  // namespace resize
}  // namespace element
}  // namespace sophon_stream

#endif
