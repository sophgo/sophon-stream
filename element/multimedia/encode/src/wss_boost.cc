#include "wss_boost.h"

namespace sophon_stream {
namespace element {
namespace encode {
bool WebSocketServer::Session::shouldExit_ = false;

void WebSocketServer::run() {
  do_accept();
  ioc_.run();
}

void WebSocketServer::pushImgDataQueue(const std::string& data) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (message_queue_.size() < MAX_WSS_QUEUE_LENGTH) {
    message_queue_.push(data);

    cv_.notify_all();
  }
}



void WebSocketServer::do_accept() {
  while (1) {
    tcp::socket socket(ioc_);

    acceptor_.accept(socket);

    auto session_ = std::make_shared<Session>(std::move(socket), message_queue_,
                                              mutex_, cv_, barrier_);
    session_->run();
  }
}

int WebSocketServer::getConnectionsNum() {
  return barrier_->get_thread_count();
}

}  // namespace encode
}  // namespace element
}  // namespace sophon_stream
