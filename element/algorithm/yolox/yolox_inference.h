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

#include "common/ErrorCode.h"
#include "common/ObjectMetadata.h"
#include "yolox_context.h"

namespace sophon_stream {
namespace element {
namespace yolox {

class YoloxInference {
 public:
  ~YoloxInference();
  /**
   * init device and engine
   * @param[in] context: model path,inputs and outputs name...
   */
  common::ErrorCode init(std::shared_ptr<YoloxContext> context);

  /**
   * network predict output
   * @param[in] context: inputData and outputData
   */
  common::ErrorCode predict(std::shared_ptr<YoloxContext> context,
                            common::ObjectMetadatas& objectMetadatas);

  void uninit();
};

}  // namespace yolox
}  // namespace element
}  // namespace sophon_stream

#endif  // SOPHON_STREAM_ELEMENT_YOLOX_INFERENCE_H_