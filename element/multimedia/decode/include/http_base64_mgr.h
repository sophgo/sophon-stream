//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_ELEMENT_MULTIMEDIA_DECODE_HTTP_BASE64_MGR_H_
#define SOPHON_STREAM_ELEMENT_MULTIMEDIA_DECODE_HTTP_BASE64_MGR_H_

#include <../../../3rdparty/httplib/httplib.h>

#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <queue>

#include "bmcv_api.h"
#include "bmcv_api_ext.h"
#include "bmlib_runtime.h"
#include "bmruntime_interface.h"
#include "common/logger.h"

namespace sophon_stream {
namespace element {
namespace decode {

static std::string base64_url_;

class HTTP_Base64_Mgr {
 public:
  HTTP_Base64_Mgr();
  ~HTTP_Base64_Mgr();

  void init(int port, std::string url);
  void stop();

  static void handler(const httplib::Request& request,
                      httplib::Response& response);
  std::string handler_Base64(const std::string& request_str);
  std::shared_ptr<bm_image> grab(bm_handle_t& handle);

  static void listen_thread();
  static HTTP_Base64_Mgr* GetInstance();

 private:
  httplib::Server server_;
  int port_;
  std::queue<std::string> base64_queue_;

  std::thread listen_thread_;
  bool is_inited_ = false;

  struct Base64Request {
    std::string Data;
  };

  struct Base64Response {
    std::string status;
    std::string message;
  };

  void from_json(const nlohmann::json& j, Base64Request& p) {
    p.Data = j.at("data").get<std::string>();
  }

  void to_json(nlohmann::json& j, const Base64Response& p) {
    j = nlohmann::json{{"status", p.status}, {"message", p.message}};
  }
};

}  // namespace decode
}  // namespace element
}  // namespace sophon_stream

#endif // SOPHON_STREAM_ELEMENT_MULTIMEDIA_DECODE_HTTP_BASE64_MGR_H_