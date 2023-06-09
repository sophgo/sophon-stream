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
  /**
   * 构造函数
   */
  Yolov5();
  /**
   * 析构函数
   */
  ~Yolov5() override;

  Yolov5(const Yolov5&) = delete;
  Yolov5& operator=(const Yolov5&) = delete;
  Yolov5(Yolov5&&) = default;
  Yolov5& operator=(Yolov5&&) = default;

  /**
   * 初始化函数
   * @param[in] side:  设备类型
   * @param[in] deviceId:  设备ID
   * @param[in] json:  初始化字符串
   * @return 错误码
   */
  common::ErrorCode initInternal(const std::string& json) override;
  /**
   * 预测函数
   * @param[in/out] objectMetadatas:  输入数据和预测结果
   */
  void process(common::ObjectMetadatas& objectMetadatas);
  /**
   * 资源释放函数
   */
  void uninitInternal() override;

  common::ErrorCode doWork(int dataPipeId) override;

 private:
  std::shared_ptr<Yolov5Context> mContext;  // context对象
  std::shared_ptr<Yolov5PreProcess> mPreProcess;         // 预处理对象
  std::shared_ptr<Yolov5Inference> mInference;    // 推理对象
  std::shared_ptr<Yolov5PostProcess> mPostProcess;       // 后处理对象

  bool use_pre = false;
  bool use_infer = false;
  bool use_post = false;

  common::ErrorCode initContext(const std::string& json);

  /**
   * @brief 需要推理的batch数
   */
  int mBatch;

};

}  // namespace yolov5
}  // namespace element
}  // namespace sophon_stream

#endif // SOPHON_STREAM_ELEMENT_YOLOV5_H_