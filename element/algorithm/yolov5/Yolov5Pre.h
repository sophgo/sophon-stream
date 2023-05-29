//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_ELEMENT_YOLOV5_PRE_H_
#define SOPHON_STREAM_ELEMENT_YOLOV5_PRE_H_

#include <memory>
#include <string>
#include <vector>

#include "Yolov5SophgoContext.h"
#include "common/ErrorCode.h"
#include "common/ObjectMetadata.h"

namespace sophon_stream {
namespace element {
namespace yolov5 {

class Yolov5Pre {
 public:
  /**
   * 执行预处理
   * @param[in] objectMetadatas:  输入数据
   * @param[out] context: 传输给推理模型的数据
   * @return 错误码
   */
  common::ErrorCode preProcess(Yolov5SophgoContext& context,
                               common::ObjectMetadatas& objectMetadatas);

  void initTensors(Yolov5SophgoContext& context,
                   common::ObjectMetadatas& objectMetadatas);

 private:
};

}  // namespace yolov5
}  // namespace element
}  // namespace sophon_stream

#endif // SOPHON_STREAM_ELEMENT_YOLOV5_PRE_H_