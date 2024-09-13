#include "wss_boost.h"

namespace sophon_stream {
namespace element {
namespace encode {

int idx = 0;

void WebSocketServer::run() {
  do_accept();
  ioc_.run();
}

void WebSocketServer::pushImgDataQueue(const std::string& data) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (message_queue_.size() < MAX_WSS_QUEUE_LENGTH) {
    for (auto& session_ : sessions_) {
      if (session_->is_open()) {
        session_->queue_.push(data);
      }
    }
    cv_.notify_all();
  }
}

void WebSocketServer::do_accept() {
  int num = 0;
  while (1) {
    tcp::socket socket(ioc_);
    acceptor_.accept(socket);
    if (sessions_.size() == 4) {
      cv_.notify_all();
      for (int i = 0; i < 4; ++i) {
        delete sessions_[i];
        sessions_[i] = nullptr;
      }
      sessions_.clear();
    }

    auto session_ = new Session(std::move(socket), mutex_, cv_);
    sessions_.push_back(session_);
    sessions_.back()->run();
    std::cout << sessions_.size() << std::endl;
  }
}

const int WebSocketServer::getConnectionsNum() const {
  return sessions_.size();
}

void WebSocketServer::reconnect(int index) {
  // std::cout << sessions_.back().use_count()<<std::endl;
  // for(int i = 0; i < 2; ++i) {
  //   if(sessions_[i]->is_open())
  //     sessions_[i]->close();
  //     delete sessions_[i];
  //     sessions_[i] = nullptr;
  // }
  // ioc_.restart();
  std::cout << index << std::endl;
  if (sessions_[index]->is_open()) sessions_[index]->close();
  sessions_[index] = nullptr;
}
}  // namespace encode
}  // namespace element
}  // namespace sophon_stream
