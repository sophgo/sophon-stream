//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#include "http_base64_mgr.h"

#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>

#include <websocketpp/base64/base64.hpp>

namespace sophon_stream {
namespace element {
namespace decode {

HTTP_Base64_Mgr::HTTP_Base64_Mgr() {}

HTTP_Base64_Mgr::~HTTP_Base64_Mgr() { stop(); }

void HTTP_Base64_Mgr::init(int port, std::string url) {
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
  base64_url_ = url;
  server_.Post(base64_url_, HTTP_Base64_Mgr::handler);
  listen_thread_ = std::thread(&HTTP_Base64_Mgr::listen_thread);
  is_inited_ = true;
}

void HTTP_Base64_Mgr::stop() {
  server_.stop();
  listen_thread_.join();

  is_inited_ = false;
}

void HTTP_Base64_Mgr::handler(const httplib::Request& request,
                              httplib::Response& response) {
  HTTP_Base64_Mgr* mgr = HTTP_Base64_Mgr::GetInstance();
  std::string response_str;
  if (request.path == base64_url_)
    response_str = mgr->handler_Base64(request.body);
  response.set_content(response_str, "application/json");
}

std::string HTTP_Base64_Mgr::handler_Base64(const std::string& request_str) {
  Base64Request request;
  nlohmann::json request_json = nlohmann::json::parse(request_str);
  from_json(request_json, request);
  base64_queue_.push(request.Data);

  Base64Response response;
  response.status = "success";
  response.message = "send base64 success";

  nlohmann::json response_json;
  to_json(response_json, response);

  return response_json.dump();
}

std::shared_ptr<bm_image> HTTP_Base64_Mgr::grab(bm_handle_t& handle) {
  std::shared_ptr<bm_image> spBmImage = nullptr;
  spBmImage.reset(new bm_image, [](bm_image* p) {
    bm_image_destroy(*p);
    delete p;
    p = nullptr;
  });

  while (1) {
    if (base64_queue_.empty()) {
      IVS_WARN("Waiting for base64 data, retry...");
      sleep(1);
      continue;
    }
    std::string base64_str = base64_queue_.front();
    base64_queue_.pop();

    std::string img_str = websocketpp::base64_decode(base64_str);
    size_t size = img_str.length();
    // 如果传入base64数据不是jpeg格式，会报错[BMCV][error]  [MESSAGE FROM
    // bmcv_api_jpeg_dec.cpp: try_soft_decoding: 433]: jpeg-turbo read header
    // failed!
    int ret = bmcv_image_jpeg_dec(handle, (void**)&img_str, &size, 1,
                                  spBmImage.get());
    if (ret == BM_SUCCESS) break;

    IVS_ERROR("bmcv_image_jpeg_dec failed");
  }
  return spBmImage;
}

HTTP_Base64_Mgr* HTTP_Base64_Mgr::GetInstance() {
  static HTTP_Base64_Mgr inst;
  return &inst;
}

void HTTP_Base64_Mgr::listen_thread() {
  HTTP_Base64_Mgr* mgr = HTTP_Base64_Mgr::GetInstance();
  mgr->server_.listen("0.0.0.0", mgr->port_);
  return;
}

}  // namespace decode
}  // namespace element
}  // namespace sophon_stream