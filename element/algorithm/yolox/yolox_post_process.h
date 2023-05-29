//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_ELEMENT_YOLOX_POST_PROCESS_H_
#define SOPHON_STREAM_ELEMENT_YOLOX_POST_PROCESS_H_

#include <memory>
#include <string>
#include <vector>

#include "common/ErrorCode.h"
#include "common/ObjectMetadata.h"
#include "yolox_context.h"

namespace sophon_stream {
namespace element {
namespace yolox {

struct YoloxBox {
  unsigned int class_id;
  float score;
  float left;
  float top;
  float right;
  float bottom;
  float width;
  float height;
};

using YoloxBoxVec = std::vector<YoloxBox>;

/**
 * yoloX后处理
 */
class YoloxPostProcess {
 public:
  void init(std::shared_ptr<YoloxContext> context);

  void postProcess(std::shared_ptr<YoloxContext> context,
                   common::ObjectMetadatas& objectMetadatas);
  ~YoloxPostProcess();

 private:
  float sigmoid(float x);
  int argmax(float* data, int num);

 private:
  // void NMS(YoloXBoxVec &dets, float nmsConfidence);
  int outlen_dim;
  int* grids_x_ = nullptr;
  int* grids_y_ = nullptr;
  int* expanded_strides_ = nullptr;
  int channel_len;

  bool outputs_3 = false;
};

}  // namespace yolox
}  // namespace element
}  // namespace sophon_stream

#endif  // SOPHON_STREAM_ELEMENT_YOLOX_POST_PROCESS_H_