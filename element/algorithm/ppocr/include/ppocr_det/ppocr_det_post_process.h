//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_ELEMENT_PPOCR_DET_POST_PROCESS_H_
#define SOPHON_STREAM_ELEMENT_PPOCR_DET_POST_PROCESS_H_

#include <memory>
#include <string>
#include <vector>

#include "common/error_code.h"
#include "common/object_metadata.h"
#include "group.h"
#include "ppocr_det_context.h"

namespace sophon_stream {
namespace element {
namespace ppocr_det {

class Ppocr_detPostProcess : public ::sophon_stream::framework::PostProcess {
 public:
  void init(std::shared_ptr<Ppocr_detContext> context);
  /**
   * @brief 对一个batch的数据做后处理
   * @param context context指针
   * @param objectMetadatas 一个batch的数据
   */
  void postProcess(std::shared_ptr<Ppocr_detContext> context,
                   common::ObjectMetadatas& objectMetadatas);
 private:
};

}  // namespace ppocr_det
}  // namespace element
}  // namespace sophon_stream

#endif  // SOPHON_STREAM_ELEMENT_PPOCR_DET_POST_PROCESS_H_