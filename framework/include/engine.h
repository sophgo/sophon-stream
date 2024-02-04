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
  using SinkHandler = framework::Graph::SinkHandler;

  common::ErrorCode start(int graphId);

  common::ErrorCode stop(int graphId);

  common::ErrorCode pause(int graphId);

  common::ErrorCode resume(int graphId);

  /**
   * @brief 从配置文件初始化一个有向无环图，并将线程状态设置为RUN
   */
  common::ErrorCode addGraph(const std::string& json);

  void removeGraph(int graphId);

  bool graphExist(int graphId);

  /**
   * @brief 向指定graph的指定element推入数据，用于向decode
   * element发送启动任务的信号
   */
  common::ErrorCode pushSourceData(int graphId, int elementId, int inputPort,
                                   std::shared_ptr<void> data);
  /**
   * @brief
   * 为指定graph的指定element设置sinkHandler，sinkHandler当且仅当指定element是sink
   * element时才生效
   */
  void setSinkHandler(int graphId, int elementId, int outputPort,
                      SinkHandler sinkHandler);

  std::pair<std::string, int> getSideAndDeviceId(int graphId, int elementId);

  std::vector<int> getGraphIds();

  inline ListenThread* getListener() { return listenThreadPtr; }

  inline void setListener(ListenThread* p) { listenThreadPtr = p; }

  static constexpr const char* JSON_GRAPH_ID_FIELD = "graph_id";

 private:
  friend class common::Singleton<Engine>;

  Engine();

  ~Engine();

  std::map<int /* graphId */, std::shared_ptr<framework::Graph> > mGraphMap;
  std::mutex mGraphMapLock;

  std::vector<int> mGraphIds;

  ListenThread* listenThreadPtr;
};

using SingletonEngine = common::Singleton<Engine>;

}  // namespace framework
}  // namespace sophon_stream

#endif  // SOPHON_STREAM_FRAMEWORK_ENGINE_H_