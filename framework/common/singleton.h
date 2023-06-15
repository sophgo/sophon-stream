//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_COMMON_SINGLETON_H_
#define SOPHON_STREAM_COMMON_SINGLETON_H_

namespace sophon_stream {
namespace common {

template <class T>
class Singleton {
 public:
  typedef T ObjectType;

  static ObjectType& getInstance() {
    static ObjectType obj;
    return obj;
  }
};

}  // namespace common
}  // namespace sophon_stream

#endif  // SOPHON_STREAM_COMMON_SINGLETON_H_