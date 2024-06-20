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

#include "algorithmApi/inference.h"
#include "yolov5_context.h"

namespace sophon_stream {
namespace element {
namespace yolov5 {

class Yolov5Inference : public ::sophon_stream::element::Inference {
 public:
  ~Yolov5Inference() override;
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

 private:
};

}  // namespace yolov5
}  // namespace element
}  // namespace sophon_stream

#endif  // SOPHON_STREAM_ELEMENT_YOLOV5_INFERENCE_H_