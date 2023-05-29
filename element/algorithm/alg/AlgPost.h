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

  /**
   * alg后处理
   */
  class AlgPost {
   public:
    void init(AlgSophgoContext& context);

    void postProcess(AlgSophgoContext& objectMetadatas);
  };

}  // namespace alg
}  // namespace element
}  // namespace sophon_stream