//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_ELEMENT_YOLOV7_PRE_PROCESS_H_
#define SOPHON_STREAM_ELEMENT_YOLOV7_PRE_PROCESS_H_

#include "algorithmApi/pre_process.h"
#include "yolov7_context.h"

namespace sophon_stream {
namespace element {
namespace yolov7 {

class Yolov7PreProcess : public ::sophon_stream::element::PreProcess {
 public:
  common::ErrorCode preProcess(std::shared_ptr<Yolov7Context> context,
                               common::ObjectMetadatas& objectMetadatas);
  void init(std::shared_ptr<Yolov7Context> context);
};

}  // namespace yolov7
}  // namespace element
}  // namespace sophon_stream

#endif  // SOPHON_STREAM_ELEMENT_YOLOV7_PRE_PROCESS_H_