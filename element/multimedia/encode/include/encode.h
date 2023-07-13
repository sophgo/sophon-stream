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
#include "common/profiler.h"
#include "element.h"
#include "encoder.h"
#include "wss.h"

namespace sophon_stream {
namespace element {
namespace encode {

class Encode : public ::sophon_stream::framework::Element {
 public:
  Encode();
  ~Encode() override;

  common::ErrorCode initInternal(const std::string& json) override;

  common::ErrorCode doWork(int dataPipeId) override;

  static constexpr const char* CONFIG_INTERNAL_ENCODE_TYPE_FIELD =
      "encode_type";
  static constexpr const char* CONFIG_INTERNAL_RTSP_PORT_FIELD = "rtsp_port";
  static constexpr const char* CONFIG_INTERNAL_RTMP_PORT_FIELD = "rtmp_port";
  static constexpr const char* CONFIG_INTERNAL_ENC_FMT_FIELD = "enc_fmt";
  static constexpr const char* CONFIG_INTERNAL_PIX_FMT_FIELD = "pix_fmt";
  static constexpr const char* CONFIG_INTERNAL_WSS_PORT_FIELD = "wss_port";
  static constexpr const char* CONFIG_INTERNAL_FPS_FIELD = "fps";

 private:
  std::map<int, std::shared_ptr<Encoder>> mEncoderMap;
  bm_handle_t m_handle;
  std::map<int, std::string> mChannelOutputPath;
  enum class EncodeType { RTSP, RTMP, VIDEO, IMG_DIR, WS, UNKNOWN };
  EncodeType mEncodeType;
  std::string mRtspPort;
  std::string mRtmpPort;
  std::string encFmt;
  std::string pixFmt;
  double mFps;

  std::map<int, std::shared_ptr<WSS>> mWSSMap;
  std::vector<std::thread> mWSSThreads;
  std::mutex mWSSThreadsMutex;
  std::string mWSSPort;

  // 处理RTSP、RTMP、VIDEO
  void processVideoStream(
      int dataPipeId, std::shared_ptr<common::ObjectMetadata> objectMetadata);
  // 处理IMG_DIR
  void processImgDir(int dataPipeId,
                     std::shared_ptr<common::ObjectMetadata> objectMetadata);
  // 处理WS
  void processWS(int dataPipeId,
                 std::shared_ptr<common::ObjectMetadata> objectMetadata);
  // WS发送停止标识
  void stopWS(int dataPipeId);

  ::sophon_stream::common::FpsProfiler mFpsProfiler;
};

}  // namespace encode
}  // namespace element
}  // namespace sophon_stream

#endif  // SOPHON_STREAM_ELEMENT_ENCODE_H_