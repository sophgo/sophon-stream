//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_ELEMENT_YOLOX_PRE_PROCESS_H_
#define SOPHON_STREAM_ELEMENT_YOLOX_PRE_PROCESS_H_

#include <memory>
#include <string>
#include <vector>

#include "common/error_code.h"
#include "common/object_metadata.h"
#include "group.h"
#include "yolox_context.h"

namespace sophon_stream {
namespace element {
namespace yolox {

class YoloxPreProcess : public ::sophon_stream::framework::PreProcess {
 public:
  common::ErrorCode preProcess(std::shared_ptr<YoloxContext> context,
                               common::ObjectMetadatas& objectMetadatas);
  void init(std::shared_ptr<YoloxContext> context);

 private:
  void initTensors(std::shared_ptr<YoloxContext> context,
                   common::ObjectMetadatas& objectMetadatas);
};

}  // namespace yolox
}  // namespace element
}  // namespace sophon_stream

#endif  // SOPHON_STREAM_ELEMENT_YOLOX_PRE_PROCESS_H_