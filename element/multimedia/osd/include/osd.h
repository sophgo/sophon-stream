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

namespace sophon_stream {
namespace element {
namespace osd {

enum OsdOutputFlags {
    VO = 1,  // VO
    ENCODE = 2  // 编码
};

class Osd : public ::sophon_stream::framework::Element {
 public:
  enum class OsdType { DET, TRACK, REC, POSE, AREA, ALGORITHM, TEXT, UNKNOWN };
  enum class DrawUtils { OPENCV, BMCV, UNKNOWN };
  Osd();
  ~Osd() override;
  common::ErrorCode initInternal(const std::string& json) override;

  common::ErrorCode doWork(int dataPipeId) override;

  static constexpr const char* CONFIG_INTERNAL_OSD_TYPE_FIELD = "osd_type";
  static constexpr const char* CONFIG_INTERNAL_CLASS_NAMES_FIELD =
      "class_names_file";
  static constexpr const char* CONFIG_INTERNAL_RECOGNIZE_NAMES_FIELD =
      "recognice_names_file";
  static constexpr const char* CONFIG_INTERNAL_DRAW_UTILS_FIELD = "draw_utils";
  static constexpr const char* CONFIG_INTERNAL_DRAW_INTERVAL_FIELD =
      "draw_interval";
  static constexpr const char* CONFIG_INTERNAL_PUT_TEXT_FIELD = "put_text";
  static constexpr const char* CONFIG_INTERNAL_DRAW_FUNC_NAME_FIELD =
      "draw_func_name";
  static constexpr const char* CONFIG_INTERNAL_HEATMAP_LOSS_FIELD =
      "heatmap_loss";
  static constexpr const char* CONFIG_INTERNAL_LEFT_FIELD = "lefts";
  static constexpr const char* CONFIG_INTERNAL_TOP_FIELD = "tops";
  static constexpr const char* CONFIG_INTERNAL_TEXT_FIELD = "texts";
  static constexpr const char* CONFIG_INTERNAL_FONT_LIBRARY_FIELD = "font_library";
  static constexpr const char* CONFIG_INTERNAL_R_FIELD = "r";
  static constexpr const char* CONFIG_INTERNAL_G_FIELD = "g";
  static constexpr const char* CONFIG_INTERNAL_B_FIELD = "b";

 private:
  std::vector<std::string> mClassNames;
  std::vector<std::string> mRecognizeNames;
  std::vector<int> tops, lefts;
  std::vector<std::string> texts;
  std::string font_library;
  OsdType mOsdType;
  DrawUtils mDrawUtils;
  bool mDrawInterval;
  bool mPutText;
  std::vector<bm_image> overlay_image_;
  int r, g, b;
  std::string heatmap_loss;
  std::function<void(std::shared_ptr<sophon_stream::common::ObjectMetadata>,
                     bm_image&)>
      draw_func;
  std::function<void(std::shared_ptr<sophon_stream::common::ObjectMetadata>,
                     cv::Mat&)>
      draw_func_opencv;
  ::sophon_stream::common::FpsProfiler mFpsProfiler;
  void draw(std::shared_ptr<common::ObjectMetadata> objectMetadata);

  std::unordered_map<int, int> channelOutputFlags;
};

}  // namespace osd
}  // namespace element
}  // namespace sophon_stream

#endif  // SOPHON_STREAM_ELEMENT_OSD_H_