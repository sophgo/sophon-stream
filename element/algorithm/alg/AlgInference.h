//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#pragma once

#include <memory>
#include <string>
#include <vector>

#include "AlgSophgoContext.h"
#include "common/ErrorCode.h"
#include "common/ObjectMetadata.h"

namespace sophon_stream {
namespace element {
namespace alg {

  class AlgInference {
   public:
    ~AlgInference();
    /**
     * init device and engine
     * @param[in] context: model path,inputs and outputs name...
     */
    common::ErrorCode init(AlgSophgoContext& context);

    /**
     * network predict output
     * @param[in] context: inputData and outputData
     */
    common::ErrorCode predict(AlgSophgoContext& context,
                              common::ObjectMetadatas& objectMetadatas);

    void uninit();
  };

}  // namespace alg
}  // namespace element
}  // namespace sophon_stream