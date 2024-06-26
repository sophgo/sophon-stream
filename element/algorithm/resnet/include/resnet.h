//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_ELEMENT_RESNET_H_
#define SOPHON_STREAM_ELEMENT_RESNET_H_

#include "element_factory.h"
#include "resnet_context.h"
#include "resnet_multitask.h"

namespace sophon_stream {
namespace element {
namespace resnet {

/**
 * 算法模块
 */
class ResNet : public ::sophon_stream::framework::Element {
 public:
  ResNet();
  ~ResNet() override;

  common::ErrorCode initInternal(const std::string& json) override;

  common::ErrorCode doWork(int dataPipeId) override;

  static constexpr const char* CONFIG_INTERNAL_MODEL_PATH_FIELD = "model_path";
  static constexpr const char* CONFIG_INTERNAL_THRESHOLD_BGR2RGB_FIELD =
      "bgr2rgb";
  static constexpr const char* CONFIG_INTERNAL_THRESHOLD_BGR2GRAY_FIELD =
      "bgr2gray";

  static constexpr const char* CONFIG_INTERNAL_THRESHOLD_MEAN_FIELD = "mean";
  static constexpr const char* CONFIG_INTERNAL_THRESHOLD_STD_FIELD = "std";

  static constexpr const char* CONFIG_INTERNAL_ROI_FILED = "roi";
  static constexpr const char* CONFIG_INTERNAL_LEFT_FILED = "left";
  static constexpr const char* CONFIG_INTERNAL_TOP_FILED = "top";
  static constexpr const char* CONFIG_INTERNAL_WIDTH_FILED = "width";
  static constexpr const char* CONFIG_INTERNAL_HEIGHT_FILED = "height";
  static constexpr const char* CONFIG_INTERNAL_TASK_TYPE_FIELD = "task_type";
  static constexpr const char* CONFIG_INTERNAL_CLASS_THRESH_FIELD =
      "class_thresh";

 private:
  std::shared_ptr<ResNetContext> mContext;      // context对象
  std::shared_ptr<ResNetMultiTask> mMultiTask;  // 推理对象
  int mBatch;

  ::sophon_stream::common::FpsProfiler mFpsProfiler;

  common::ErrorCode initContext(const std::string& json);
  void process(common::ObjectMetadatas& objectMetadatas);
};

}  // namespace resnet
}  // namespace element
}  // namespace sophon_stream

#endif  // SOPHON_STREAM_ELEMENT_RESNET_H_