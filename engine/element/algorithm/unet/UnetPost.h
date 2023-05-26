#pragma once

#include <memory>
#include <string>
#include <vector>

#include "UnetSophgoContext.h"
#include "common/ErrorCode.h"
#include "common/ObjectMetadata.h"

namespace sophon_stream {
namespace algorithm {
namespace unet {

class UnetPost {
 public:
  void init(UnetSophgoContext& context);

  float sigmoid(float x);

  void postProcess(UnetSophgoContext& context,
                   common::ObjectMetadatas& objectMetadatas);

  std::shared_ptr<common::Frame> bm_image2Frame(bm_handle_t&& handle,
                                                bm_image& img);
};
}  // namespace unet
}  // namespace algorithm
}  // namespace sophon_stream