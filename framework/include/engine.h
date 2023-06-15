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

#include "common/error_code.h"
#include "common/logger.h"
#include "common/no_copyable.h"
#include "common/singleton.h"
#include "graph.h"

namespace sophon_stream {
namespace framework {

class Engine : public ::sophon_stream::common::NoCopyable {
 public:
  using DataHandler = framework::Graph::DataHandler;

  common::ErrorCode start(int graphId);

  common::ErrorCode stop(int graphId);

  common::ErrorCode pause(int graphId);

  common::ErrorCode resume(int graphId);

  common::ErrorCode addGraph(const std::string& json);

  void removeGraph(int graphId);

  bool graphExist(int graphId);

  common::ErrorCode pushSourceData(int graphId, int elementId, int inputPort,
                                  std::shared_ptr<void> data);

  void setSinkHandler(int graphId, int elementId, int outputPort,
                      DataHandler dataHandler);

  std::pair<std::string, int> getSideAndDeviceId(int graphId, int elementId);

  std::vector<int> getGraphIds();

  static constexpr const char* JSON_GRAPH_ID_FIELD = "graph_id";

 private:
  friend class common::Singleton<Engine>;

  Engine();

  ~Engine();

  std::map<int /* graphId */, std::shared_ptr<framework::Graph> >
      mGraphMap;
  std::mutex mGraphMapLock;

  std::vector<int> mGraphIds;
};

using SingletonEngine = common::Singleton<Engine>;

}  // namespace framework
}  // namespace sophon_stream

#endif  // SOPHON_STREAM_FRAMEWORK_ENGINE_H_