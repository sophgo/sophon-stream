//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef HTTP_INTERACT_MGR_H_
#define HTTP_INTERACT_MGR_H_
#include "common/logger.h"
#include "httplib.h"

class HTTP_Interact_Mgr {
 public:
  HTTP_Interact_Mgr();
  ~HTTP_Interact_Mgr();

  void init(int port);
  void stop();

  static void handler(const httplib::Request& request,
                      httplib::Response& response);

  std::string handler_get_pipelines();
  std::string handler_get_pipelines_id_field(const std::string& request_str);
  std::string handler_get_pipelines_id(const std::string& pipeline_id);
  std::string handler_put_pipelines_id();
  std::string handler_patch_pipelines_id(const std::string& request_str,
                                         const std::string& pipeline_id);

  std::string handler_get_jsons();
  std::string handler_get_jsons_id_field();
  std::string handler_get_jsons_id(const std::string& json_id);
  std::string handler_put_jsons_id(const std::string& request_str,
                                   const std::string& json_id);
  std::string handler_patch_jsons_id();

  static void listen_thread();
  static HTTP_Interact_Mgr* GetInstance();
  bool pipeline_is_running = false;
  std::string running_pipeline_id;
  std::vector<std::string> pipelines_name_list;
  void run(const std::string& pipeline_id);
  void stop(const std::string& pipeline_id);

 private:
  httplib::Server server_;
  int port_;
  std::thread listen_thread_;
  bool is_inited_ = false;
};

#endif  // HTTP_INTERACT_MGR_H_
