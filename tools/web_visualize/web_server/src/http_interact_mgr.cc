//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#include "http_interact_mgr.h"

#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>

#include <nlohmann/json.hpp>
#include <regex>
#include <vector>

#include "common/error_code.h"
#include "common/logger.h"
#include "element_factory.h"
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
  pipelines_name_list = {"yolov5",
                         "yolox",
                         "resnet",
                         "bytetrack",
                         "yolov5_bytetrack_distributor_resnet_converger",
                         "yolox_bytetrack_osd_encode"};
  port_ = port;
  // URL处理,对不同方法的URL路由，传入到handler
  server_.Options("/jsons/.*", HTTP_Interact_Mgr::handler);
  server_.Options("/pipelines/.*", HTTP_Interact_Mgr::handler);

  server_.Get("/pipelines.*", HTTP_Interact_Mgr::handler);
  server_.Put("/pipelines/.*", HTTP_Interact_Mgr::handler);
  server_.Patch("/pipelines/.*", HTTP_Interact_Mgr::handler);

  server_.Get("/jsons.*", HTTP_Interact_Mgr::handler);
  server_.Put("/jsons/.*", HTTP_Interact_Mgr::handler);
  server_.Patch("/jsons/.*", HTTP_Interact_Mgr::handler);

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
  std::regex pattern_pipelines("/pipelines/(\\d+).*");
  std::regex pattern_jsons("/jsons/(\\d+).*");
  std::smatch matches_pattern_pipelines;
  std::smatch matches_pattern_jsons;
  std::string pipelines_field_param;
  std::string jsons_field_param;
  std::string request_path_tmp;
  if (request.method == "OPTIONS") {
    IVS_INFO("调用OPTIONS方法，将直接返回！");
  } else if (request.method == "GET" && request.path == "/pipelines") {
    IVS_INFO(
        "解析URL... 获取到主要参数为: pipelines , 任务为：获取pipelines "
        "list！");
    response_str =
        mgr->handler_get_pipelines();  // 处理/pipelines，得到/pipelines的列表单
  } else if (std::regex_match(request.path, matches_pattern_pipelines,
                              pattern_pipelines)) {
    // 处理pipelines，正则匹配对应的URL后缀
    IVS_INFO(
        "解析URL... 获取到主要参数为: pipelines ! 匹配到pipeline id为: {0}",
        matches_pattern_pipelines[1].str());
    request_path_tmp = "/pipelines/" + matches_pattern_pipelines[1].str();
    for (std::multimap<std::string, std::string>::const_iterator it =
             request.params.begin();
         it != request.params.end(); ++it) {
      std::string params = it->first + " = " + it->second;
      pipelines_field_param = it->second;
    }
    if (request.method == "GET" && pipelines_field_param != "") {
      response_str = mgr->handler_get_pipelines_id_field(
          request.body);  // 处理带field参数的URL
    } else if (request.method == "GET" && pipelines_field_param == "") {
      response_str = mgr->handler_get_pipelines_id(
          matches_pattern_pipelines[1].str());  // 处理/pipelines/{pipelines_id}
    } else if (request.method == "PUT" && request.path == request_path_tmp) {
      response_str = mgr->handler_put_pipelines_id();
    } else if (request.method == "PATCH" && request.path == request_path_tmp) {
      response_str = mgr->handler_patch_pipelines_id(
          request.body, matches_pattern_pipelines[1].str());
    } else {
    }
  } else if (request.method == "GET" && request.path == "/jsons") {
    IVS_INFO("解析URL... 获取到主要参数为: jsons ， 任务为：获取jsons list！");
    response_str =
        mgr->handler_get_jsons();  // 处理/jsons，得到的是"/jsons的列表单
  } else if (
      std::regex_match(
          request.path, matches_pattern_jsons,
          pattern_jsons)) {  // 处理jsons相关内容，正则匹配对应的URL后缀等
    IVS_INFO("解析URL... 获取到主要参数为: jsons ! 匹配到json id为:  {0}",
             matches_pattern_jsons[1].str());
    request_path_tmp = "/jsons/" + matches_pattern_jsons[1].str();
    for (std::multimap<std::string, std::string>::const_iterator it =
             request.params.begin();
         it != request.params.end(); ++it) {
      std::string params = it->first + "=" + it->second;
      jsons_field_param = it->second;
    }
    if (request.method == "GET" && jsons_field_param != "") {
      response_str = mgr->handler_get_jsons_id_field();  // 处理带field参数的URL
    } else if (request.method == "GET" && jsons_field_param == "") {
      response_str = mgr->handler_get_jsons_id(
          matches_pattern_jsons[1].str());  // 处理/jsons/{jsons_id}
    } else if (request.method == "PUT" && request.path == request_path_tmp) {
      response_str = mgr->handler_put_jsons_id(request.body,
                                               matches_pattern_jsons[1].str());
    } else if (request.method == "PATCH" && request.path == request_path_tmp) {
      response_str = mgr->handler_patch_jsons_id();
    } else {
    }
  } else {
    IVS_INFO("调用方法错误，错误信息已返回！！");
  }
  response.set_header("Access-Control-Allow-Origin", "*");
  response.set_header("Access-Control-Allow-Methods",
                      "GET, PATCH, PUT, OPTIONS");
  response.set_header("Access-Control-Allow-Headers",
                      "Content-Type, Authorization");
  response.set_content(response_str, "application/json");
}

