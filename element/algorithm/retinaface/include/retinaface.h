//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_ELEMENT_RETINAFACE_H_
#define SOPHON_STREAM_ELEMENT_RETINAFACE_H_

#include "element_factory.h"
#include "group.h"
#include "retinaface_context.h"
#include "retinaface_inference.h"
#include "retinaface_post_process.h"
#include "retinaface_pre_process.h"

namespace sophon_stream {
namespace element {
namespace retinaface {

class Retinaface : public ::sophon_stream::framework::Element {
 public:
  Retinaface();
  ~Retinaface() override;

  const static std::string elementName;

  /**
   * @brief
   * 解析configure，初始化派生element的特有属性；调用initContext初始化算法相关参数
   * @param json json格式的配置文件
   * @return common::ErrorCode
   * 成功返回common::ErrorCode::SUCCESS，失败返回common::ErrorCode::PARSE_CONFIGURE_FAIL
   */
  common::ErrorCode initInternal(const std::string& json) override;

  /**
   * @brief
   * element的功能在这里实现。例如，算法模块需要实现组batch、调用算法、发送数据等功能
   * @param dataPipeId pop数据时对应的dataPipeId
   * @return common::ErrorCode 成功返回common::ErrorCode::SUCCESS
   */
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

  /**
   * @brief 从json文件读取的配置项
   */
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

  static constexpr const char* CONFIG_INTERNAL_FACE_COUNT_FIELD =
      "max_face_count";
  static constexpr const char* CONFIG_INTERNAL_SCORE_THRESHOLD_FIELD =
      "score_threshold";

 private:
  std::shared_ptr<RetinafaceContext> mContext;          // context对象
  std::shared_ptr<RetinafacePreProcess> mPreProcess;    // 预处理对象
  std::shared_ptr<RetinafaceInference> mInference;      // 推理对象
  std::shared_ptr<RetinafacePostProcess> mPostProcess;  // 后处理对象

  bool use_pre = false;
  bool use_infer = false;
  bool use_post = false;

  std::string mFpsProfilerName;
  ::sophon_stream::common::FpsProfiler mFpsProfiler;

  common::ErrorCode initContext(const std::string& json);
  void process(common::ObjectMetadatas& objectMetadatas);
};

}  // namespace retinaface
}  // namespace element
}  // namespace sophon_stream

#endif  // SOPHON_STREAM_ELEMENT_RETINAFACE_H_