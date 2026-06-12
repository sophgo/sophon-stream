//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_ELEMENT_YOLO8_TEST_INFERENCE_H_
#define SOPHON_STREAM_ELEMENT_YOLO8_TEST_INFERENCE_H_

#include "algorithmApi/inference.h"
#include "yolo8_test_context.h"

namespace sophon_stream {
namespace element {
namespace yolo8_test {

class Yolo8TestInference : public ::sophon_stream::element::Inference {
 public:
  ~Yolo8TestInference() override;
  /**
   * init device and engine
   * @param[in] context: model path,inputs and outputs name...
   */
  void init(std::shared_ptr<Yolo8TestContext> context);

  /**
   * network predict output
   * @param[in] context: inputData and outputData
   */
  common::ErrorCode predict(std::shared_ptr<Yolo8TestContext> context,
                            common::ObjectMetadatas& objectMetadatas);
};

}  // namespace yolo8_test
}  // namespace element
}  // namespace sophon_stream

#endif  // SOPHON_STREAM_ELEMENT_YOLO8_TEST_INFERENCE_H_