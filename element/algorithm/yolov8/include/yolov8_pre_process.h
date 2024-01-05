//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_ELEMENT_YOLOV8_PRE_PROCESS_H_
#define SOPHON_STREAM_ELEMENT_YOLOV8_PRE_PROCESS_H_

#include <memory>
#include <string>
#include <vector>

#include "common/error_code.h"
#include "common/object_metadata.h"
#include "group.h"
#include "yolov8_context.h"

namespace sophon_stream {
namespace element {
namespace yolov8 {

class Yolov8PreProcess : public ::sophon_stream::framework::PreProcess {
 public:
  common::ErrorCode preProcess(std::shared_ptr<Yolov8Context> context,
                               common::ObjectMetadatas& objectMetadatas);
  void init(std::shared_ptr<Yolov8Context> context);

 private:
  void initTensors(std::shared_ptr<Yolov8Context> context,
                   common::ObjectMetadatas& objectMetadatas);
  float get_aspect_scaled_ratio(int src_w, int src_h, int dst_w, int dst_h,
                                bool* pIsAligWidth);
};

}  // namespace yolov8
}  // namespace element
}  // namespace sophon_stream

#endif  // SOPHON_STREAM_ELEMENT_YOLOV8_PRE_PROCESS_H_