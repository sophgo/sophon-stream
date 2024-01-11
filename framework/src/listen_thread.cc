//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#include "listen_thread.h"

#include "common/logger.h"

namespace sophon_stream {
namespace framework {
ReportImpl_::ReportImpl_(std::string& ip, int port, std::string path_,
                         int channel)
    : cli(ip, port), path(path_) {
  workThread = std::thread(&ReportImpl_::postFunc, this);
}

void ReportImpl_::release() {
  isRunning = false;
  workThread.join();
}

void ReportImpl_::postFunc() {
  while (isRunning) {
    auto ptr = popQueue();
    if (ptr == nullptr) {
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
      continue;
    }
    int retry_times = 5;
    while (retry_times--) {
      auto res = cli.Post(path.c_str(), ptr->dump(), "application/json");
      std::string err_str = to_string(res.error());
      if (err_str != "Success (no error)")
        IVS_ERROR("Report error:{0}", err_str);
      else
        break;
    }
  }
}

bool ReportImpl_::pushQueue(std::shared_ptr<nlohmann::json> j) {
  int len = getQueueSize();
  if (len >= maxQueueLen) return false;
  {
    std::lock_guard<std::mutex> lock(mtx);
    objQueue.push(j);
  }
  return true;
}

std::shared_ptr<nlohmann::json> ReportImpl_::popQueue() {
  std::lock_guard<std::mutex> lock(mtx);
  std::shared_ptr<nlohmann::json> j = nullptr;
  if (objQueue.empty()) return j;
  j = objQueue.front();
  objQueue.pop();
  return j;
}

size_t ReportImpl_::getQueueSize() {
  int len = -1;
  {
    std::lock_guard<std::mutex> lock(mtx);
    len = objQueue.size();
  }
  return len;
}

ListenThread* ListenThread::getInstance() {
  static ListenThread inst;
  return &inst;
}

ListenThread::ListenThread() {}

ListenThread::~ListenThread() {
  if (isRunning) {
    stop();
  }
  if (if_report_) {
    IVS_INFO("Start to Stop Report Thread... Path is {0}:{1}{2}",
             report_config.ip, report_config.port, report_config.path);
    client->release();
    IVS_INFO("Complete to Stop Report Thread... Path is {0}:{1}{2}",
             report_config.ip, report_config.port, report_config.path);
  }
}

void ListenThread::init(const nlohmann::json& report_json,
                        const nlohmann::json& listen_json) {
  if (report_json.contains(JSON_IP_FILED) &&
      report_json.contains(JSON_PORT_FILED) &&
      report_json.contains(JSON_PATH_FILED)) {
    if_report_ = true;

    report_config.port = report_json.find(JSON_PORT_FILED)->get<int>();
    report_config.ip = report_json.find(JSON_IP_FILED)->get<std::string>();
    report_config.path = report_json.find(JSON_PATH_FILED)->get<std::string>();
  }
  if (listen_json.contains(JSON_PORT_FILED)) {
    listen_config.port = listen_json.find(JSON_PORT_FILED)->get<int>();
  }
  if (listen_json.contains(JSON_IP_FILED)) {
    listen_config.ip = listen_json.find(JSON_IP_FILED)->get<std::string>();
  }
  if (listen_json.contains(JSON_PATH_FILED)) {
    listen_config.path = listen_json.find(JSON_PATH_FILED)->get<std::string>();
  }
  IVS_INFO("Start to Init Listen Thread... Path is {0}:{1}{2}",
           listen_config.ip, listen_config.port, listen_config.path);
  if (!server.is_valid()) {
    IVS_ERROR("HttpServer has Error!");
    abort();
  }
  if (if_report_) {
    IVS_INFO("Start to Init Report Thread... Path is {0}:{1}{2}",
             report_config.ip, report_config.port, report_config.path);
    client = std::make_shared<ReportImpl_>(report_config.ip, report_config.port,
                                           report_config.path, 0);
    IVS_INFO("Complete to Init Report Thread... Path is {0}:{1}{2}",
             report_config.ip, report_config.port, report_config.path);
  }

  
  listen_thread_ = std::thread(&ListenThread::listen_loop);
  IVS_INFO("Complete to Init Listen Thread... Path is {0}:{1}{2}",
           listen_config.ip, listen_config.port, listen_config.path);
}
bool ListenThread::pushQueue(std::shared_ptr<nlohmann::json> j) {
  return client->pushQueue(j);
}
void ListenThread::stop() {
  IVS_INFO("Start to Stop Listen Thread... Path is {0}:{1}{2}",
           listen_config.ip, listen_config.port, listen_config.path);
  server.stop();
  isRunning = false;
  listen_thread_.join();
  IVS_INFO("Complete to Stop Listen Thread... Path is {0}:{1}{2}",
           listen_config.ip, listen_config.port, listen_config.path);
}

void ListenThread::setHandler(const std::string& path, RequestType type,
                              httplib::Server::Handler handler) {
  switch (type) {
    case RequestType::GET:
      server.Get(path, handler);
      server.Options(path, handler);
      break;
    case RequestType::PUT:
      server.Put(path, handler);
      server.Options(path, handler);
      break;
    case RequestType::POST:
      server.Post(path, handler);
      server.Options(path, handler);
      break;
    default:
      IVS_ERROR("RequestType is not supported!");
      abort();
  }
  // server.Post(path, handler);
  IVS_INFO("Regist handler, path is {0}", path);
}

void ListenThread::listen_loop() {
  ListenThread* pthis = ListenThread::getInstance();
  IVS_INFO("Start to Listen, Path is {0}:{1}{2}", pthis->listen_config.ip,
           pthis->listen_config.port, pthis->listen_config.path);
  pthis->server.listen(pthis->listen_config.ip, pthis->listen_config.port);
}

nlohmann::json ListenThread::handle_task_test(const std::string& json) {
  common::Response resp;
  std::vector<common::Result> results{{0, "0"}, {1, "1"}};
  resp.code = 0;
  resp.msg = "success";
  resp.results = results;
  nlohmann::json j = resp;
  return j.dump();
}

void ListenThread::handle_task_interact(const httplib::Request& request,
                                        httplib::Response& response) {
  ListenThread* pthis = ListenThread::getInstance();
  std::string str_ret;
  str_ret = pthis->handle_task_test(request.body);
  IVS_INFO("List Return: {0}", str_ret);
  response.set_content(str_ret, "application/json");
}

void ListenThread::report_status(common::ErrorCode errorcode) {
  if (!if_report_) return;
  std::shared_ptr<nlohmann::json> j_patch =
      std::make_shared<nlohmann::json>(R"({
        "error": "config not good"
    })"_json);

  std::string error = common::ErrorCodeToString(errorcode);
  (*j_patch)["error"] = error;
  pushQueue(j_patch);
  return;
}
}  // namespace framework
}  // namespace sophon_stream