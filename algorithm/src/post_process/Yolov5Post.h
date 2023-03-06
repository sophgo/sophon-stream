#pragma once

#include "../PostProcess.h"

namespace sophon_stream {
namespace algorithm {
namespace post_process {

/**
 * yolov5后处理
 */
class Yolov5Post : public algorithm::PostProcess {
  public:
    void init(algorithm::Context& context) override;

    void postProcess(algorithm::Context& context,
                     common::ObjectMetadatas& objectMetadatas) override;
};

} // namespace post_process
} // namespace algorithm
} // namespace sophon_stream