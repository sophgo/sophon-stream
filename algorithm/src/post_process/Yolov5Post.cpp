#include "Yolov5Post.h"
#include "../context/SophgoContext.h"
#include "common/Logger.h"

namespace sophon_stream {
namespace algorithm {
namespace post_process {

void Yolov5Post::init(algorithm::Context& context) {

}
void Yolov5Post::postProcess(algorithm::Context& context,
        common::ObjectMetadatas& objectMetadatas) {
            context::SophgoContext* pGpuContext = dynamic_cast<context::SophgoContext*>(&context);
        }


} // namespace post_process
} // namespace algorithm
} // namespace sophon_stream