//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_ELEMENT_UNET_UNET_PRE_H_
#define SOPHON_STREAM_ELEMENT_UNET_UNET_PRE_H_

#include <memory>
#include <string>
#include <vector>

#include "UnetSophgoContext.h"
#include "common/ErrorCode.h"
#include "common/ObjectMetadata.h"

namespace sophon_stream {
namespace element {
namespace unet {

class UnetPre {
 public:
  /**
   * execute preprocess
   * @param[in] objectMetadatas: input data
   * @param[out] context: data transport to model
   * @return ErrorCode
   */
  common::ErrorCode preProcess(UnetSophgoContext& context,
                               common::ObjectMetadatas& objectMetadatas);
  float get_aspect_scaled_ratio(int src_w, int src_h, int dst_w, int dst_h,
                                bool* pIsAligWidth);

  void initTensors(UnetSophgoContext& context,
                   common::ObjectMetadatas& objectMetadatas);
};
}  // namespace unet
}  // namespace element
}  // namespace sophon_stream

#endif // SOPHON_STREAM_ELEMENT_UNET_UNET_PRE_H_