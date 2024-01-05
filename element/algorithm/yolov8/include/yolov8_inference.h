//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_ELEMENT_YOLOV8_INFERENCE_H_
#define SOPHON_STREAM_ELEMENT_YOLOV8_INFERENCE_H_

#include <memory>
#include <string>
#include <vector>

#include "common/error_code.h"
#include "common/object_metadata.h"
#include "group.h"
#include "yolov8_context.h"

namespace sophon_stream {
namespace element {
namespace yolov8 {

class Yolov8Inference : public ::sophon_stream::framework::Inference {
 public:
  ~Yolov8Inference() override;
  /**
   * init device and engine
   * @param[in] context: model path,inputs and outputs name...
   */
  void init(std::shared_ptr<Yolov8Context> context);

  /**
   * network predict output
   * @param[in] context: inputData and outputData
   */
  common::ErrorCode predict(std::shared_ptr<Yolov8Context> context,
                            common::ObjectMetadatas& objectMetadatas);

 private:
  std::shared_ptr<sophon_stream::common::bmTensors> mergeInputDeviceMem(
      std::shared_ptr<Yolov8Context> context,
      common::ObjectMetadatas& objectMetadatas);

  std::shared_ptr<sophon_stream::common::bmTensors> getOutputDeviceMem(
      std::shared_ptr<Yolov8Context> context);

  void splitOutputMemIntoObjectMetadatas(
      std::shared_ptr<Yolov8Context> context,
      common::ObjectMetadatas& objectMetadatas,
      std::shared_ptr<sophon_stream::common::bmTensors> outputTensors);
};

}  // namespace yolov8
}  // namespace element
}  // namespace sophon_stream

#endif  // SOPHON_STREAM_ELEMENT_YOLOV8_INFERENCE_H_