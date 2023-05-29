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

/**
 * 算法模块
 */
class Yolox : public ::sophon_stream::framework::Element {
 public:
  /**
   * 构造函数
   */
  Yolox();
  /**
   * 析构函数
   */
  ~Yolox() override;

  Yolox(const Yolox&) = delete;
  Yolox& operator=(const Yolox&) = delete;
  Yolox(Yolox&&) = default;
  Yolox& operator=(Yolox&&) = default;

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
  // void uninit() override;
  void uninitInternal() override;

  common::ErrorCode doWork() override;
  /**
   * @brief 从processed objectmetadatas队列中获取数据并发送至0通道
   */
  common::ErrorCode sendProcessedData();

 private:
  std::shared_ptr<YoloxContext> mContext;          // context对象
  std::shared_ptr<YoloxPreProcess> mPreProcess;    // 预处理对象
  std::shared_ptr<YoloxInference> mInference;      // 推理对象
  std::shared_ptr<YoloxPostProcess> mPostProcess;  // 后处理对象

  bool use_pre = false;
  bool use_infer = false;
  bool use_post = false;

  bool hasEof = false;

  /**
   * @brief 需要推理的batch数
   */
  int mBatch;

  /**
   * @brief 上一次推理前datapipe中dataqueue的size
   */
  int mLastDataCount;

  /**
   * @brief 发送数据缓冲
   */
  std::deque<std::shared_ptr<common::ObjectMetadata> >
      mProcessedObjectMetadatas;

  /**
   * @brief batch数据缓冲
   */
  common::ObjectMetadatas mPendingObjectMetadatas;

  std::mutex mMutex2;
};

}  // namespace yolox
}  // namespace element
}  // namespace sophon_stream

#endif  // SOPHON_STREAM_ELEMENT_YOLOX_H_