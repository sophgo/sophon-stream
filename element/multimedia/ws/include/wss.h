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
namespace ws {

typedef websocketpp::server<websocketpp::config::asio> server;
using websocketpp::connection_hdl;
using websocketpp::lib::bind;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
typedef server::message_ptr message_ptr;
typedef std::set<connection_hdl, std::owner_less<connection_hdl>> con_list;

class WSS {
 public:
  WSS() {
    m_fps = 25;
    m_frame_interval = 1 / m_fps * 1000;  // ms
    m_last_send_time.tv_sec = 0;
    m_last_send_time.tv_usec = 0;
  }
  ~WSS() {}

  void on_open(connection_hdl hdl) { m_connections.insert(hdl); }

  void on_close(connection_hdl hdl) { m_connections.erase(hdl); }

  void init(int port) {
    try {
      // Set logging settings
      m_server.set_access_channels(websocketpp::log::alevel::all);
      m_server.clear_access_channels(websocketpp::log::alevel::frame_payload);

      // Initialize Asio
      m_server.init_asio();

      // m_server.set_message_handler(bind(&on_message,&mServer,::_1,::_2));
      m_server.set_open_handler(bind(&WSS::on_open, this, _1));
      m_server.set_close_handler(bind(&WSS::on_close, this, _1));

      m_server.listen(port);

      // Start the server accept loop
      m_server.start_accept();

      // Start the ASIO io_service run loop
      m_server.run();
    } catch (websocketpp::exception const& e) {
      IVS_ERROR("wss init error: {}", e.what());
    } catch (...) {
      IVS_ERROR("wss init other error");
    }
  }

  void send(const std::string& data) {
    gettimeofday(&m_current_send_time, NULL);
    double time_delta =
        1000 *
        ((m_current_send_time.tv_sec - m_last_send_time.tv_sec) +
         (double)(m_current_send_time.tv_usec - m_last_send_time.tv_usec) /
             1000000.0);
    IVS_DEBUG("wss send time_delta: {}ms", time_delta);
    int time_to_sleep = m_frame_interval - time_delta;
    if (time_to_sleep > 0)
      std::this_thread::sleep_for(std::chrono::milliseconds(time_to_sleep));
    gettimeofday(&m_last_send_time, NULL);
    for (auto it : m_connections) {
      m_server.send(it, data, websocketpp::frame::opcode::text);
    }
  }

  void stop() { m_server.stop_listening(); }

 private:
  server m_server;
  con_list m_connections;
  struct timeval m_last_send_time;
  struct timeval m_current_send_time;
  double m_fps;
  double m_frame_interval;
};

}  // namespace ws
}  // namespace element
}  // namespace sophon_stream

#endif  // SOPHON_STREAM_ELEMENT_WSS_H_