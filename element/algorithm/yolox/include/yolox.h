//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_ELEMENT_YOLOX_H_
#define SOPHON_STREAM_ELEMENT_YOLOX_H_

#include <memory>

#include "element.h"
#include "yolox_context.h"
#include "yolox_inference.h"
#include "yolox_post_process.h"
#include "yolox_pre_process.h"

namespace sophon_stream {
namespace element {
namespace yolox {

class Yolox : public ::sophon_stream::framework::Element {
 public:
  Yolox();
  ~Yolox() override;

  common::ErrorCode initInternal(const std::string& json) override;
  void uninitInternal() override;

  common::ErrorCode doWork(int dataPipeId) override;

  static constexpr const char* CONFIG_INTERNAL_STAGE_NAME_FIELD = "stage";
  static constexpr const char* CONFIG_INTERNAL_MODEL_PATH_FIELD = "model_path";
  static constexpr const char* CONFIG_INTERNAL_THRESHOLD_CONF_FIELD =
      "threshold_conf";
  static constexpr const char* CONFIG_INTERNAL_THRESHOLD_NMS_FIELD =
      "threshold_nms";

 private:
  std::shared_ptr<YoloxContext> mContext;          // context对象
  std::shared_ptr<YoloxPreProcess> mPreProcess;    // 预处理对象
  std::shared_ptr<YoloxInference> mInference;      // 推理对象
  std::shared_ptr<YoloxPostProcess> mPostProcess;  // 后处理对象

  bool use_pre = false;
  bool use_infer = false;
  bool use_post = false;

  int mBatch;

  common::ErrorCode initContext(const std::string& json);
  void process(common::ObjectMetadatas& objectMetadatas);
};

}  // namespace yolox
}  // namespace element
}  // namespace sophon_stream

#endif  // SOPHON_STREAM_ELEMENT_YOLOX_H_