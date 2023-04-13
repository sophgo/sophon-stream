#pragma once

#include "../PostProcess.h"

namespace sophon_stream {
namespace algorithm {
namespace post_process {

struct YoloXBox {
  // int x, y, width, height;
  // float score;
  // int class_id;

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
class YoloXPost : public algorithm::PostProcess {
  public:
    void init(algorithm::Context& context) override;

    void postProcess(algorithm::Context& context,
                     common::ObjectMetadatas& objectMetadatas) override;
  private:
    float sigmoid(float x);
    int argmax(float* data, int num);
  private:
    // void NMS(YoloXBoxVec &dets, float nmsConfidence);
    
};

} // namespace post_process
} // namespace algorithm
} // namespace sophon_stream