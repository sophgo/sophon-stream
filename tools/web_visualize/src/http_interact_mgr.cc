#include "http_interact_mgr.h"

#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>

#include <nlohmann/json.hpp>

#include "http_struct_utils.h"

HTTP_Interact_Mgr::HTTP_Interact_Mgr() {}

HTTP_Interact_Mgr::~HTTP_Interact_Mgr() { stop(); }

void HTTP_Interact_Mgr::init(int port) {
  if (!server_.is_valid()) {
    return;
  }
  if (is_inited_) {
    if (port == port_)
      return;
    else
      stop();
  }

  port_ = port;
  server_.Get("/pipeline-list", HTTP_Interact_Mgr::handler);
  server_.Post("/run", HTTP_Interact_Mgr::handler);
  listen_thread_ = std::thread(&HTTP_Interact_Mgr::listen_thread);
  is_inited_ = true;
}

void HTTP_Interact_Mgr::stop() {
  server_.stop();
  listen_thread_.join();

  is_inited_ = false;
}

void HTTP_Interact_Mgr::handler(const httplib::Request& request,
                                httplib::Response& response) {
  HTTP_Interact_Mgr* mgr = HTTP_Interact_Mgr::GetInstance();
  std::string response_str;
  if (request.path == "/pipeline-list")
    response_str = mgr->handler_pipeline_list();
  if (request.path == "/run") response_str = mgr->handler_run(request.body);
  response.set_content(response_str, "application/json");
}

std::string HTTP_Interact_Mgr::handler_pipeline_list() {
  PipelineListResponse response;
  PipelineListResponseData data;
  data.id = 1;
  data.name = "yolox_bytetrack_osd_encode_demo.json";
  data.size = "1.5KB";
  data.created_at = "2023-06-15 10:30:15";
  response.data.push_back(data);

  response.status = "success";
  response.message = "get pipeline list success";

  nlohmann::json response_json;
  to_json(response_json, response);
  return response_json.dump();
}

std::string HTTP_Interact_Mgr::handler_run(const std::string& request_str) {
  RunRequest request;
  nlohmann::json request_json = nlohmann::json::parse(request_str);
  from_json(request_json, request);

  RunResponse response;
  RunResponseData data;
  data.status = "running";
  data.pipeline_id = 1;
  response.data = data;

  response.status = "success";
  response.message = "run pipeline success";

  nlohmann::json response_json;
  to_json(response_json, response);
  return response_json.dump();
}

HTTP_Interact_Mgr* HTTP_Interact_Mgr::GetInstance() {
  static HTTP_Interact_Mgr inst;
  return &inst;
}

void HTTP_Interact_Mgr::listen_thread() {
  HTTP_Interact_Mgr* mgr = HTTP_Interact_Mgr::GetInstance();
  mgr->server_.listen("0.0.0.0", mgr->port_);
  return;
}