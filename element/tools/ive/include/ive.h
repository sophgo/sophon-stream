//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_ELEMENT_IVE_H_
#define SOPHON_STREAM_ELEMENT_IVE_H_

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
namespace ive {

enum DisplayType { RAW_DPU_DIS = 0, DWA_DPU_DIS = 1, ONLY_DPU_DIS = 2 };

class Ive : public ::sophon_stream::framework::Element {
 public:
  Ive();
  ~Ive() override;

  common::ErrorCode initInternal(const std::string& json) override;

  common::ErrorCode doWork(int dataPipeId) override;

  common::ErrorCode ive_work(std::shared_ptr<common::ObjectMetadata> iveObj);
  void dpu_ive_map(bm_image& dpu_image, bm_image& dpu_image_map,
                   int ive_src_stride[]);

  static constexpr const char* CONFIG_INTERNAL_MAPY_FILED = "ive_mapy";
  static constexpr const char* CONFIG_INTERNAL_MAPU_FILED = "ive_mapu";
  static constexpr const char* CONFIG_INTERNAL_MAPV_FILED = "ive_mapv";

  static constexpr const char* CONFIG_INTERNAL_IS_IVE_FILED = "is_ive";

  DisplayType dis_type = DWA_DPU_DIS;
  
  int dev_id = 0;
  bm_handle_t handle = NULL;

  unsigned char FixMapU[256];
  unsigned char FixMapV[256];
  unsigned char FixMapY[256];

  bm_device_mem_t mapTable;
  bm_device_mem_t mapTableY;
  bm_device_mem_t mapTableU;
  bm_device_mem_t mapTableV;
  bm_ive_map_mode map_mode;

  bool is_ive;
  std::mutex mtx;

  ::sophon_stream::common::FpsProfiler mFpsProfiler;

 private:
  void setDispType(const httplib::Request& request,
                   httplib::Response& response);

  void registListenFunc(
      sophon_stream::framework::ListenThread* listener) override;
};

}  // namespace ive
}  // namespace element
}  // namespace sophon_stream

#endif