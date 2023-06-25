//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_ELEMENT_WS_H_
#define SOPHON_STREAM_ELEMENT_WS_H_

#include <memory>

#include "common/object_metadata.h"
#include "element.h"

#include <iostream>
#include "wss.h"


namespace sophon_stream {
namespace element {
namespace ws {

class WS : public ::sophon_stream::framework::Element {
 public:
  WS();
  ~WS() override;

  common::ErrorCode initInternal(const std::string& json) override;
  void uninitInternal() override;

  common::ErrorCode doWork(int dataPipeId) override;

  static constexpr const char* CONFIG_INTERNAL_START_PORT_FIELD = "start_port";

 private:
  std::map<int, std::shared_ptr<WSS>> mServerMap;
  std::vector<std::thread> mServerThreads;
  std::mutex mServerThreadsMutex;
  int mStartPort;
};

}  // namespace ws
}  // namespace element
}  // namespace sophon_stream

#endif  // SOPHON_STREAM_ELEMENT_WS_H_