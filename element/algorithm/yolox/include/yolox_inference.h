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

#include "algorithmApi/inference.h"
#include "yolox_context.h"

namespace sophon_stream {
namespace element {
namespace yolox {

class YoloxInference : public ::sophon_stream::element::Inference {
 public:
  ~YoloxInference() override;

  void init(std::shared_ptr<YoloxContext> context);

  common::ErrorCode predict(std::shared_ptr<YoloxContext> context,
                            common::ObjectMetadatas& objectMetadatas);
};

}  // namespace yolox
}  // namespace element
}  // namespace sophon_stream

#endif  // SOPHON_STREAM_ELEMENT_YOLOX_INFERENCE_H_