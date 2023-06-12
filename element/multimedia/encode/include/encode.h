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

#include "common/object_metadata.h"
#include "element.h"
#include "encoder.h"

namespace sophon_stream {
namespace element {
namespace encode {

class Encode : public ::sophon_stream::framework::Element {
 public:
  Encode();
  ~Encode() override;

  common::ErrorCode initInternal(const std::string& json) override;
  void uninitInternal() override;

  common::ErrorCode doWork(int dataPipeId) override;

 private:
  std::map<int, std::shared_ptr<Encoder>> mEncoderMap;
  bm_handle_t m_handle;
  std::map<int, std::string> mChannelOutputPath;
  enum class EncodeType { RTSP, RTMP, VIDEO };
  EncodeType mEncodeType;
  std::string mRtspPort;
  std::string mRtmpPort;
  std::string encFmt;
  std::string pixFmt;
};

}  // namespace encode
}  // namespace element
}  // namespace sophon_stream

#endif  // SOPHON_STREAM_ELEMENT_ENCODE_H_