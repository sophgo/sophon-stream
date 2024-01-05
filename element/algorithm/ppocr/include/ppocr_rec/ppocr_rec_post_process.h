//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_ELEMENT_PPOCR_REC_POST_PROCESS_H_
#define SOPHON_STREAM_ELEMENT_PPOCR_REC_POST_PROCESS_H_

#include <memory>
#include <string>
#include <vector>

#include "common/error_code.h"
#include "common/object_metadata.h"
#include "group.h"
#include "ppocr_rec_context.h"

namespace sophon_stream {
namespace element {
namespace ppocr_rec {

// get recognition results.
struct BeamSearchCandidate {
  std::vector<int> prefix;
  float score;
  std::vector<float> confs;
  BeamSearchCandidate() : score(0.0f) {}
  BeamSearchCandidate(const std::vector<int>& pref, float scr,
                      const std::vector<float>& cfs)
      : prefix(pref), score(scr), confs(cfs) {}

  bool operator<(const BeamSearchCandidate& other) const {
    return score < other.score;
  }
};

class PpocrRecPostProcess : public ::sophon_stream::framework::PostProcess {
 public:
  void init(std::shared_ptr<PpocrRecContext> context);
  /**
   * @brief 对一个batch的数据做后处理
   * @param context context指针
   * @param objectMetadatas 一个batch的数据
   */
  void postProcess(std::shared_ptr<PpocrRecContext> context,
                   common::ObjectMetadatas& objectMetadatas);

 private:
};

}  // namespace ppocr_rec
}  // namespace element
}  // namespace sophon_stream

#endif  // SOPHON_STREAM_ELEMENT_PPOCR_REC_POST_PROCESS_H_