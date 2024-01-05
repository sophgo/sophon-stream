//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_ELEMENT_BLEND_H_
#define SOPHON_STREAM_ELEMENT_BLEND_H_

#include "common/object_metadata.h"
#include "element.h"
void bm_read_bin(bm_image src, const char* input_name);
void bm_dem_read_bin(bm_handle_t handle, bm_device_mem_t* dmem,
                     const char* input_name, unsigned int size);
#define ALIGN(x, a) (((x) + ((a)-1)) & ~((a)-1))

namespace sophon_stream {
namespace element {
namespace blend {

enum DisplayType { RAW_BLEND_DIS = 0, DWA_BLEND_DIS = 1, ONLY_BLEND_DIS = 2 };
class Blend : public ::sophon_stream::framework::Element {
 public:
  Blend();
  ~Blend() override;

  common::ErrorCode initInternal(const std::string& json) override;

  common::ErrorCode doWork(int dataPipeId) override;

  common::ErrorCode blend_work(
      std::shared_ptr<common::ObjectMetadata> leftObj,
      std::shared_ptr<common::ObjectMetadata> rightObj,
      std::shared_ptr<common::ObjectMetadata> blendObj);

  static constexpr const char* CONFIG_INTERNAL_HEIGHT_FILED = "scr_h";
  static constexpr const char* CONFIG_INTERNAL_WEIGHT_FILED = "scr_w";
  static constexpr const char* CONFIG_INTERNAL_WGT1_FILED = "wgt1";
  static constexpr const char* CONFIG_INTERNAL_WGT2_FILED = "wgt2";

  static constexpr const char* CONFIG_INTERNAL_OVLP_LX_FILED = "ovlp_lx";
  static constexpr const char* CONFIG_INTERNAL_OVLP_RX_FILED = "ovlp_rx";
  static constexpr const char* CONFIG_INTERNAL_BD_LX0_FILED = "bd_lx0";
  static constexpr const char* CONFIG_INTERNAL_BD_RX0_FILED = "bd_rx0";
  static constexpr const char* CONFIG_INTERNAL_BD_LX1_FILED = "bd_lx1";
  static constexpr const char* CONFIG_INTERNAL_BD_RX1_FILED = "bd_rx1";

  static constexpr const char* CONFIG_INTERNAL_WET_MODE_FILED = "wgt_mode";


  DisplayType dis_type = RAW_BLEND_DIS;
  bool isDwa = false;
  int dev_id = 0;
  bm_handle_t handle = NULL;

  int subId = 0;
  int input_num = 2;
  int src_h, src_w;
  std::mutex mtx;
  struct stitch_param blend_config;

 private:
  void setDispType(const httplib::Request& request,
                   httplib::Response& response);

  void registListenFunc(
      sophon_stream::framework::ListenThread* listener) override;
};

}  // namespace blend
}  // namespace element
}  // namespace sophon_stream

#endif