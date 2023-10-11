//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-DEMO is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_ELEMENT_RESNET_CLASSIFY_H_
#define SOPHON_STREAM_ELEMENT_RESNET_CLASSIFY_H_

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "common/error_code.h"
#include "common/object_metadata.h"
#include "resnet_context.h"

namespace sophon_stream {
namespace element {
namespace resnet {

class ResNetMultiTask {
 public:
  ~ResNetMultiTask();
  /**
   * init device and engine
   * @param[in] context: model path,inputs and outputs name...
   */
  void init(std::shared_ptr<ResNetContext> context);

  common::ErrorCode multiTask(std::shared_ptr<ResNetContext> context,
                             common::ObjectMetadatas& objectMetadatas);

 private:
  // preprocess
  void initTensors(std::shared_ptr<ResNetContext> context,
                   common::ObjectMetadatas& objectMetadatas);

  common::ErrorCode pre_process(std::shared_ptr<ResNetContext> context,
                                common::ObjectMetadatas& objectMetadatas);

  static float get_aspect_scaled_ratio(int src_w, int src_h, int dst_w,
                                       int dst_h, bool* alignWidth);

  // inference
  std::shared_ptr<sophon_stream::common::bmTensors> mergeInputDeviceMem(
      std::shared_ptr<ResNetContext> context,
      common::ObjectMetadatas& objectMetadatas);

  std::shared_ptr<sophon_stream::common::bmTensors> getOutputDeviceMem(
      std::shared_ptr<ResNetContext> context);

  void splitOutputMemIntoObjectMetadatas(
      std::shared_ptr<ResNetContext> context,
      common::ObjectMetadatas& objectMetadatas,
      std::shared_ptr<sophon_stream::common::bmTensors> outputTensors);

  common::ErrorCode predict(std::shared_ptr<ResNetContext> context,
                            common::ObjectMetadatas& objectMetadatas);

  // postprocess
  common::ErrorCode post_process_classfy(std::shared_ptr<ResNetContext> context,
                                 common::ObjectMetadatas& objectMetadatas);
  common::ErrorCode post_process_extract(std::shared_ptr<ResNetContext> context,
                                 common::ObjectMetadatas& objectMetadatas);

  int subId = 0;
};
}  // namespace resnet
}  // namespace element
}  // namespace sophon_stream

#endif /* SOPHON_STREAM_ELEMENT_RESNET_CLASSIFY_H_ */
