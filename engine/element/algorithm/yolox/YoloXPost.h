#pragma once

#include <memory>
#include <string>
#include <vector>

#include "YoloXSophgoContext.h"
#include "common/ErrorCode.h"
#include "common/ObjectMetadata.h"

namespace sophon_stream {
namespace algorithm {
namespace yolox {

struct YoloXBox {
  unsigned int class_id;
  float score;
  float left;
  float top;
  float right;
  float bottom;
  float width;
  float height;
};

using YoloXBoxVec = std::vector<YoloXBox>;

/**
 * yoloX后处理
 */
class YoloXPost {
 public:
  void init(YoloXSophgoContext& context);

  void postProcess(YoloXSophgoContext& context,
                   common::ObjectMetadatas& objectMetadatas);
  ~YoloXPost();

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
}  // namespace algorithm
}  // namespace sophon_stream