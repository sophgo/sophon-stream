#pragma once

#include "../PostProcess.h"

namespace sophon_stream {
namespace algorithm {
namespace post_process {

struct YoloV5Box {
  int x, y, width, height;
  float score;
  int class_id;
};

using YoloV5BoxVec = std::vector<YoloV5Box>;

/**
 * yolov5后处理
 */
class Yolov5Post : public algorithm::PostProcess {
  public:
    void init(algorithm::Context& context) override;

    void postProcess(algorithm::Context& context,
                     common::ObjectMetadatas& objectMetadatas) override;
    void initTpuKernel(algorithm::Context& context);
    void setTpuKernelMem(algorithm::Context& context, common::ObjectMetadatas& objectMetadatas);
  private:
    float sigmoid(float x);
    int argmax(float* data, int num);
  private:
    void NMS(YoloV5BoxVec &dets, float nmsConfidence);
    
};

} // namespace post_process
} // namespace algorithm
} // namespace sophon_stream