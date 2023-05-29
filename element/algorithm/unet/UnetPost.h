//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_ELEMENT_UNET_UNET_POST_H_
#define SOPHON_STREAM_ELEMENT_UNET_UNET_POST_H_

#include <memory>
#include <string>
#include <vector>

#include "UnetSophgoContext.h"
#include "common/ErrorCode.h"
#include "common/ObjectMetadata.h"

namespace sophon_stream {
namespace element {
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
}  // namespace element
}  // namespace sophon_stream

#endif // SOPHON_STREAM_ELEMENT_UNET_UNET_POST_H_