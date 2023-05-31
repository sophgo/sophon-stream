//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_ELEMENT_YOLOV5_POST_PROCESS_H_
#define SOPHON_STREAM_ELEMENT_YOLOV5_POST_PROCESS_H_

#include <memory>
#include <string>
#include <vector>

#include "common/ErrorCode.h"
#include "common/ObjectMetadata.h"
#include "yolov5_context.h"

namespace sophon_stream {
namespace element {
namespace yolov5 {

struct YoloV5Box {
  int x, y, width, height;
  float score;
  int class_id;
};

using YoloV5BoxVec = std::vector<YoloV5Box>;

/**
 * yolov5后处理
 */
class Yolov5PostProcess {
 public:
  void init(std::shared_ptr<Yolov5Context> context);

  void postProcess(std::shared_ptr<Yolov5Context> context,
                   common::ObjectMetadatas& objectMetadatas);
  
  void initTpuKernel(std::shared_ptr<Yolov5Context> context);

  void setTpuKernelMem(std::shared_ptr<Yolov5Context> context,
                       common::ObjectMetadatas& objectMetadatas);

 private:
  float sigmoid(float x);
  int argmax(float* data, int num);
  void NMS(YoloV5BoxVec& dets, float nmsConfidence);
  float get_aspect_scaled_ratio(int src_w, int src_h, int dst_w,
                                                int dst_h, bool* pIsAligWidth);
};

}  // namespace yolov5
}  // namespace element
}  // namespace sophon_stream

#endif  // SOPHON_STREAM_ELEMENT_YOLOV5_POST_PROCESS_H_