//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_ELEMENT_LIGHTSTEREO_INFERENCE_H_
#define SOPHON_STREAM_ELEMENT_LIGHTSTEREO_INFERENCE_H_

#include "algorithmApi/inference.h"
#include "lightstereo_context.h"

namespace sophon_stream {
namespace element {
namespace lightstereo {

class LightstereoInference : public ::sophon_stream::element::Inference {
 public:
  ~LightstereoInference() override;
  /**
   * @brief init device and engine
   * @param[in] context: model path,inputs and outputs name...
   */
  void init(std::shared_ptr<LightstereoContext> context);
  /**
   * @brief network predict output
   * @param[in] context: inputData and outputData
   */
  common::ErrorCode predict(std::shared_ptr<LightstereoContext> context,
                            common::ObjectMetadatas& leftObjectMetadatas,
                            common::ObjectMetadatas& rightObjectMetadatas,
                            common::ObjectMetadatas& outputObjectMetadatas);
  std::shared_ptr<common::bmTensors> mergeInputDeviceMem(std::shared_ptr<LightstereoContext> context,
                                                         common::ObjectMetadatas& leftObjectMetadatas,
                                                         common::ObjectMetadatas& rightObjectMetadatas);
};

}  // namespace lightstereo
}  // namespace element
}  // namespace sophon_stream

#endif  // SOPHON_STREAM_ELEMENT_LIGHTSTEREO_INFERENCE_H_