//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_ELEMENT_DWA_H_
#define SOPHON_STREAM_ELEMENT_DWA_H_
#include <algorithm>

#include "common/object_metadata.h"
#include "element.h"

namespace sophon_stream {
namespace element {
namespace dwa {

#define FFALIGN(x, a) (((x) + (a)-1) & ~((a)-1))

class Dwa : public ::sophon_stream::framework::Element {
 public:
  Dwa();
  ~Dwa() override;

  common::ErrorCode initInternal(const std::string& json) override;

  common::ErrorCode doWork(int dataPipeId) override;

  common::ErrorCode dwa_work(std::shared_ptr<common::ObjectMetadata> dwaObj);

  float get_aspect_scaled_ratio(int src_w, int src_h, int dst_w, int dst_h,
                                bool* pIsAligWidth);
  static constexpr const char* CONFIG_INTERNAL_IS_GRAY_FILED = "is_gray";
  static constexpr const char* CONFIG_INTERNAL_IS_RESIZE_FILED = "is_resize";

  bm_image_format_ext_ src_fmt;
  int subId = 0;

  bool is_resize = false;
  int dst_w = 1920;
  int dst_h = 1080;
  bmcv_gdc_attr ldc_attr = {true, 0, 0, 0, 0, 0, -200};
  std::mutex dwa_lock;

 private:
};

}  // namespace dwa
}  // namespace element
}  // namespace sophon_stream

#endif
