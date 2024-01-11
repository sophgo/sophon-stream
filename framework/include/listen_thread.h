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

#include <mutex>
#include <queue>
#include <thread>

#include "common/error_code.h"
#include "common/http_defs.h"
#include "common/profiler.h"
#include "httplib.h"
#include "nlohmann/json.hpp"
namespace sophon_stream {
namespace framework {

enum class RequestType { GET, PUT, POST, OPTIONS };
struct http_config {
  std::string ip = "0.0.0.0";
  int port = 8000;
  std::string path = "/task/test";
};
class ReportImpl_ {
 public:
  ReportImpl_(std::string& ip, int port, std::string path, int channel);
  bool pushQueue(std::shared_ptr<nlohmann::json> j);
  void release();

 private:
  std::queue<std::shared_ptr<nlohmann::json>> objQueue;
  std::thread workThread;
  void postFunc();
  bool isRunning = true;
  std::shared_ptr<nlohmann::json> popQueue();
  size_t getQueueSize();
  std::mutex mtx;
  constexpr static int maxQueueLen = 20;

  httplib::Client cli;
  std::string path;
};
class ListenThread {
 public:
  ListenThread();
  ~ListenThread();
  void init(const nlohmann::json& report_json,
            const nlohmann::json& listen_json);
  void stop();

  static ListenThread* getInstance();

  void setHandler(const std::string& path, RequestType type,
                  httplib::Server::Handler handler);

  bool pushQueue(std::shared_ptr<nlohmann::json> j);

  void report_status(common::ErrorCode errorcode);
  static constexpr const char* JSON_IP_FILED = "ip";
  static constexpr const char* JSON_PORT_FILED = "port";
  static constexpr const char* JSON_PATH_FILED = "path";

 private:
  httplib::Server server;
  std::shared_ptr<ReportImpl_> client;
  http_config report_config;
  http_config listen_config;
  std::thread listen_thread_;
  bool isRunning = true;
  bool if_report_ = false;
  //   std::queue<httplib::Request*> send_queue_;

  nlohmann::json handle_task_test(const std::string& json);

  static void handle_task_interact(const httplib::Request& request,
                                   httplib::Response& reponse);
  static void listen_loop();
};

}  // namespace framework
}  // namespace sophon_stream

#endif