//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_ELEMENT_FISHEYE_H_
#define SOPHON_STREAM_ELEMENT_FISHEYE_H_
#include <algorithm>
#include "common/common_defs.h"
#include "common/object_metadata.h"
#include "element.h"

namespace sophon_stream {
namespace element {
namespace fisheye {
class Fisheye : public ::sophon_stream::framework::Element {
 public:
  Fisheye();
  ~Fisheye() override;

  common::ErrorCode initInternal(const std::string& json) override;

  common::ErrorCode doWork(int dataPipeId) override;

  void fisheye_work(std::shared_ptr<common::ObjectMetadata> fisheyeObj);

  static constexpr const char* CONFIG_INTERNAL_IS_GRAY_FILED = "is_gray";
  bm_image_format_ext_ src_fmt;
  int subId = 0;

  bmcv_fisheye_attr_s fisheye_attr = {0};
  std::mutex dwa_lock;

 private:
};

}  // namespace dwa
}  // namespace element
}  // namespace sophon_stream

#endif
