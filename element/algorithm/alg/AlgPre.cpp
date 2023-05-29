//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#include "AlgPre.h"

#include "AlgSophgoContext.h"
#include "common/ObjectMetadata.h"
#include "common/logger.h"

namespace sophon_stream {
namespace element {
namespace alg {

  void AlgPre::initTensors(AlgSophgoContext & context,
                                common::ObjectMetadatas & objectMetadatas) {}

  common::ErrorCode AlgPre::preProcess(
      AlgSophgoContext & context,
      common::ObjectMetadatas & objectMetadatas) {
    return common::ErrorCode::SUCCESS;
  }

}  // namespace alg
}  // namespace element
}  // namespace sophon_stream