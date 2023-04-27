#pragma once

#include "../Inference.h"

namespace sophon_stream {
namespace algorithm {
namespace inference {

class UnetInference : public algorithm::Inference {
    public:
        ~UnetInference();

        /**
         * init device and engine
         * @param[in] context: model path, inputs and outputs name...
        */
       common::ErrorCode init(algorithm::Context & context) override;

       /**
        * network predict output
        * @param[in] context: inputData and outputData
       */
      common::ErrorCode predict(algorithm::Context & context, common::ObjectMetadatas &objectMetadatas) override;

      void uninit() override;
};
}
}
}