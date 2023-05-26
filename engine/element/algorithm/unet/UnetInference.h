#pragma once

#include <memory>
#include <string>
#include <vector>

#include "UnetSophgoContext.h"
#include "common/ErrorCode.h"
#include "common/ObjectMetadata.h"

namespace sophon_stream {
namespace algorithm {
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
}  // namespace algorithm
}  // namespace sophon_stream