//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_FRAMEWORK_LISTEN_THREAD_H_
#define SOPHON_STREAM_FRAMEWORK_LISTEN_THREAD_H_

// #include <queue>
#include <thread>

#include "common/http_defs.h"
#include "httplib.h"
#include "nlohmann/json.hpp"

namespace sophon_stream {
namespace framework {

enum class RequestType { GET, PUT, POST, OPTIONS };

class ListenThread {
 public:
  ListenThread();
  ~ListenThread();
  void init(int port);
  void stop();

  static ListenThread* getInstance();

  void setHandler(const std::string& path, RequestType type, httplib::Server::Handler handler);

 private:
  httplib::Server server;
  int port_;
  std::thread listen_thread_;
  bool isRunning = true;
  //   std::queue<httplib::Request*> send_queue_;

  nlohmann::json handle_task_test(const std::string& json);

  static void handle_task_interact(const httplib::Request& request,
                                   httplib::Response& reponse);
  static void listen_loop();
};

}  // namespace framework
}  // namespace sophon_stream

#endif