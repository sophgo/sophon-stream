#ifndef HTTP_INTERACT_MGR_H_
#define HTTP_INTERACT_MGR_H_
#include "httplib.h"

class HTTP_Interact_Mgr {
 public:
  HTTP_Interact_Mgr();
  ~HTTP_Interact_Mgr();

  void init(int port);
  void stop();

  static void handler(const httplib::Request& request,
                      httplib::Response& response);
  std::string handler_pipeline_list();
  std::string handler_run(const std::string& request_str);

  static void listen_thread();
  static HTTP_Interact_Mgr* GetInstance();

 private:
  httplib::Server server_;
  int port_;

  std::thread listen_thread_;
  bool is_inited_ = false;
};

#endif  // HTTP_INTERACT_MGR_H_
