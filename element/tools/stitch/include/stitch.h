//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_ELEMENT_STITCH_H_
#define SOPHON_STREAM_ELEMENT_STITCH_H_

#include "common/object_metadata.h"
#include "element.h"
#include "common/profiler.h"

namespace sophon_stream {
namespace element {
namespace stitch {


class Stitch : public ::sophon_stream::framework::Element {
 public:
  Stitch();
  ~Stitch() override;

  common::ErrorCode initInternal(const std::string& json) override;

  common::ErrorCode doWork(int dataPipeId) override;

  common::ErrorCode stitch_work(std::shared_ptr<common::ObjectMetadata> leftObj,
    std::shared_ptr<common::ObjectMetadata> rightObj
    ,std::shared_ptr<common::ObjectMetadata> stitchObj);

  static constexpr const char* CONFIG_INTERNAL_STITCH_MODE_FILED = "stitch_mode";

  int dev_id = 0;
  bm_handle_t handle = NULL;

  std::string stitch_mode;

  ::sophon_stream::common::FpsProfiler mFpsProfiler;
};

}  // namespace dpu
}  // namespace element
}  // namespace sophon_stream

#endif