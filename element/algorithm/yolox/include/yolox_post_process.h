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

#include "algorithmApi/post_process.h"
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

class YoloxPostProcess : public ::sophon_stream::element::PostProcess {
 public:
  void init(std::shared_ptr<YoloxContext> context);

  void postProcess(std::shared_ptr<YoloxContext> context,
                   common::ObjectMetadatas& objectMetadatas);
  ~YoloxPostProcess() override;

 private:
  float sigmoid(float x);
  int argmax(float* data, int num);
  float box_iou(const YoloxBox& a, const YoloxBox& b);

  void nms_sorted_bboxes(const std::vector<YoloxBox>& objects,
                         std::vector<int>& picked, float nms_threshold);

 private:
  int m_box_num;
  int* m_grids_x = nullptr;
  int* m_grids_y = nullptr;
  int* m_expanded_strides = nullptr;

  int m_min_box_area = 100;
};

}  // namespace yolox
}  // namespace element
}  // namespace sophon_stream

#endif  // SOPHON_STREAM_ELEMENT_YOLOX_POST_PROCESS_H_