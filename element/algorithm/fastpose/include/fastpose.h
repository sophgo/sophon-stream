//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_ELEMENT_FASTPOSE_H_
#define SOPHON_STREAM_ELEMENT_FASTPOSE_H_

#include "element_factory.h"
#include "fastpose_context.h"
#include "fastpose_inference.h"
#include "fastpose_post_process.h"
#include "fastpose_pre_process.h"
#include "group.h"

namespace sophon_stream {
namespace element {
namespace fastpose {

class Fastpose : public ::sophon_stream::framework::Element {
 public:
  Fastpose();
  ~Fastpose() override;

  const static std::string elementName;

  /**
   * @brief
   * 解析configure，初始化派生element的特有属性；调用initContext初始化算法相关参数
   * @param json json格式的配置文件
   * @return common::ErrorCode
   * 成功返回common::ErrorCode::SUCCESS，失败返回common::ErrorCode::PARSE_CONFIGURE_FAIL
   */
  common::ErrorCode initInternal(const std::string& json) override;

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
   * @brief
   * element的功能在这里实现。例如，算法模块需要实现组batch、调用算法、发送数据等功能
   * @param dataPipeId pop数据时对应的dataPipeId
   * @return common::ErrorCode 成功返回common::ErrorCode::SUCCESS
   */
  common::ErrorCode doWork(int dataPipeId) override;

  /**
   * @brief 从json文件读取的配置项
   */
  static constexpr const char* CONFIG_INTERNAL_STAGE_NAME_FIELD = "stage";
  static constexpr const char* CONFIG_INTERNAL_MODEL_PATH_FIELD = "model_path";

  static constexpr const char* CONFIG_INTERNAL_HEATMAP_LOSS_FIELD =
      "heatmap_loss";
  static constexpr const char* CONFIG_INTERNAL_AREA_THRESH_FIELD =
      "area_thresh";

 private:
  std::shared_ptr<FastposeContext> mContext;          // context对象
  std::shared_ptr<FastposePreProcess> mPreProcess;    // 预处理对象
  std::shared_ptr<FastposeInference> mInference;      // 推理对象
  std::shared_ptr<FastposePostProcess> mPostProcess;  // 后处理对象

  bool use_pre = false;
  bool use_infer = false;
  bool use_post = false;

  std::string mFpsProfilerName;
  ::sophon_stream::common::FpsProfiler mFpsProfiler;

  common::ErrorCode initContext(const std::string& json);
  void process(common::ObjectMetadatas& objectMetadatas);
};

}  // namespace fastpose
}  // namespace element
}  // namespace sophon_stream

#endif  // SOPHON_STREAM_ELEMENT_FASTPOSE_H_