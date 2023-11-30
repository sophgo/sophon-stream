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

ListenThread* ListenThread::getInstance() {
  static ListenThread inst;
  return &inst;
}

ListenThread::ListenThread() {}

ListenThread::~ListenThread() {
  if (isRunning) {
    stop();
  }
}

void ListenThread::init(int port) {
  IVS_INFO("Start to Init Listen Thread... Port is {0}", port);
  if (!server.is_valid()) {
    IVS_ERROR("HttpServer has Error!");
    abort();
  }
  port_ = port;
  server.Post("/task/test", ListenThread::handle_task_interact);
  listen_thread_ = std::thread(&ListenThread::listen_loop);
  IVS_INFO("Complete to Init Listen Thread... Port is {0}", port);
}

void ListenThread::stop() {
  IVS_INFO("Start to Stop Listen Thread... Port is {0}", port_);
  server.stop();
  isRunning = false;
  listen_thread_.join();
  IVS_INFO("Complete to Stop Listen Thread... Port is {0}", port_);
}

void ListenThread::setHandler(const std::string& path,
                              httplib::Server::Handler handler) {
  server.Post(path, handler);
  IVS_INFO("Regist handler, path is {0}", path);
}

void ListenThread::listen_loop() {
  ListenThread* pthis = ListenThread::getInstance();
  IVS_INFO("Start to Listen, Port is {0}", pthis->port_);
  pthis->server.listen("0.0.0.0", pthis->port_);
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
  if (request.path == "/task/test") {
    str_ret = pthis->handle_task_test(request.body);
    IVS_INFO("List Return: {0}", str_ret);
  } else {
  }
  response.set_content(str_ret, "application/json");
}

}  // namespace framework
}  // namespace sophon_stream