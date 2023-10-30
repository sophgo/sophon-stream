//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_ELEMENT_OPENPOSE_PRE_PROCESS_H_
#define SOPHON_STREAM_ELEMENT_OPENPOSE_PRE_PROCESS_H_

#include <memory>
#include <string>
#include <vector>

#include "common/error_code.h"
#include "common/object_metadata.h"
#include "group.h"
#include "openpose_context.h"

namespace sophon_stream {
namespace element {
namespace openpose {

class OpenposePreProcess : public ::sophon_stream::framework::PreProcess {
 public:
  /**
   * @brief 对一个batch的数据做预处理
   * @param context context指针
   * @param objectMetadatas 一个batch的数据
   * @return common::ErrorCode
   * common::ErrorCode::SUCCESS，中间过程失败会中断执行
   */
  common::ErrorCode preProcess(std::shared_ptr<OpenposeContext> context,
                               common::ObjectMetadatas& objectMetadatas);
  void init(std::shared_ptr<OpenposeContext> context);

 private:
  /**
   * @brief 为一个batch的数据初始化设备内存
   * @param context context指针
   * @param objectMetadatas 一个batch的数据
   */
  void initTensors(std::shared_ptr<OpenposeContext> context,
                   common::ObjectMetadatas& objectMetadatas);
};

}  // namespace openpose
}  // namespace element
}  // namespace sophon_stream

#endif  // SOPHON_STREAM_ELEMENT_OPENPOSE_PRE_PROCESS_H_