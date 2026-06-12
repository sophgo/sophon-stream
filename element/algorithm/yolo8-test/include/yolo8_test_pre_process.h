//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_ELEMENT_YOLO8_TEST_PRE_PROCESS_H_
#define SOPHON_STREAM_ELEMENT_YOLO8_TEST_PRE_PROCESS_H_

#include "algorithmApi/pre_process.h"
#include "yolo8_test_context.h"

namespace sophon_stream {
namespace element {
namespace yolo8_test {

class Yolo8TestPreProcess : public ::sophon_stream::element::PreProcess {
 public:
  common::ErrorCode preProcess(std::shared_ptr<Yolo8TestContext> context,
                               common::ObjectMetadatas& objectMetadatas);
  void init(std::shared_ptr<Yolo8TestContext> context);
};

}  // namespace yolo8_test
}  // namespace element
}  // namespace sophon_stream

#endif  // SOPHON_STREAM_ELEMENT_YOLO8_TEST_PRE_PROCESS_H_