// 前端请求获取pipeline的list信息
std::string HTTP_Interact_Mgr::handler_get_pipelines() {
  IVS_INFO("正在调用 get pipelines 方法... 将获取到所有 pipeline 的信息");
  PipelineResponse response;
  PipelineResponseData responsedata;
  ErrorResponse error_response;
  nlohmann::json response_json;
  for (int i = 0; i < 6; i++) {
    responsedata = read_json_preview(std::to_string(i));
    if (responsedata.pipeline_name == "") {
      error_response.status = "error";
      error_response.message = "无法获取下拉框选项！请检查get pipelines方法！";
      error_response.error_code = 500;
      to_json(response_json, error_response);
      IVS_INFO(
          "ERROR! 发生错误！ 无法获取下拉框选项！请检查get pipelines方法！  ");
      return response_json.dump();
    }
    response.data.push_back(responsedata);
  }
  response.status = "success";
  response.message = "get pipeline list success";
  to_json(response_json, response);
  return response_json.dump();
}

// 前端请求,获取pipeline的全部区域信息
std::string HTTP_Interact_Mgr::handler_get_pipelines_id(
    const std::string& pipeline_id) {
  IVS_INFO(
      "正在调用 get pipeline id 方法... 需要调用 get 方法的 pipeline id 为:   "
      "{0}",
      pipeline_id);
  PipelineResponse response;
  PipelineResponseData responsedata;
  ErrorResponse error_response;
  nlohmann::json response_json;
  if (std::stoi(pipeline_id) < 0 || std::stoi(pipeline_id) > 5) {
    error_response.status = "error";
    error_response.message = "无法获取下拉框选项, get方法的pipeline_id有误！";
    error_response.error_code = 500;
    to_json(response_json, error_response);
    IVS_INFO(
        " ERROR! 发生错误！ 无法获取下拉框选项, get方法的pipeline_id有误！  ");
    return response_json.dump();
  }
  responsedata = read_json_preview(pipeline_id);
  if (responsedata.pipeline_name == "") {
    error_response.status = "error";
    error_response.message =
        "无法获取下拉框选项，get方法的pipeline_id对应的pipeline内容为空!";
    error_response.error_code = 500;
    to_json(response_json, error_response);
    IVS_INFO(
        " ERROR! "
        "无法获取下拉框选项，get方法的pipeline_id对应的pipeline内容为空!  ");
    return response_json.dump();
  }
  response.data.push_back(responsedata);
  response.status = "success";
  response.message = "get pipeline id content success";
  to_json(response_json, response);
  return response_json.dump();
}

