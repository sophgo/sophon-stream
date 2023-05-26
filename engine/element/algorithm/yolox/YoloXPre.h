#pragma once

#include <memory>
#include <string>
#include <vector>

#include "YoloXSophgoContext.h"
#include "common/ErrorCode.h"
#include "common/ObjectMetadata.h"

namespace sophon_stream {
namespace element {
namespace yolox {

class YoloXPre {
 public:
  common::ErrorCode preProcess(YoloXSophgoContext& context,
                               common::ObjectMetadatas& objectMetadatas);

  void initTensors(YoloXSophgoContext& context,
                   common::ObjectMetadatas& objectMetadatas);
};

}  // namespace yolox
}  // namespace element
}  // namespace sophon_stream