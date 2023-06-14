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

  static constexpr const char* CONFIG_INTERNAL_ENCODE_TYPE_FIELD =
      "encode_type";
  static constexpr const char* CONFIG_INTERNAL_RTSP_PORT_FIELD = "rtsp_port";
  static constexpr const char* CONFIG_INTERNAL_RTMP_PORT_FIELD = "rtmp_port";
  static constexpr const char* CONFIG_INTERNAL_ENC_FMT_FIELD = "enc_fmt";
  static constexpr const char* CONFIG_INTERNAL_PIX_FMT_FIELD = "pix_fmt";

 private:
  std::map<int, std::shared_ptr<Encoder>> mEncoderMap;
  bm_handle_t m_handle;
  std::map<int, std::string> mChannelOutputPath;
  enum class EncodeType { RTSP, RTMP, VIDEO, UNKNOWN };
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