//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_ELEMENT_MULTIMEDIA_DECODE_CHANNEL_H_
#define SOPHON_STREAM_ELEMENT_MULTIMEDIA_DECODE_CHANNEL_H_

#include <string>
#include <vector>

#include "common/error_code.h"

namespace sophon_stream {
namespace element {
namespace decode {

struct ChannelOperateRequest {
  enum class ChannelOperate {
    START,
    STOP,
    PAUSE,
    RESUME,
  };
  enum class SampleStrategy {
    DROP,
    SAVE,
  };
  enum class SourceType { RTSP, RTMP, VIDEO, IMG_DIR, BASE64, UNKNOWN };
  int channelId;
  int loopNum;
  std::string url;
  SourceType sourceType;
  ChannelOperate operation;
  std::string json;
  double fps;
  int sampleInterval;
  int base64Port;
  std::vector<int> skip_element;
  SampleStrategy sampleStrategy;
};

struct ChannelOperateResponse {
  common::ErrorCode errorCode;
  std::string errorInfo;
};

struct ChannelTask {
  ChannelOperateRequest request;
  ChannelOperateResponse response;
};

}  // namespace decode
}  // namespace element
}  // namespace sophon_stream

#endif  // SOPHON_STREAM_ELEMENT_MULTIMEDIA_DECODE_CHANNEL_H_