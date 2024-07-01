//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#include "template_pre_process.h"

namespace sophon_stream {
namespace element {
namespace template {

void TemplatePreProcess::init(std::shared_ptr<TemplateContext> context) {}

common::ErrorCode TemplatePreProcess::preProcess(
    std::shared_ptr<TemplateContext> context,
    common::ObjectMetadatas& objectMetadatas) {
  if (objectMetadatas.size() == 0) return common::ErrorCode::SUCCESS;
  initTensors(context, objectMetadatas);
  
  // write your pre process here
  
  return common::ErrorCode::SUCCESS;
}

}  // namespace template
}  // namespace element
}  // namespace sophon_stream