//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_ELEMENT_POSEC3D_INFERENCE_H_
#define SOPHON_STREAM_ELEMENT_POSEC3D_INFERENCE_H_

#include "algorithmApi/inference.h"
#include "posec3d_context.h"

namespace sophon_stream {
namespace element {
namespace posec3d {

class Posec3dInference : public ::sophon_stream::element::Inference {
 public:
  ~Posec3dInference() override;
  /**
   * @brief init device and engine
   * @param[in] context: model path,inputs and outputs name...
   */
  void init(std::shared_ptr<Posec3dContext> context);

  /**
   * @brief network predict output
   * @param[in] context: inputData and outputData
   */
  common::ErrorCode predict(std::shared_ptr<Posec3dContext> context,
                            common::ObjectMetadatas& objectMetadatas);
};

}  // namespace posec3d
}  // namespace element
}  // namespace sophon_stream

#endif  // SOPHON_STREAM_ELEMENT_POSEC3D_INFERENCE_H_