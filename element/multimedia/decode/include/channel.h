#pragma once

#include <string>

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
  enum class SourceType { RTSP, RTMP, VIDEO, IMG_DIR, UNKNOWN };
  int channelId;
  std::string url;
  SourceType sourceType;
  ChannelOperate operation;
  std::string json;
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