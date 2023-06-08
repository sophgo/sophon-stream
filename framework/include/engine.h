//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_FRAMEWORK_ELEMENT_ENGINE_H_
#define SOPHON_STREAM_FRAMEWORK_ELEMENT_ENGINE_H_

#include <chrono>
#include <map>
#include <memory>
#include <string>

#include "common/ErrorCode.h"
#include "common/Singleton.hpp"
#include "common/logger.h"
#include "element_manager.h"

namespace sophon_stream {
namespace framework {

class Engine {
 public:
  using DataHandler = framework::ElementManager::DataHandler;

  common::ErrorCode start(int graphId);

  common::ErrorCode stop(int graphId);

  common::ErrorCode pause(int graphId);

  common::ErrorCode resume(int graphId);

  common::ErrorCode addGraph(const std::string& json);

  void removeGraph(int graphId);

  bool graphExist(int graphId);

  common::ErrorCode pushInputData(int graphId, int elementId, int inputPort,
                                  std::shared_ptr<void> data);

  void setStopHandler(int graphId, int elementId, int outputPort,
                      DataHandler dataHandler);

  std::pair<std::string, int> getSideAndDeviceId(int graphId, int elementId);

  std::vector<int> getGraphIds();

 private:
  friend class common::Singleton<Engine>;

  Engine();

  ~Engine();

  Engine(const Engine&) = delete;
  Engine& operator=(const Engine&) = delete;
  Engine(Engine&&) = delete;
  Engine& operator=(Engine&&) = delete;

  std::map<int /* graphId */, std::shared_ptr<framework::ElementManager> >
      mElementManagerMap;
  std::mutex mElementManagerMapLock;

  std::vector<int> mGraphIds;
};

using SingletonEngine = common::Singleton<Engine>;

}  // namespace framework
}  // namespace sophon_stream

#endif  // SOPHON_STREAM_FRAMEWORK_ENGINE_H_