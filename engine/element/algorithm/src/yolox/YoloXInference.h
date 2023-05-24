#pragma once

#include <memory>
#include <string>
#include <vector>
#include "common/ErrorCode.h"
#include "common/ObjectMetadata.h"
#include "YoloXSophgoContext.h"

namespace sophon_stream {
namespace algorithm {
namespace yolox {

class YoloXInference {
  public:
    ~YoloXInference();
    /**
     * init device and engine
     * @param[in] context: model path,inputs and outputs name...
     */
    common::ErrorCode init(YoloXSophgoContext& context);

    /**
     * network predict output
     * @param[in] context: inputData and outputData
     */
    common::ErrorCode predict(YoloXSophgoContext& context, common::ObjectMetadatas &objectMetadatas);

    void uninit();
};

} // namespace yolox
} // namespace algorithm
} // namespace sophon_stream