//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#include "http_interact_mgr.h"

int main(int argc, const char** argv) {
  int port = 8000;

  if (argc >= 2) {
    port = atoi(argv[1]);  // ASCLL to INT
  }

  HTTP_Interact_Mgr* mgr = HTTP_Interact_Mgr::GetInstance();
  mgr->init(port);
  std::cout << "  ___           _             " << std::endl;
  std::cout << " / __| ___ _ __| |_  ___ _ _  " << std::endl;
  std::cout << " \\__ \\/ _ \\ '_ \\ ' \\/ _ \\ ' \\ " << std::endl;
  std::cout << " |___/\\___/ .__/|_|_\\___/_||_|" << std::endl;
  std::cout << "          |_|                  " << std::endl << std::endl;
  IVS_INFO("正在监听端口... listen on: {0}", port);
  IVS_INFO(
      "------------------------------------------------------------------");
  while (1) {
    sleep(10);
  }
  return 0;
}
