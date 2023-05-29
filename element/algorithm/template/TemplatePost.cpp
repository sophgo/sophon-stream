//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#include "TemplatePost.h"

#include "TemplateSophgoContext.h"
#include "common/logger.h"

namespace sophon_stream {
namespace element {
namespace template {

  void TemplatePost::init(TemplateSophgoContext & context) {}

  void TemplatePost::postProcess(TemplateSophgoContext & context,
                                 common::ObjectMetadatas & objectMetadatas) {}
}  // namespace template
}  // namespace element
}  // namespace sophon_stream
