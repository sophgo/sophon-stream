//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_ELEMENT_DPU_H_
#define SOPHON_STREAM_ELEMENT_DPU_H_

#include "common/object_metadata.h"
#include "common/profiler.h"
#include "element.h"
#define MAP_TABLE_SIZE 256
extern "C" {
extern bm_status_t bm_ive_image_calc_stride(bm_handle_t handle, int img_h,
                                            int img_w,
                                            bm_image_format_ext image_format,
                                            bm_image_data_format_ext data_type,
                                            int* stride) __attribute__((weak));
}
namespace sophon_stream {
namespace element {
namespace dpu {

enum DisplayType { RAW_DPU_DIS = 0, DWA_DPU_DIS = 1, ONLY_DPU_DIS = 2 };

enum DpuType { DPU_ONLINE, DPU_FGS, DPU_SGBM };

class Dpu : public ::sophon_stream::framework::Element {
 public:
  Dpu();
  ~Dpu() override;

  common::ErrorCode initInternal(const std::string& json) override;

  common::ErrorCode doWork(int dataPipeId) override;

  common::ErrorCode dpu_work(std::shared_ptr<common::ObjectMetadata> leftObj,
                             std::shared_ptr<common::ObjectMetadata> rightObj,
                             std::shared_ptr<common::ObjectMetadata> dpuObj);
  void dpu_ive_map(bm_image& dpu_image, bm_image& dpu_image_map,
                   int ive_src_stride[]);

  static constexpr const char* CONFIG_INTERNAL_DPU_TYPE_FILED = "dpu_type";
  static constexpr const char* CONFIG_INTERNAL_DPU_MODE_FILED = "dpu_mode";
  static constexpr const char* CONFIG_INTERNAL_IS_IVE_FILED = "is_ive";
  static constexpr const char* CONFIG_INTERNAL_DISP_RANGE_EN_FIELD =
      "disp_range_en";
  static constexpr const char* CONFIG_INTERNAL_DISP_START_POS_FIELD =
      "disp_start_pos";
  static constexpr const char* CONFIG_INTERNAL_DCC_DIR_EN_FIELD = "dcc_dir_en";
  static constexpr const char* CONFIG_INTERNAL_DPU_CENSUS_SHIFT_FIELD =
      "dpu_census_shift";
  static constexpr const char* CONFIG_INTERNAL_DPU_RSHIFT1_FIELD =
      "dpu_rshift1";
  static constexpr const char* CONFIG_INTERNAL_DPU_RSHIFT2_FIELD =
      "dpu_rshift2";
  static constexpr const char* CONFIG_INTERNAL_DPU_CA_P1_FIELD = "dpu_ca_p1";
  static constexpr const char* CONFIG_INTERNAL_DPU_CA_P2_FIELD = "dpu_ca_p2";
  static constexpr const char* CONFIG_INTERNAL_DPU_UNIQ_RATIO_FIELD =
      "dpu_uniq_ratio";
  static constexpr const char* CONFIG_INTERNAL_DPU_DISP_SHIFT_FIELD =
      "dpu_disp_shift";
  static constexpr const char* CONFIG_INTERNAL_DEPTH_UNIT_EN_FIELD =
      "depth_unit_en";
  static constexpr const char* CONFIG_INTERNAL_FGS_MAX_COUNT_FIELD =
      "fgs_max_count";
  static constexpr const char* CONFIG_INTERNAL_FGS_MAX_T_FIELD = "fgs_max_t";
  static constexpr const char* CONFIG_INTERNAL_FXBASE_LINE_FIELD =
      "fxbase_line";

  DisplayType dis_type = DWA_DPU_DIS;
  int subId = 0;

  // DPU FGS attrs
  bmcv_dpu_fgs_attrs dpu_fgs_attr;
  bmcv_dpu_fgs_mode dpu_fgs_mode;
  bmcv_dpu_sgbm_attrs dpu_sgbm_attr;
  bmcv_dpu_sgbm_mode dpu_sgbm_mode;

  bmcv_dpu_online_mode dpu_online_mode;

  bm_ive_map_mode map_mode;
  bm_image_format_ext dpu_fmt = FORMAT_GRAY;

  bool is_ive;

  int co = 0;
  DpuType dpu_type;

 private:
  void getConfig(const httplib::Request& request, httplib::Response& response);
  void setConfig(const httplib::Request& request, httplib::Response& response);
  void setDispType(const httplib::Request& request,
                   httplib::Response& response);
  void registListenFunc(
      sophon_stream::framework::ListenThread* listener) override;

  std::mutex mtx;
  ::sophon_stream::common::FpsProfiler mFpsProfiler;

  std::unordered_map<std::string, DpuType> dpu_type_map = {
      {"DPU_ONLINE", DpuType::DPU_ONLINE},
      {"DPU_FGS", DpuType::DPU_FGS},
      {"DPU_SGBM", DpuType::DPU_SGBM}};

  std::unordered_map<std::string, bmcv_dpu_online_mode_> online_mode_map{
      {"DPU_ONLINE_MUX0", bmcv_dpu_online_mode_::DPU_ONLINE_MUX0},
      {"DPU_ONLINE_MUX1", bmcv_dpu_online_mode_::DPU_ONLINE_MUX1},
      {"DPU_ONLINE_MUX2", bmcv_dpu_online_mode_::DPU_ONLINE_MUX2}};

  std::unordered_map<std::string, bmcv_dpu_fgs_mode_> fgs_mode_map{
      {"DPU_FGS_MUX0", bmcv_dpu_fgs_mode_::DPU_FGS_MUX0},
      {"DPU_FGS_MUX1", bmcv_dpu_fgs_mode_::DPU_FGS_MUX1}};

  std::unordered_map<std::string, bmcv_dpu_sgbm_mode_> sgbm_mode_map{
      {"DPU_SGBM_MUX0", bmcv_dpu_sgbm_mode_::DPU_SGBM_MUX0},
      {"DPU_SGBM_MUX1", bmcv_dpu_sgbm_mode_::DPU_SGBM_MUX1},
      {"DPU_SGBM_MUX2", bmcv_dpu_sgbm_mode_::DPU_SGBM_MUX2}};
};

}  // namespace dpu
}  // namespace element
}  // namespace sophon_stream

#endif