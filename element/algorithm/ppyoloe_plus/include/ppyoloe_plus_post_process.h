//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_ELEMENT_PPYOLOE_PLUS_POST_PROCESS_H_
#define SOPHON_STREAM_ELEMENT_PPYOLOE_PLUS_POST_PROCESS_H_

#include "algorithmApi/post_process.h"
#include "ppyoloe_plus_context.h"

namespace sophon_stream {
namespace element {
namespace ppyoloe_plus {

struct ppYoloePlusBox {
    int x, y, width, height;
    float score;
    int class_id;
};

using ppYoloePlusBoxVec = std::vector<ppYoloePlusBox>;

class Ppyoloe_plusPostProcess : public ::sophon_stream::element::PostProcess {
 public:
  void init(std::shared_ptr<Ppyoloe_plusContext> context);
  /**
   * @brief 对一个batch的数据做后处理
   * @param context context指针
   * @param objectMetadatas 一个batch的数据
   */
  void postProcess(std::shared_ptr<Ppyoloe_plusContext> context,
                   common::ObjectMetadatas& objectMetadatas);
 private:
  int argmax_interval(float *data, int class_num, int box_num);
  void NMS(ppYoloePlusBoxVec& dets, float nmsConfidence);
};

}  // namespace ppyoloe_plus
}  // namespace element
}  // namespace sophon_stream

#endif  // SOPHON_STREAM_ELEMENT_PPYOLOE_PLUS_POST_PROCESS_H_