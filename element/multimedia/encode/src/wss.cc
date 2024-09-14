#include "wss.h"

namespace sophon_stream {
namespace element {
namespace encode {

WSS::WSS() {}

WSS::~WSS() {}

void WSS::on_open(connection_hdl hdl) { m_connections.insert(hdl); }

void WSS::on_close(connection_hdl hdl) { m_connections.erase(hdl); }

void WSS::init(int port, double fps) {
  try {
    // Get fps
    m_fps = fps;
    m_frame_interval = 1 / m_fps * 1000;  // ms
    m_last_send_time = {0, 0};

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

// 从队列中取数据发送
void WSS::send() {
  while (1) {
    // 队列为空，停止5ms
    if (mImgDataQueue.empty()) {
      std::this_thread::sleep_for(std::chrono::milliseconds(5));
      continue;
    }
    auto data = popImgDataQueue();
    if (WS_STOP_FLAG == data) {
      IVS_DEBUG(
          "WSS recieve flag: {0}, demo will stop after closing the browser",
          WS_STOP_FLAG);
      stop();
      break;
    }
    gettimeofday(&m_current_send_time, NULL);
    double time_delta =
        1000 *
        ((m_current_send_time.tv_sec - m_last_send_time.tv_sec) +
         (double)(m_current_send_time.tv_usec - m_last_send_time.tv_usec) /
             1000000.0);
    int time_to_sleep = m_frame_interval - time_delta;
    if (time_to_sleep > 0)
      std::this_thread::sleep_for(std::chrono::milliseconds(time_to_sleep));
    gettimeofday(&m_last_send_time, NULL);
    for (auto it : m_connections) {
      m_server.send(it, data, websocketpp::frame::opcode::text);
    }
  }
}

uint WSS::getConnectionsNum() {
    return m_connections.size();
}

void WSS::stop() { m_server.stop_listening(); }

void WSS::pushImgDataQueue(const std::string& data) {
  std::lock_guard<std::mutex> lock(mQueueMtx);
  if (mImgDataQueue.size() < WSS_MAX_QUEUE_LEN)
    mImgDataQueue.push(data);
}

std::string WSS::popImgDataQueue() {
  IVS_DEBUG("WSS::popImgDataQueue, queue size: {}", mImgDataQueue.size());
  std::lock_guard<std::mutex> lock(mQueueMtx);
  auto data = mImgDataQueue.front();
  mImgDataQueue.pop();
  return data;
}

}  // namespace encode
}  // namespace element
}  // namespace sophon_stream