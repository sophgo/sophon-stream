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

  static constexpr const char* CONFIG_INTERNAL_MAPY_FILED = "ive_mapy";
  static constexpr const char* CONFIG_INTERNAL_MAPU_FILED = "ive_mapu";
  static constexpr const char* CONFIG_INTERNAL_MAPV_FILED = "ive_mapv";

  DisplayType dis_type = RAW_DPU_DIS;
  int subId = 0;

  int dev_id = 0;
  bm_handle_t handle = NULL;

  // DPU FGS attrs
  bmcv_dpu_fgs_attrs dpu_fgs_attr;
  bmcv_dpu_fgs_mode dpu_fgs_mode;
  bmcv_dpu_sgbm_attrs dpu_sgbm_attr;
  bmcv_dpu_sgbm_mode dpu_sgbm_mode;

  bmcv_dpu_online_mode dpu_mode;

  unsigned char FixMapU[256];
  unsigned char FixMapV[256];
  unsigned char FixMapY[256];

  bm_device_mem_t mapTable;
  bm_device_mem_t mapTableY;
  bm_device_mem_t mapTableU;
  bm_device_mem_t mapTableV;
  bm_ive_map_mode map_mode;
  bm_image_format_ext dpu_fmt = FORMAT_GRAY;

 private:
  void getConfig(const httplib::Request& request, httplib::Response& response);
  void setConfig(const httplib::Request& request, httplib::Response& response);
  void setDispType(const httplib::Request& request,
                   httplib::Response& response);
  void registListenFunc(
      sophon_stream::framework::ListenThread* listener) override;

  std::mutex mtx;
};

}  // namespace dpu
}  // namespace element
}  // namespace sophon_stream

#endif