//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#include "TemplatePre.h"

#include "TemplateSophgoContext.h"
#include "common/ObjectMetadata.h"
#include "common/logger.h"

namespace sophon_stream {
namespace element {
namespace template {

  void TemplatePre::initTensors(TemplateSophgoContext & context,
                                common::ObjectMetadatas & objectMetadatas) {}

  common::ErrorCode TemplatePre::preProcess(
      TemplateSophgoContext & context,
      common::ObjectMetadatas & objectMetadatas) {
    return common::ErrorCode::SUCCESS;
  }

}  // namespace template
}  // namespace element
}  // namespace sophon_stream