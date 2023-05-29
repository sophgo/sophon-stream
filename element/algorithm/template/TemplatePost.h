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

#include "TemplateSophgoContext.h"
#include "common/ErrorCode.h"
#include "common/ObjectMetadata.h"

namespace sophon_stream {
namespace element {
namespace template {

  /**
   * template后处理
   */
  class TemplatePost {
   public:
    void init(TemplateSophgoContext& context);

    void postProcess(TemplateSophgoContext& objectMetadatas);
  };

}  // namespace template
}  // namespace element
}  // namespace sophon_stream