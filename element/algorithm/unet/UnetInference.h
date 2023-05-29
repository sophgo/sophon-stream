//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_ELEMENT_UNET_UNET_INFERENCE_H_
#define SOPHON_STREAM_ELEMENT_UNET_UNET_INFERENCE_H_

#include <memory>
#include <string>
#include <vector>

#include "UnetSophgoContext.h"
#include "common/ErrorCode.h"
#include "common/ObjectMetadata.h"

namespace sophon_stream {
namespace element {
namespace unet {

class UnetInference {
 public:
  ~UnetInference();

  /**
   * init device and engine
   * @param[in] context: model path, inputs and outputs name...
   */
  common::ErrorCode init(UnetSophgoContext& context);

  /**
   * network predict output
   * @param[in] context: inputData and outputData
   */
  common::ErrorCode predict(UnetSophgoContext& context,
                            common::ObjectMetadatas& objectMetadatas);

  void uninit();
};
}  // namespace unet
}  // namespace element
}  // namespace sophon_stream

#endif // SOPHON_STREAM_ELEMENT_UNET_UNET_INFERENCE_H_