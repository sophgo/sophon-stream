//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_COMMON_NO_COPYABLE_H_
#define SOPHON_STREAM_COMMON_NO_COPYABLE_H_

namespace sophon_stream {
namespace common {

class NoCopyable {
 protected:
  NoCopyable() = default;
  ~NoCopyable() = default;
  NoCopyable(const NoCopyable&) = delete;
  NoCopyable& operator=(const NoCopyable& rhs) = delete;
};

}  // namespace common
}  // namespace sophon_stream

#endif  // SOPHON_STREAM_COMMON_NO_COPYABLE_H_