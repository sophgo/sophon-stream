//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_ELEMENT_YOLOV5_POST_H_
#define SOPHON_STREAM_ELEMENT_YOLOV5_POST_H_

#include <memory>
#include <string>
#include <vector>

#include "Yolov5SophgoContext.h"
#include "common/ErrorCode.h"
#include "common/ObjectMetadata.h"

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
class Yolov5Post {
 public:
  void init(Yolov5SophgoContext& context);

  void postProcess(Yolov5SophgoContext& context,
                   common::ObjectMetadatas& objectMetadatas);
  void initTpuKernel(Yolov5SophgoContext& context);
  void setTpuKernelMem(Yolov5SophgoContext& context,
                       common::ObjectMetadatas& objectMetadatas);

 private:
  float sigmoid(float x);
  int argmax(float* data, int num);

 private:
  void NMS(YoloV5BoxVec& dets, float nmsConfidence);
};

}  // namespace yolov5
}  // namespace element
}  // namespace sophon_stream

#endif // SOPHON_STREAM_ELEMENT_YOLOV5_POST_H_