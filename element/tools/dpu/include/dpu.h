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