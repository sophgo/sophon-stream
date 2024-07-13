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

class Resize : public ::sophon_stream::framework::Element {
 public:
  Resize();
  ~Resize() override;

  common::ErrorCode initInternal(const std::string& json) override;

  common::ErrorCode doWork(int dataPipeId) override;

  common::ErrorCode resize_work(std::shared_ptr<common::ObjectMetadata> resObj);



  static constexpr const char* CONFIG_INTERNAL_DST_H_FILED = "dst_h";
  static constexpr const char* CONFIG_INTERNAL_DST_W_FILED = "dst_w";
  static constexpr const char* CONFIG_INTERNAL_CROP_TOP_FILED = "crop_top";
  static constexpr const char* CONFIG_INTERNAL_CROP_LEFT_FILED = "crop_left";
  static constexpr const char* CONFIG_INTERNAL_CROP_H_FILED = "crop_h";
  static constexpr const char* CONFIG_INTERNAL_CROP_W_FILED = "crop_w";

  int dst_h, dst_w;
  int crop_top,crop_left;
  int crop_h,crop_w;

 private:
  ::sophon_stream::common::FpsProfiler mFpsProfiler;
};

}  // namespace resize
}  // namespace element
}  // namespace sophon_stream

#endif
