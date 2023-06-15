//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_COMMON_CLOCKER_H_
#define SOPHON_STREAM_COMMON_CLOCKER_H_

#include <sys/time.h>

#include <string>

#include "common/no_copyable.h"

namespace sophon_stream {
namespace common {

class Clocker : public ::sophon_stream::common::NoCopyable {
 public:
  Clocker() { reset(); }
  ~Clocker() {}

  static void getCurrentUs(std::string& strDateTime, std::string& strDate) {
    time_t now;
    time(&now);
    char buf[sizeof "2011-10-08T07:07:09Z"];
    strftime(buf, sizeof buf, "%FT%TZ", gmtime(&now));
    strDate = std::string(&buf[0], 10);
    strDateTime = std::string(buf);
  }

  void reset() { gettimeofday(&time_begin_, nullptr); }

  long tell_ms() {
    gettimeofday(&time_end_, nullptr);
    return ((time_end_.tv_sec - time_begin_.tv_sec) * 1000 +
            (time_end_.tv_usec - time_begin_.tv_usec) / 1000);
  }

  long tell_us() {
    gettimeofday(&time_end_, nullptr);
    return ((time_end_.tv_sec - time_begin_.tv_sec) * 1000000 +
            (time_end_.tv_usec - time_begin_.tv_usec));
  }

 private:
  struct timeval time_begin_;
  struct timeval time_end_;
};  // class Clocker

}  // namespace common
}  // namespace sophon_stream
#endif  // SOPHON_STREAM_COMMON_CLOCKER_H_
