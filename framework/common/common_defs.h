//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_COMMON_COMMON_DEFS_H_
#define SOPHON_STREAM_COMMON_COMMON_DEFS_H_

#include <iostream>
#include <sstream>
#include <string>

#include "logger.h"

#define STREAM_LIKELY(expr) (__builtin_expect(static_cast<bool>(expr), 1))
#define STREAM_UNLIKELY(expr) (__builtin_expect(static_cast<bool>(expr), 0))

inline std::string concatArgs() { return ""; }

template <typename T, typename... Args>
inline std::string concatArgs(const T& arg, const Args&... args) {
  std::stringstream ss;
  ss << std::string(arg);
  return ss.str() + concatArgs(args...);
}

#define STREAM_CHECK(cond, ...)                                     \
  if (STREAM_UNLIKELY(!(cond))) {                                   \
    std::string msg = concatArgs(__VA_ARGS__);                      \
    std::string error_msg =                                         \
        "Expected " #cond " to be true, but got false. " + (msg);   \
    std::cerr << "\033[0;31m"                                       \
              << "[STREAM_CHECK_ERROR] "                            \
              << "\033[0m"                                          \
              << "\t" << __FILE__ << ": " << __LINE__ << std::endl; \
    std::cerr << "\033[0;31m"                                       \
              << "[STREAM_CHECK_MESSAGE] "                          \
              << "\033[0m"                                          \
              << "\t" << error_msg << std::endl;                    \
    exit(1);                                                        \
  }

#endif