// 前端请求,对pipeline的指定参数进行操作
std::string HTTP_Interact_Mgr::handler_patch_pipelines_id(
    const std::string& request_str, const std::string& pipeline_id) {
  IVS_INFO(
      "正在调用 patch pipeline id 方法... 需要调用 patch 方法的 pipeline id "
      "为:   {0}",
      pipeline_id);
  PipelineRequest request;
  PipelineResponseData pipelineresponsedata;
  PipelineResponse pipelineresponse;
  nlohmann::json response_json;
  nlohmann::json request_json = nlohmann::json::parse(request_str);
  ErrorResponse error_response;
  if (std::stoi(pipeline_id) > 5 || std::stoi(pipeline_id) < 0) {
    error_response.status = "error";
    error_response.message =
        "无法对pipeline进行修改，请检查您的patch方法的pipeline_id参数！";
    error_response.error_code = 400;
    to_json(response_json, error_response);
    IVS_INFO(
        " ERROR! "
        "无法对pipeline进行修改，请检查您的patch方法的pipeline_id参数！");
    return response_json.dump();
  } else {
  }
  from_json(request_json, request);  // patch的body的json解析is_runnning
  if (request.is_running == true && pipeline_is_running == true) {
    pipelineresponse.status = "error";
    pipelineresponse.message =
        "Your pipeline " + pipelines_name_list[std::stoi(running_pipeline_id)] +
        " id: " + running_pipeline_id + " is running, Please stop it first!";
    pipelineresponsedata.is_running = pipeline_is_running;
    pipelineresponsedata.pipeline_id = std::stoi(pipeline_id);
    pipelineresponsedata.pipeline_name =
        pipelines_name_list[std::stoi(pipeline_id)];
    pipelineresponse.data.push_back(pipelineresponsedata);
    to_json(response_json, pipelineresponse);
    return response_json.dump();
  } else if (request.is_running == true && pipeline_is_running == false) {
    pipeline_is_running = true;
    running_pipeline_id = pipeline_id;
    std::thread p1_run_thread([this, pipeline_id]() { run(pipeline_id); });
    p1_run_thread.detach();
  } else {
    pipeline_is_running = false;
    stop(pipeline_id);
  }
  pipelineresponse.status = "success";
  pipelineresponse.message = "patch pipeline success";
  pipelineresponsedata.is_running = pipeline_is_running;
  pipelineresponsedata.pipeline_id = std::stoi(pipeline_id);
  pipelineresponsedata.pipeline_name =
      pipelines_name_list[std::stoi(pipeline_id)];
  pipelineresponse.data.push_back(pipelineresponsedata);
  to_json(response_json, pipelineresponse);
  return response_json.dump();
}

