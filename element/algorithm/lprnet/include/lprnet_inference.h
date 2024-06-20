//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_ELEMENT_LPRNET_INFERENCE_H_
#define SOPHON_STREAM_ELEMENT_LPRNET_INFERENCE_H_

#include "algorithmApi/inference.h"
#include "lprnet_context.h"

namespace sophon_stream {
namespace element {
namespace lprnet {

class LprnetInference : public ::sophon_stream::element::Inference {
 public:
  ~LprnetInference() override;
  /**
   * @brief init device and engine
   * @param[in] context: model path,inputs and outputs name...
   */
  void init(std::shared_ptr<LprnetContext> context);

  /**
   * @brief network predict output
   * @param[in] context: inputData and outputData
   */
  common::ErrorCode predict(std::shared_ptr<LprnetContext> context,
                            common::ObjectMetadatas& objectMetadatas);
};

}  // namespace lprnet
}  // namespace element
}  // namespace sophon_stream

#endif  // SOPHON_STREAM_ELEMENT_LPRNET_INFERENCE_H_