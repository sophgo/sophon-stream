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
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
  HttpPushImpl_(std::string& scheme, std::string& ip, int port, std::string cert, std::string key,
		  std::string cacert_, bool verify_, std::string path_, int channel);
#else
  HttpPushImpl_(std::string& ip, int port, std::string path, int channel);
#endif
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
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
  std::string scheme;
  std::string cacert;
  std::string key;
  bool verify;
#endif
  std::string path;

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
  static constexpr const char* CONFIG_INTERNAL_PATH_FILED = "path";
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
  static constexpr const char* CONFIG_INTERNAL_SCHEME_FILED = "scheme";
  static constexpr const char* CONFIG_INTERNAL_CERT_FILED = "cert";
  static constexpr const char* CONFIG_INTERNAL_KEY_FILED = "key";
  static constexpr const char* CONFIG_INTERNAL_CACERT_FILED = "cacert";
  static constexpr const char* CONFIG_INTERNAL_VERIFY_FILED = "verify";
#endif

 private:
  std::unordered_map<int, std::shared_ptr<HttpPushImpl_>> mapImpl_;
  std::mutex mapMtx;
  std::string ip_;
  int port_;
  std::string path_;
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
  std::string scheme_;
  std::string cert_;
  std::string key_;
  std::string cacert_;
  bool verify_;
#endif
};

}  // namespace http_push
}  // namespace element
}  // namespace sophon_stream

#endif
