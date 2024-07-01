//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#include "template_post_process.h"

namespace sophon_stream {
namespace element {
namespace template {

void TemplatePostProcess::init(std::shared_ptr<TemplateContext> context) {}

void TemplatePostProcess::postProcess(std::shared_ptr<TemplateContext> context,
                                    common::ObjectMetadatas& objectMetadatas) {
  if (objectMetadatas.size() == 0) return;
  // write your post process here
}

}  // namespace template
}  // namespace element
}  // namespace sophon_stream