//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_ELEMENT_YOLOX_INFERENCE_H_
#define SOPHON_STREAM_ELEMENT_YOLOX_INFERENCE_H_

#include <memory>
#include <string>
#include <vector>

#include "common/error_code.h"
#include "common/object_metadata.h"
#include "group.h"
#include "yolox_context.h"

namespace sophon_stream {
namespace element {
namespace yolox {

class YoloxInference : public ::sophon_stream::framework::Inference {
 public:
  ~YoloxInference() override;

  void init(std::shared_ptr<YoloxContext> context);

  common::ErrorCode predict(std::shared_ptr<YoloxContext> context,
                            common::ObjectMetadatas& objectMetadatas);

 private:
  std::shared_ptr<sophon_stream::common::bmTensors> mergeInputDeviceMem(
      std::shared_ptr<YoloxContext> context,
      common::ObjectMetadatas& objectMetadatas);

  std::shared_ptr<sophon_stream::common::bmTensors> getOutputDeviceMem(
      std::shared_ptr<YoloxContext> context);

  void splitOutputMemIntoObjectMetadatas(
      std::shared_ptr<YoloxContext> context,
      common::ObjectMetadatas& objectMetadatas,
      std::shared_ptr<sophon_stream::common::bmTensors> outputTensors);
};

}  // namespace yolox
}  // namespace element
}  // namespace sophon_stream

#endif  // SOPHON_STREAM_ELEMENT_YOLOX_INFERENCE_H_