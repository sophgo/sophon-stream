//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_ELEMENT_YOLOV5_H_
#define SOPHON_STREAM_ELEMENT_YOLOV5_H_

#include <memory>
#include <mutex>

#include "element.h"
#include "yolov5_context.h"
#include "yolov5_inference.h"
#include "yolov5_post_process.h"
#include "yolov5_pre_process.h"

namespace sophon_stream {
namespace element {
namespace yolov5 {

/**
 * 算法模块
 */
class Yolov5 : public ::sophon_stream::framework::Element {
 public:
  Yolov5();
  ~Yolov5() override;

  common::ErrorCode initInternal(const std::string& json) override;
  void uninitInternal() override;

  common::ErrorCode doWork(int dataPipeId) override;

  static constexpr const char* CONFIG_INTERNAL_STAGE_NAME_FIELD = "stage";
  static constexpr const char* CONFIG_INTERNAL_MODEL_PATH_FIELD = "model_path";
  static constexpr const char* CONFIG_INTERNAL_THRESHOLD_CONF_FIELD = "threshold_conf";
  static constexpr const char* CONFIG_INTERNAL_THRESHOLD_NMS_FIELD = "threshold_nms";
  static constexpr const char* CONFIG_INTERNAL_THRESHOLD_TPU_KERNEL_FIELD =
    "use_tpu_kernel";

 private:
  std::shared_ptr<Yolov5Context> mContext;          // context对象
  std::shared_ptr<Yolov5PreProcess> mPreProcess;    // 预处理对象
  std::shared_ptr<Yolov5Inference> mInference;      // 推理对象
  std::shared_ptr<Yolov5PostProcess> mPostProcess;  // 后处理对象

  bool use_pre = false;
  bool use_infer = false;
  bool use_post = false;
  int mBatch;

  common::ErrorCode initContext(const std::string& json);
  void process(common::ObjectMetadatas& objectMetadatas);
};

}  // namespace yolov5
}  // namespace element
}  // namespace sophon_stream

#endif  // SOPHON_STREAM_ELEMENT_YOLOV5_H_