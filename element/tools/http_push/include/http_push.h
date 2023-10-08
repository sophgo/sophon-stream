//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_ELEMENT_HTTP_PUSH_H_
#define SOPHON_STREAM_ELEMENT_HTTP_PUSH_H_

#include <mutex>
#include <nlohmann/json.hpp>
#include <queue>

#include "common/object_metadata.h"
#include "common/profiler.h"
#include "element.h"
#include "httplib.h"

namespace sophon_stream {
namespace element {
namespace http_push {

class HttpPushImpl_ {
 public:
  HttpPushImpl_(std::string& ip, int port, int channel);
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
  std::string path = "/flask_test";

  std::string mFpsProfilerName;
  ::sophon_stream::common::FpsProfiler mFpsProfiler;
};

class HttpPush : public ::sophon_stream::framework::Element {
 public:
  HttpPush();
  ~HttpPush() override;

  common::ErrorCode initInternal(const std::string& json) override;

  common::ErrorCode doWork(int dataPipeId) override;

  static constexpr const char* CONFIG_INTERNAL_IP_FILED = "ip";
  static constexpr const char* CONFIG_INTERNAL_PORT_FILED = "port";

 private:
  std::unordered_map<int, std::shared_ptr<HttpPushImpl_>> mapImpl_;
  std::mutex mapMtx;
  std::string ip_;
  int port_;
};

}  // namespace http_push
}  // namespace element
}  // namespace sophon_stream

#endif