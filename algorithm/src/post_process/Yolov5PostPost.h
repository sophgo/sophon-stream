#pragma once

#include "../PostProcess.h"

namespace sophon_stream {
namespace algorithm {
namespace post_process {

struct Yolov5PostBox {
  int x, y, width, height;
  float score;
  int class_id;
};

using Yolov5PostBoxVec = std::vector<Yolov5PostBox>;

/**
 * Yolov5Post后处理
 */
class Yolov5PostPost : public algorithm::PostProcess {
  public:
    void init(algorithm::Context& context) override;

    void initTpuKernel(algorithm::Context &context, common::ObjectMetadatas &objectMetadatas);

    void postProcess(algorithm::Context& context,
                     common::ObjectMetadatas& objectMetadatas) override;
  private:
    float sigmoid(float x);
    int argmax(float* data, int num);
  private:
    void NMS(Yolov5PostBoxVec &dets, float nmsConfidence);
    
};

} // namespace post_process
} // namespace algorithm
} // namespace sophon_stream