// 前端请求,获取json的全部区域信息
std::string HTTP_Interact_Mgr::handler_get_jsons_id(
    const std::string& json_id) {
  IVS_INFO(
      " 正在调用 get jsons id 方法... 需要调用 get 方法的 json id 为:  {0}",
      json_id);
  JsonResponse jsonresponse;
  JsonResponseData jsonresponsedata;
  ErrorResponse error_response;
  nlohmann::json response_json;
  JsonsList jsonslist;
  int json_id_int = std::stoi(json_id);
  if (json_id_int > 48 || json_id_int < 0) {
    error_response.status = "error";
    error_response.message = "无法获取 JSON ！get方法的json_id有误!";
    error_response.error_code = 500;
    to_json(response_json, error_response);
    IVS_INFO(" ERROR! 无法获取 JSON ！get方法的json_id有误!  ");
    return response_json.dump();
  }
  jsonslist = read_jsons_list();
  std::string json_name = jsonslist.jsons_list[json_id_int][json_id];
  std::string json_path = "../../../../samples/";
  if (json_id_int <= 5) {
    json_path.append("yolov5/config/");
  } else if (json_id_int <= 11) {
    json_path.append("yolox/config/");
  } else if (json_id_int <= 16) {
    json_path.append("resnet/config/");
  } else if (json_id_int <= 23) {
    json_path.append("bytetrack/config/");
  } else if (json_id_int <= 38) {
    json_path.append("yolov5_bytetrack_distributor_resnet_converger/config/");
  } else {
    json_path.append("yolox_bytetrack_osd_encode/config/");
  }
  json_path.append(json_name);
  std::ifstream file(json_path);
  nlohmann::json buffer;
  file >> buffer;
  file.close();
  jsonresponsedata.json_id = std::stoi(json_id);
  jsonresponsedata.json_name = json_name;
  jsonresponsedata.json_content = buffer;
  jsonresponse.data = jsonresponsedata;
  jsonresponse.status = "success";
  jsonresponse.message = "get json id content success!!!";
  to_json(response_json, jsonresponse);
  return response_json.dump();
}

