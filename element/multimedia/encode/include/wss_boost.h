//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_ELEMENT_WSS_BOOST_H_
#define SOPHON_STREAM_ELEMENT_WSS_BOOST_H_

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <chrono>
#include <codecvt>
#include <condition_variable>
#include <fstream>
#include <locale>
#include <mutex>
#include <nlohmann/json.hpp>
#include <queue>
#include <string>
#include <thread>
#include <vector>

#include "common/logger.h"

namespace sophon_stream {
namespace element {
namespace encode {
namespace beast = boost::beast;
namespace http = beast::http;
namespace websocket = beast::websocket;
namespace net = boost::asio;
using tcp = net::ip::tcp;
using json = nlohmann::json;

class WebSocketServer {
 public:
  WebSocketServer(unsigned short port, int fps, int conns)
      : ioc_(),
        acceptor_(ioc_, tcp::endpoint(tcp::v4(), port)),
        fps_(fps),
        conns_(conns),
        strand_(net::make_strand(ioc_)),
        timer_(ioc_) {
  }

  void run();

  bool is_open();

  void destroy();

  void reconnect(int index);

  void pushImgDataQueue(const std::string& data);

  const int getConnectionsNum() const;

 private:
  void do_accept();

  void close_sessions();

  void send_frame();

  const int MAX_WSS_QUEUE_LENGTH = 5;

  class Session {
   public:
    Session(tcp::socket socket, std::mutex& mutex, std::condition_variable& cv)
        : ws_(std::move(socket)), mutex_(mutex), cv_(cv) {
      ws_.read_message_max(64 * 1024 * 1024);  // 64 MB
      //   ws_.write_buffer_size(64 * 1024 * 1024);  // 64 MB
    }

    ~Session() {
      while (!queue_.empty()) {
        queue_.pop();
      }
      std::cout << "Session released." << std::endl;
    }

    std::queue<std::string> queue_;  // 要发送的消息队列

    void run() {
      boost::system::error_code ec;
      ws_.accept(ec);
      if (!ec) {
        writeThread_ = std::thread(&Session::do_write, this);
        writeThread_.detach();
      }
    }

    void stop() {
      close();
      shouldExit_ = true;
    }

    void setCallback(std::function<void(int)> cb) { callback = cb; }

    void invokeCallback(int param) {
      callback(param);
    }

    bool is_open() { return ws_.is_open(); }

    void close() {
      boost::system::error_code ec;
      ws_.close(websocket::close_code::normal, ec);
    }

    bool should_des() {
      if (rflag && wflag) return true;
      return false;
    }

    void do_read() {
      while (is_open() && !shouldExit_) {
        boost::system::error_code ec;
        std::size_t bytes_transferred = ws_.read(buffer_, ec);
        if (ec == boost::system::errc::connection_reset) {
          if (is_open()) {
            stop();
          }
          break;
        } else if (ec) {
          std::cerr << "Error reading data: " << ec.message() << std::endl;
          if (is_open()) {
            stop();
          }
          break;
        }
      }
      rflag = true;
    }

    void do_write() {
      while (is_open() && !shouldExit_) {
        std::unique_lock<std::mutex> lock(mutex_);
        // cv_.wait(lock, [this] { return !queue_.empty(); });
        cv_.wait(lock, [this] { return !queue_.empty();});
        std::string message = queue_.front();
        queue_.pop();
        std::vector<uint8_t> binary_message(message.begin(), message.end());
        ws_.binary(true);
        try {
          ws_.write(net::buffer(binary_message));
        } catch (boost::wrapexcept<boost::system::system_error>& ex) {
          if (is_open()) {
            stop();
          }
          break;
        }
      }
      wflag = true;
    }

   private:
    /**
     * @brief 从message_queue_中取出消息，异步发送。发送完成后递归调用自身
     *
     */
    bool rflag = false;
    bool wflag = false;
    bool shouldExit_ = false;
    std::function<void(int)> callback;  // 回调函数指针
    std::thread readThread_, writeThread_;
    websocket::stream<tcp::socket> ws_;  // websocket会话的流对象
    beast::flat_buffer buffer_;          // 从客户端接收到的数据
    std::mutex& mutex_;
    std::condition_variable& cv_;
    bool writing_ = false;
    int index;
  };
  int num = 0;
  int conns_;
  net::io_context ioc_;  // 管理IO上下文
  tcp::acceptor acceptor_;  // 侦听传入的连接请求，创建新的tcp::socket
  int fps_;
  net::strand<net::io_context::executor_type>
      strand_;               // 提供串行处理机制，避免竞态
  net::steady_timer timer_;  // 定时操作
  std::queue<std::string> message_queue_;
  std::mutex mutex_;
  std::mutex ws_mutex;
  std::condition_variable cv_;
  // std::vector<std::shared_ptr<Session>> sessions_;
  std::vector<Session*> sessions_;
};

}  // namespace encode
}  // namespace element
}  // namespace sophon_stream

#endif
