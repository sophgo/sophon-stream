//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#include "ppocr_det_post_process.h"

#include <cmath>

#include "common/logger.h"

namespace sophon_stream {
namespace element {
namespace ppocr_det {

void Ppocr_detPostProcess::init(std::shared_ptr<Ppocr_detContext> context) {}

void Ppocr_detPostProcess::postProcess(std::shared_ptr<Ppocr_detContext> context,
                                    common::ObjectMetadatas& objectMetadatas) {
  if (objectMetadatas.size() == 0) return;
  // write your post process here
}

}  // namespace ppocr_det
}  // namespace element
}  // namespace sophon_stream