//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_ELEMENT_ENCODE_H_
#define SOPHON_STREAM_ELEMENT_ENCODE_H_

#include <memory>

#include "common/ObjectMetadata.h"
#include "element.h"
#include "encoder.h"

namespace sophon_stream {
namespace element {
namespace encode {

class Encode : public ::sophon_stream::framework::Element {
 public:
  Encode();
  ~Encode() override;

  Encode(const Encode&) = delete;
  Encode& operator=(const Encode&) = delete;
  Encode(Encode&&) = default;
  Encode& operator=(Encode&&) = default;

  common::ErrorCode initInternal(const std::string& json) override;
  void uninitInternal() override;

  common::ErrorCode doWork(int dataPipeId) override;

 private:
  std::map<int, std::shared_ptr<Encoder>> mEncoderMap;
  bm_handle_t m_handle;
  std::map<int, std::string> mChannelOutputPath;
  int mRtspPort;
};

}  // namespace encode
}  // namespace element
}  // namespace sophon_stream

#endif  // SOPHON_STREAM_ELEMENT_ENCODE_H_