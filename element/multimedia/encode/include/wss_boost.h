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

class FlexibleBarrier {
 public:
  using Callback = std::function<void()>;  // 定义回调类型

  FlexibleBarrier(
      int count, Callback callback = [] {})
      : thread_count(count), count_to_wait(count), on_completion(callback) {}

  void arrive_and_wait() {
    std::unique_lock<std::mutex> lock(mtx);
    --count_to_wait;

    if (count_to_wait == 0) {
      // 执行回调函数
      on_completion();

      count_to_wait = thread_count;  // 重置等待计数
      lock.unlock();
      cv.notify_all();  // 唤醒所有等待线程
    } else {
      cv.wait(lock);
    }
  }

  void add_thread() {
    std::lock_guard<std::mutex> lock(mtx);
    ++thread_count;
    ++count_to_wait;
  }
  void del_thread() {
    std::lock_guard<std::mutex> lock(mtx);
    --thread_count;
    --count_to_wait;
  }
  // 允许在运行时更改回调函数
  void set_on_completion(Callback callback) {
    std::lock_guard<std::mutex> lock(mtx);
    on_completion = callback;
  }
  int get_thread_count() {
    std::lock_guard<std::mutex> lock(mtx);
    return thread_count;
  }

 private:
  std::mutex mtx;
  std::condition_variable cv;
  int thread_count;        // 总线程数
  int count_to_wait;       // 当前需要等待的线程数
  Callback on_completion;  // 完成时的回调函数
};

class WebSocketServer {
 public:
  WebSocketServer(unsigned short port, int fps)
      : ioc_(),
        acceptor_(ioc_, tcp::endpoint(tcp::v4(), port)),
        fps_(fps),
        strand_(net::make_strand(ioc_)),
        timer_(ioc_) {
    barrier_ = std::make_shared<FlexibleBarrier>(0, [this]() {
      std::unique_lock<std::mutex> lock(mutex_);
      message_queue_.pop();
    });
  }

  void run();

  bool is_open();

  void reconnect();

  void pushImgDataQueue(const std::string& data);
  int getConnectionsNum();

 private:
  void do_accept();

  void close_sessions();

  void send_frame();

  const int MAX_WSS_QUEUE_LENGTH = 5;

  class Session : public std::enable_shared_from_this<Session> {
   public:
    Session(tcp::socket socket, std::queue<std::string>& message_queue,
            std::mutex& mutex, std::condition_variable& cv,
            std::shared_ptr<FlexibleBarrier>& barrier)
        : ws_(std::move(socket)),
          message_queue_(message_queue),
          mutex_(mutex),
          cv_(cv),
          barrier_(barrier) {
      ws_.read_message_max(64 * 1024 * 1024);  // 64 MB
      //   ws_.write_buffer_size(64 * 1024 * 1024);  // 64 MB
    }
    Session(const Session&) = delete;
    Session& operator=(const Session&) = delete;
    ~Session() {
      
    }

    void run() {
      boost::system::error_code ec;
      ws_.accept(ec);
      if (!ec) {
        do_write();
      }
    }

    bool is_open() { return ws_.is_open(); }

    void close() {
      boost::system::error_code ec;
      ws_.close(websocket::close_code::normal, ec);
    }

    void do_write() {
      barrier_->add_thread();
      while (true) {
        std::string message;
        {
          std::unique_lock<std::mutex> lock(mutex_);
          cv_.wait(lock, [this] { return !message_queue_.empty(); });

          message = message_queue_.front();
        }

        barrier_->arrive_and_wait();

        ws_.text(true);

        boost::system::error_code ec;
        // Synchronously write the message to the WebSocket
        ws_.write(net::buffer(message), ec);

        // Check if the operation succeeded
        if (!ec) {
 
        } else {
          barrier_->del_thread();
          close();
          break;
        }
      }
    }

   private:
    /**
     * @brief 从message_queue_中取出消息，异步发送。发送完成后递归调用自身
     *
     */
    std::thread writeThread_;
    websocket::stream<tcp::socket> ws_;       // websocket会话的流对象
    beast::flat_buffer buffer_;               // 从客户端接收到的数据
    std::queue<std::string>& message_queue_;  // 要发送的消息队列
    std::mutex& mutex_;
    std::condition_variable& cv_;
    std::shared_ptr<FlexibleBarrier> barrier_;  // std::thread writeThread_;
  };

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
  std::vector<std::shared_ptr<Session>> sessions_;
  std::shared_ptr<FlexibleBarrier> barrier_;
};

}  // namespace encode
}  // namespace element
}  // namespace sophon_stream

#endif
