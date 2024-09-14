//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_ELEMENT_WSS_H_
#define SOPHON_STREAM_ELEMENT_WSS_H_

#include <sys/time.h>

#include <chrono>
#include <iostream>
#include <memory>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

#include "common/object_metadata.h"
#include "element.h"

namespace sophon_stream {
namespace element {
namespace encode {

typedef websocketpp::server<websocketpp::config::asio> server;
using websocketpp::connection_hdl;
using websocketpp::lib::bind;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
typedef server::message_ptr message_ptr;
typedef std::set<connection_hdl, std::owner_less<connection_hdl>> con_list;

static const std::string WS_STOP_FLAG = "ws_stop_flag";

class WSS {
 public:
  WSS();

  ~WSS();

  void on_open(connection_hdl hdl);

  void on_close(connection_hdl hdl);

  void init(int port, double fps);

  // 从队列中取数据发送
  void send();

  void stop();

  void pushImgDataQueue(const std::string& data);

  std::string popImgDataQueue();
  uint getConnectionsNum();

 private:
  server m_server;
  con_list m_connections;
  struct timeval m_last_send_time;
  struct timeval m_current_send_time;
  double m_fps;
  double m_frame_interval;
  std::queue<std::string> mImgDataQueue;
  std::mutex mQueueMtx;

  static constexpr int WSS_MAX_QUEUE_LEN = 5;
};

}  // namespace encode
}  // namespace element
}  // namespace sophon_stream

#endif  // SOPHON_STREAM_ELEMENT_WSS_H_