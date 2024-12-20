//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_ELEMENT_LIGHTSTEREO_PRE_PROCESS_H_
#define SOPHON_STREAM_ELEMENT_LIGHTSTEREO_PRE_PROCESS_H_

#include "algorithmApi/pre_process.h"
#include "lightstereo_context.h"

namespace sophon_stream {
namespace element {
namespace lightstereo {

class LightstereoPreProcess : public ::sophon_stream::element::PreProcess {
 public:
  /**
   * @brief 对一个batch的数据做预处理
   * @param context context指针
   * @param objectMetadatas 一个batch的数据
   * @param tid 这一个batch数据对应的tensor id
   * @return common::ErrorCode
   * common::ErrorCode::SUCCESS，中间过程失败会中断执行
   */
  common::ErrorCode preProcess(std::shared_ptr<LightstereoContext> context,
                               common::ObjectMetadatas& objectMetadatas,
                               bool is_left=true);
  void init(std::shared_ptr<LightstereoContext> context);
};

}  // namespace lightstereo
}  // namespace element
}  // namespace sophon_stream

#endif  // SOPHON_STREAM_ELEMENT_LIGHTSTEREO_PRE_PROCESS_H_