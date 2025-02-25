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

#include "element_factory.h"
#include "encoder.h"
#include "websocketpp/base64/base64.hpp"
#include "wss.h"
#include "wss_boost.h"

namespace sophon_stream {
namespace element {
namespace encode {

class WSSManager {
 public:
  WSSManager() = default;
  ~WSSManager() = default;
  WSSManager(std::shared_ptr<WSS> ptr) : wsspp(ptr), wssboost(nullptr) {};
  WSSManager(std::shared_ptr<WebSocketServer> ptr)
      : wsspp(nullptr), wssboost(ptr) {};

  void pushImgDataQueue(const std::string& data) {
    if (wsspp) {
      wsspp->pushImgDataQueue(data);
    } else if (wssboost) {
      wssboost->pushImgDataQueue(data);
    }
    return;
  }

  const int getConnectionsNum() const {
    int n = 0;
    if (wsspp) {
      n = wsspp->getConnectionsNum();
    } else if (wssboost) {
      n = wssboost->getConnectionsNum();
    }
    return n;
  }

  std::shared_ptr<WSS> wsspp;
  std::shared_ptr<WebSocketServer> wssboost;
};

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
  static constexpr const char* CONFIG_INTERNAL_WSS_BACKEND = "wss_backend";
  static constexpr const char* CONFIG_INTERNAL_FPS_FIELD = "fps";

  // for customizing shape and ip
  static constexpr const char* CONFIG_INTERNAL_WIDTH_FIELD = "width";
  static constexpr const char* CONFIG_INTERNAL_HEIGHT_FIELD = "height";
  static constexpr const char* CONFIG_INTERNAL_WSENCTYPE_FIELD = "ws_enc_type";
  static constexpr const char* CONFIG_INTERNAL_IP_FIELD = "ip";
  static constexpr const char* CONFIG_INTERNAL_PREFIX = "prefix";

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

  int width = -1;
  int height = -1;

  enum class WSencType { IMG_ONLY, SERIALIZED };
  enum class WSSBackend { WEBSOCKETPP, BOOST };
  WSencType mWsEncType = WSencType::IMG_ONLY;
  WSSBackend mWssBackend = WSSBackend::WEBSOCKETPP;

  std::string ip = "localhost";
  std::string prefix = "";

  std::map<int, std::shared_ptr<WSSManager>> mWSSMap;
  //   std::map<int, std::shared_ptr<WSS>> mWSSMap;  // for websocketpp
  //   std::map<int, std::shared_ptr<WebSocketServer>>
  //       mWSSBoostMap;  // for websocket boost
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

  // 如果多decoder，各自连接到各自的encoder上，那么直接把encoder的profiler
  // resize成线程数会出错 所以这里使用一个unordered_map来代替
  // key: channelIdInternal
  std::unordered_map<unsigned int, std::shared_ptr<common::FpsProfiler>>
      mFpsProfilers;
};

}  // namespace encode
}  // namespace element
}  // namespace sophon_stream

#endif  // SOPHON_STREAM_ELEMENT_ENCODE_H_