// 前端请求,对json进行操作
std::string HTTP_Interact_Mgr::handler_put_jsons_id(
    const std::string& request_str, const std::string& json_id) {
  IVS_INFO(" 正在调用 put jsons id 方法... 需要调用 put 方法的 json id 为: {0}",
           json_id);
  JsonResponse jsonresponse;
  JsonResponseData jsonresponsedata;
  nlohmann::json request_json = nlohmann::json::parse(request_str);
  ErrorResponse error_response;
  nlohmann::json response_json;
  int json_id_int = std::stoi(json_id);
  if (json_id_int > 48 || json_id_int < 0) {
    error_response.status = "error";
    error_response.message = "无法更改服务器上的 JSON 配置！调用的方法为put！";
    error_response.error_code = 400;
    to_json(response_json, error_response);
    IVS_INFO(" ERROR! 无法更改服务器上的 JSON 配置！调用的方法为put！ ");
    return response_json.dump();
  }
  JsonsList jsonslist;
  jsonslist = read_jsons_list();
  std::string json_name = jsonslist.jsons_list[json_id_int][json_id];
  std::string json_path = "../../../../samples/";
  if (json_id_int <= 5) {
    json_path.append("yolov5/config/");
  } else if (json_id_int <= 11) {
    json_path.append("yolox/config/");
  } else if (json_id_int <= 16) {
    json_path.append("resnet/config/");
  } else if (json_id_int <= 23) {
    json_path.append("bytetrack/config/");
  } else if (json_id_int <= 38) {
    json_path.append("yolov5_bytetrack_distributor_resnet_converger/config/");
  } else {
    json_path.append("yolox_bytetrack_osd_encode/config/");
  }
  json_path.append(json_name);
  std::ofstream file_old(json_path);
  if (!file_old.is_open()) {
    error_response.status = "error";
    error_response.message =
        "没有文件读写权限，无法更改服务器上的 JSON 配置！调用的方法为put！";
    error_response.error_code = 401;
    to_json(response_json, error_response);
    IVS_INFO(
        " ERROR! 没有文件读写权限，无法更改服务器上的 JSON "
        "配置！调用的方法为put！ ");
    return response_json.dump();
  }
  file_old << "";
  file_old.close();
  std::ofstream file_new(json_path);
  file_new << std::setw(4) << request_json << std::endl;
  file_new.close();
  std::ifstream file_read(json_path);
  nlohmann::json buffer;
  file_read >> buffer;
  file_read.close();
  jsonresponsedata.json_id = std::stoi(json_id);
  jsonresponsedata.json_name = json_name;
  jsonresponsedata.json_content = buffer;
  jsonresponse.status = "success";
  jsonresponse.message = "put json id content success!!!";
  jsonresponse.data = jsonresponsedata;
  to_json(response_json, jsonresponse);
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

void HTTP_Interact_Mgr::run(const std::string& pipeline_id) {
  int run_pipeline_id = std::stoi(pipeline_id);
  char currentDir[200];  // 记录当前目录
  std::string pipeline_name_tmp = pipelines_name_list[run_pipeline_id];
  IVS_INFO(" 开始运行程序: {0}", pipeline_name_tmp);
  getcwd(currentDir, sizeof(currentDir));
  // 切换到指定目录
  std::string targetDir = "../../../../samples/";
  targetDir.append(pipeline_name_tmp);
  targetDir.append("/build");
  std::string command = "./";
  command.append(pipeline_name_tmp);
  command.append("_demo");
  if (chdir(targetDir.c_str()) == 0) {
    // 使用系统调用运行编译好的程序
    IVS_INFO(" 开始程序运行.");
    std::system(command.c_str());
  } else {
    IVS_INFO("Failed to change directory.");
  }
  // 切换回原来的工作目录
  chdir(currentDir);
}

void HTTP_Interact_Mgr::stop(const std::string& pipeline_id) {
  int stop_pipeline_id = std::stoi(pipeline_id);
  std::string pipeline_name_tmp = pipelines_name_list[stop_pipeline_id];
  IVS_INFO(" stop_pipeline_id: {0}", stop_pipeline_id);
  IVS_INFO(" 停止运行程序: {0}", pipeline_name_tmp);
  std::string command = "killall ";
  command.append(pipeline_name_tmp);
  command.append("_demo");
  std::system(command.c_str());
}

// for the future use, start here
// ----------------------------------------------------------------------//
// 前端请求,获取pipeline的指定区域信息
std::string HTTP_Interact_Mgr::handler_get_pipelines_id_field(
    const std::string& request_str) {
  IVS_INFO(" 目前暂不支持get pipeline filed 方法");
  ErrorResponse response;
  nlohmann::json response_json;
  response.status = "error";
  response.message = "目前暂不支持get pipeline filed 方法";
  response.error_code = 404;
  to_json(response_json, response);
  return response_json.dump();
}

// 前端请求,对pipeline进行操作
std::string HTTP_Interact_Mgr::handler_put_pipelines_id() {
  IVS_INFO(" 目前暂不支持put pipeline id 方法");
  ErrorResponse response;
  nlohmann::json response_json;
  response.status = "error";
  response.message = "目前暂不支持put pipeline id 方法";
  response.error_code = 404;
  to_json(response_json, response);
  return response_json.dump();
}

// 前端请求,获取json的指定区域信息
std::string HTTP_Interact_Mgr::handler_get_jsons_id_field() {
  IVS_INFO(" 目前暂不支持 get jsons filed 方法");
  ErrorResponse response;
  nlohmann::json response_json;
  response.status = "error";
  response.message = "目前暂不支持 get jsons filed 方法";
  response.error_code = 404;
  to_json(response_json, response);
  return response_json.dump();
}

// 前端请求，获取json的list信息
std::string HTTP_Interact_Mgr::handler_get_jsons() {
  IVS_INFO(" 目前暂不支持 get jsons 方法");
  ErrorResponse response;
  nlohmann::json response_json;
  response.status = "error";
  response.message = "目前暂不支持 get jsons 方法";
  response.error_code = 404;
  to_json(response_json, response);
  return response_json.dump();
}

// 前端请求,对json的指定参数进行操作
std::string HTTP_Interact_Mgr::handler_patch_jsons_id() {
  IVS_INFO(" 目前暂不支持 patch jsons filed 方法");
  ErrorResponse response;
  nlohmann::json response_json;
  response.status = "error";
  response.message = "目前暂不支持 patch jsons filed 方法";
  response.error_code = 404;
  to_json(response_json, response);
  return response_json.dump();
}
// ----------------------------------------------------------------------//
// end here