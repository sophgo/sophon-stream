//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_ELEMENT_OSD_H_
#define SOPHON_STREAM_ELEMENT_OSD_H_

#include <memory>
#include <mutex>

#include "common/object_metadata.h"
#include "common/profiler.h"
#include "element.h"
#define USE_OPENCV_DRAW_BOX 1

namespace sophon_stream {
namespace element {
namespace osd {

class Osd : public ::sophon_stream::framework::Element {
 public:
  enum class OsdType { DET, TRACK, REC, UNKNOWN };
  Osd();
  ~Osd() override;

  common::ErrorCode initInternal(const std::string& json) override;

  common::ErrorCode doWork(int dataPipeId) override;

  static constexpr const char* CONFIG_INTERNAL_OSD_TYPE_FIELD = "osd_type";
  static constexpr const char* CONFIG_INTERNAL_CLASS_NAMES_FIELD =
      "class_names_file";

 private:
  std::vector<std::string> mClassNames;
  OsdType mOsdType;
  ::sophon_stream::common::FpsProfiler mFpsProfiler;
  void draw(std::shared_ptr<common::ObjectMetadata> objectMetadata);
};

}  // namespace osd
}  // namespace element
}  // namespace sophon_stream

#endif  // SOPHON_STREAM_ELEMENT_OSD_H_