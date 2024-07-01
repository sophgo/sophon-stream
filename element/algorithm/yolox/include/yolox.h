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

#include "element_factory.h"
#include "group.h"
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

  const static std::string elementName;

  common::ErrorCode initInternal(const std::string& json) override;

  common::ErrorCode doWork(int dataPipeId) override;

  void setContext(std::shared_ptr<::sophon_stream::element::Context> context);
  void setPreprocess(std::shared_ptr<::sophon_stream::element::PreProcess> pre);
  void setInference(std::shared_ptr<::sophon_stream::element::Inference> infer);
  void setPostprocess(
      std::shared_ptr<::sophon_stream::element::PostProcess> post);
  void setStage(bool pre, bool infer, bool post);
  void initProfiler(std::string name, int interval);
  std::shared_ptr<::sophon_stream::element::Context> getContext() {
    return mContext;
  }
  std::shared_ptr<::sophon_stream::element::PreProcess> getPreProcess() {
    return mPreProcess;
  }
  std::shared_ptr<::sophon_stream::element::Inference> getInference() {
    return mInference;
  }
  std::shared_ptr<::sophon_stream::element::PostProcess> getPostProcess() {
    return mPostProcess;
  }

  static constexpr const char* CONFIG_INTERNAL_STAGE_NAME_FIELD = "stage";
  static constexpr const char* CONFIG_INTERNAL_MODEL_PATH_FIELD = "model_path";
  static constexpr const char* CONFIG_INTERNAL_THRESHOLD_CONF_FIELD =
      "threshold_conf";
  static constexpr const char* CONFIG_INTERNAL_THRESHOLD_NMS_FIELD =
      "threshold_nms";
  static constexpr const char* CONFIG_INTERNAL_THRESHOLD_BGR2RGB_FIELD =
      "bgr2rgb";
  static constexpr const char* CONFIG_INTERNAL_THRESHOLD_MEAN_FIELD = "mean";
  static constexpr const char* CONFIG_INTERNAL_THRESHOLD_STD_FIELD = "std";
  static constexpr const char* CONFIG_INTERNAL_CLASS_NAMES_FILE_FIELD =
      "class_names_file";
  static constexpr const char* CONFIG_INTERNAL_ROI_FILED = "roi";
  static constexpr const char* CONFIG_INTERNAL_LEFT_FILED = "left";
  static constexpr const char* CONFIG_INTERNAL_TOP_FILED = "top";
  static constexpr const char* CONFIG_INTERNAL_WIDTH_FILED = "width";
  static constexpr const char* CONFIG_INTERNAL_HEIGHT_FILED = "height";

 private:
  std::shared_ptr<YoloxContext> mContext;          // context对象
  std::shared_ptr<YoloxPreProcess> mPreProcess;    // 预处理对象
  std::shared_ptr<YoloxInference> mInference;      // 推理对象
  std::shared_ptr<YoloxPostProcess> mPostProcess;  // 后处理对象

  bool use_pre = false;
  bool use_infer = false;
  bool use_post = false;

  std::string mFpsProfilerName;
  ::sophon_stream::common::FpsProfiler mFpsProfiler;

  common::ErrorCode initContext(const std::string& json);
  void process(common::ObjectMetadatas& objectMetadatas);
};

}  // namespace yolox
}  // namespace element
}  // namespace sophon_stream

#endif  // SOPHON_STREAM_ELEMENT_YOLOX_H_