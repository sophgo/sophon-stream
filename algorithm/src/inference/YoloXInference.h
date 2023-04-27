#pragma once

#include "../Inference.h"

namespace sophon_stream {
namespace algorithm {
namespace inference {

class YoloXInference : public algorithm::Inference {
  public:
    ~YoloXInference();
    /**
     * init device and engine
     * @param[in] context: model path,inputs and outputs name...
     */
    common::ErrorCode init(algorithm::Context& context) override;

    /**
     * network predict output
     * @param[in] context: inputData and outputData
     */
    common::ErrorCode predict(algorithm::Context& context, common::ObjectMetadatas &objectMetadatas) override;

    void uninit() override;

  private:
};

} // namespace inference
} // namespace algorithm
} // namespace sophon_stream