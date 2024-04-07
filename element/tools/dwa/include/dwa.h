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
#include "common/profiler.h"
#include "element.h"

namespace sophon_stream {
namespace element {
namespace dwa {

#define FFALIGN(x, a) (((x) + (a)-1) & ~((a)-1))

enum DwaMode {
  DWA_GDC_MODE,
  DWA_FISHEYE_MODE,
};

class Dwa : public ::sophon_stream::framework::Element {
 public:
  Dwa();
  ~Dwa() override;

  common::ErrorCode initInternal(const std::string& json) override;

  common::ErrorCode doWork(int dataPipeId) override;

  common::ErrorCode dwa_gdc_work(
      std::shared_ptr<common::ObjectMetadata> dwaObj);
  common::ErrorCode fisheye_work(
      std::shared_ptr<common::ObjectMetadata> dwaObj);

  float get_aspect_scaled_ratio(int src_w, int src_h, int dst_w, int dst_h,
                                bool* pIsAligWidth);
  static constexpr const char* CONFIG_INTERNAL_IS_GRAY_FILED = "is_gray";
  static constexpr const char* CONFIG_INTERNAL_IS_RESIZE_FILED = "is_resize";
  static constexpr const char* CONFIG_INTERNAL_IS_ROT_FILED = "is_rot";
  static constexpr const char* CONFIG_INTERNAL_DIS_MODE_FILED = "dis_mode";
  static constexpr const char* CONFIG_INTERNAL_GRID_NAME_FILED = "grid_name";
  static constexpr const char* CONFIG_INTERNAL_USE_GRIDE_FILED = "use_grid";
  static constexpr const char* CONFIG_INTERNAL_GRIDE_SIZE_FILED = "grid_size";

  static constexpr const char* CONFIG_INTERNAL_SRC_H_FILED = "src_h";
  static constexpr const char* CONFIG_INTERNAL_SRC_W_FILED = "src_w";
  static constexpr const char* CONFIG_INTERNAL_DST_H_FILED = "dst_h";
  static constexpr const char* CONFIG_INTERNAL_DST_W_FILED = "dst_w";

  static constexpr const char* CONFIG_INTERNAL_DWA_MODE_FILED = "dwa_mode";

  int src_h, src_w, dst_h, dst_w;

  bm_image_format_ext_ src_fmt;
  int subId = 0;
  bool is_resize = false;
  bool is_rot = false;

  bmcv_usage_mode dis_mode;
  DwaMode dwa_mode;

  bmcv_rot_mode rot_mode;

  std::string grid_name;

  bmcv_gdc_attr ldc_attr = {0};
  bmcv_fisheye_attr_s fisheye_attr = {0};

  std::mutex dwa_lock;

 private:
  ::sophon_stream::common::FpsProfiler mFpsProfiler;

  std::unordered_map<std::string, DwaMode> dwa_mode_map{
      {"DWA_GDC_MODE", DwaMode::DWA_GDC_MODE},
      {"DWA_FISHEYE_MODE", DwaMode::DWA_FISHEYE_MODE}};

  std::unordered_map<std::string, bmcv_usage_mode> fisheye_mode_map{
      {"BMCV_MODE_PANORAMA_360", bmcv_usage_mode::BMCV_MODE_PANORAMA_360},
      {"BMCV_MODE_PANORAMA_180", bmcv_usage_mode::BMCV_MODE_PANORAMA_180},
      {"BMCV_MODE_01_1O", bmcv_usage_mode::BMCV_MODE_01_1O},
      {"BMCV_MODE_02_1O4R", bmcv_usage_mode::BMCV_MODE_02_1O4R},
      {"BMCV_MODE_03_4R", bmcv_usage_mode::BMCV_MODE_03_4R},
      {"BMCV_MODE_04_1P2R", bmcv_usage_mode::BMCV_MODE_04_1P2R},
      {"BMCV_MODE_05_1P2R", bmcv_usage_mode::BMCV_MODE_05_1P2R},
      {"BMCV_MODE_06_1P", bmcv_usage_mode::BMCV_MODE_06_1P},
      {"BMCV_MODE_07_2P", bmcv_usage_mode::BMCV_MODE_07_2P},
      {"BMCV_MODE_STEREO_FIT", bmcv_usage_mode::BMCV_MODE_STEREO_FIT},
      {"BMCV_MODE_MAX", bmcv_usage_mode::BMCV_MODE_MAX}};
};

}  // namespace dwa
}  // namespace element
}  // namespace sophon_stream

#endif
