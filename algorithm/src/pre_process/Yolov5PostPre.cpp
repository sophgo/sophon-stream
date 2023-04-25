#include "Yolov5PostPre.h"
#include "../context/SophgoContext.h"
#include "common/Logger.h"
#include "common/type_trans.hpp"
#include "common/Clocker.h"

namespace sophon_stream {
namespace algorithm {
namespace pre_process {

common::ErrorCode Yolov5PostPre::preProcess(algorithm::Context& context,
    common::ObjectMetadatas& objectMetadatas) {
        // context::SophgoContext* pSophgoContext = dynamic_cast<context::SophgoContext*>(&context);
        return common::ErrorCode::SUCCESS;
    }


} // namespace pre_process
} // namespace algorithm
} // namespace sophon_stream