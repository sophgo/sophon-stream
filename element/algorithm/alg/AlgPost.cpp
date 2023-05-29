//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#include "AlgPost.h"

#include "AlgSophgoContext.h"
#include "common/logger.h"

namespace sophon_stream {
namespace element {
namespace alg {

  void AlgPost::init(AlgSophgoContext & context) {}

  void AlgPost::postProcess(AlgSophgoContext & context,
                                 common::ObjectMetadatas & objectMetadatas) {}
}  // namespace alg
}  // namespace element
}  // namespace sophon_stream
