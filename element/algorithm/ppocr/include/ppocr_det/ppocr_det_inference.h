//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_ELEMENT_PPOCR_DET_INFERENCE_H_
#define SOPHON_STREAM_ELEMENT_PPOCR_DET_INFERENCE_H_

#include "algorithmApi/inference.h"
#include "ppocr_det_context.h"

namespace sophon_stream {
namespace element {
namespace ppocr_det {

class Ppocr_detInference : public ::sophon_stream::element::Inference {
 public:
  ~Ppocr_detInference() override;
  /**
   * @brief init device and engine
   * @param[in] context: model path,inputs and outputs name...
   */
  void init(std::shared_ptr<Ppocr_detContext> context);
  /**
   * @brief network predict output
   * @param[in] context: inputData and outputData
   */
  common::ErrorCode predict(std::shared_ptr<Ppocr_detContext> context,
                            common::ObjectMetadatas& objectMetadatas);
};

}  // namespace ppocr_det
}  // namespace element
}  // namespace sophon_stream

#endif  // SOPHON_STREAM_ELEMENT_PPOCR_DET_INFERENCE_H_