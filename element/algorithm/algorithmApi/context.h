//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_ELEMENT_ALGORITHMAPI_CONTEXT_H_
#define SOPHON_STREAM_ELEMENT_ALGORITHMAPI_CONTEXT_H_

#include <cmath>
#include <memory>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include "common/bmnn_utils.h"
#include "common/common_defs.h"
#include "common/object_metadata.h"

namespace sophon_stream {
namespace element {

class Context {
 public:
  Context() = default;
  virtual ~Context() = default;
};

}  // namespace element
}  // namespace sophon_stream

#endif