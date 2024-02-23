//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_ELEMENT_YOLOV8_POST_PROCESS_H_
#define SOPHON_STREAM_ELEMENT_YOLOV8_POST_PROCESS_H_

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

struct YoloV8Box {
  int x1, y1, x2, y2;
  float score;
  int class_id;
  std::vector<float> kps;
};

using YoloV8BoxVec = std::vector<YoloV8Box>;

class Yolov8PostProcess : public ::sophon_stream::framework::PostProcess {
 public:
  void init(std::shared_ptr<Yolov8Context> context);

  void postProcess(std::shared_ptr<Yolov8Context> context,
                   common::ObjectMetadatas& objectMetadatas, int dataPipeId);

  ~Yolov8PostProcess() override;

  int max_det = 300;

 private:
  std::shared_ptr<Yolov8Context> global_context = nullptr;

  float sigmoid(float x);
  int argmax(float* data, int num);
  void NMS(YoloV8BoxVec& dets, float nmsConfidence);

  float get_aspect_scaled_ratio(int src_w, int src_h, int dst_w, int dst_h,
                                bool* pIsAligWidth);
  void postProcessDet(std::shared_ptr<Yolov8Context> context,
                      common::ObjectMetadatas& objectMetadatas);
  void postProcessPose(std::shared_ptr<Yolov8Context> context,
                       common::ObjectMetadatas& objectMetadatas);
  void postProcessCls(std::shared_ptr<Yolov8Context> context,
                       common::ObjectMetadatas& objectMetadatas);
  void clip_boxes(YoloV8BoxVec& yolobox_vec, int src_w, int src_h);
};

}  // namespace yolov8
}  // namespace element
}  // namespace sophon_stream

#endif  // SOPHON_STREAM_ELEMENT_YOLOV8_POST_PROCESS_H_