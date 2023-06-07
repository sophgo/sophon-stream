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

  Yolox(const Yolox&) = delete;
  Yolox& operator=(const Yolox&) = delete;
  Yolox(Yolox&&) = default;
  Yolox& operator=(Yolox&&) = default;

  common::ErrorCode initInternal(const std::string& json) override;

  void process(common::ObjectMetadatas& objectMetadatas);

  void uninitInternal() override;

  common::ErrorCode doWork(int dataPipeId) override;

 private:
  std::shared_ptr<YoloxContext> mContext;          // context对象
  std::shared_ptr<YoloxPreProcess> mPreProcess;    // 预处理对象
  std::shared_ptr<YoloxInference> mInference;      // 推理对象
  std::shared_ptr<YoloxPostProcess> mPostProcess;  // 后处理对象

  bool use_pre = false;
  bool use_infer = false;
  bool use_post = false;

  common::ErrorCode initContext(const std::string& json);

  /**
   * @brief 需要推理的batch数
   */
  int mBatch;


};

}  // namespace yolox
}  // namespace element
}  // namespace sophon_stream

#endif  // SOPHON_STREAM_ELEMENT_YOLOX_H_