//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_ELEMENT_YOLOV5_INFERENCE_H_
#define SOPHON_STREAM_ELEMENT_YOLOV5_INFERENCE_H_

#include <memory>
#include <string>
#include <vector>

#include "common/ErrorCode.h"
#include "common/ObjectMetadata.h"
#include "yolov5_context.h"

namespace sophon_stream {
namespace element {
namespace yolov5 {

class Yolov5Inference {
 public:
  ~Yolov5Inference();
  /**
   * init device and engine
   * @param[in] context: model path,inputs and outputs name...
   */
  void init(std::shared_ptr<Yolov5Context> context);

  /**
   * network predict output
   * @param[in] context: inputData and outputData
   */
  common::ErrorCode predict(std::shared_ptr<Yolov5Context> context,
                            common::ObjectMetadatas& objectMetadatas);

};

}  // namespace yolov5
}  // namespace element
}  // namespace sophon_stream

#endif  // SOPHON_STREAM_ELEMENT_YOLOV5_INFERENCE_H_