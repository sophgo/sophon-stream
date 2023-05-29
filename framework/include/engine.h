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

#include "element_manager.h"
#include "common/ErrorCode.h"
#include "common/logger.h"
#include "common/Singleton.hpp"

namespace sophon_stream {
namespace framework {

/**
 * A class manages a lot of graphs, each graph is a WorkManager.
 */
class Engine {
 public:
  using DataHandler = framework::ElementManager::DataHandler;

  /**
   * Start all threads of Elements in a graph,
   * @param[in] graphId : Id of ElementManager.
   * @return If thread status is not ThreadStatus::STOP,
   * it will return common::ErrorCode::THREAD_STATUS_ERROR,
   * otherwise return common::ErrorCode::SUCESS.
   */
  common::ErrorCode start(int graphId);

  /**
   * Stop all threads of Elements in a graph.
   * @param[in] graphId : Id of ElementManager.
   * @return If thread status is ThreadStatus::STOP,
   * it will return common::ErrorCode::THREAD_STATUS_ERROR,
   * otherwise return common::ErrorCode::SUCESS.
   */
  common::ErrorCode stop(int graphId);

  /**
   * Pause all threads of Elements in a graph.
   * @param[in] graphId : Id of ElementManager.
   * @return If thread status is not ThreadStatus::RUN,
   * it will return common::ErrorCode::THREAD_STATUS_ERROR,
   * otherwise return common::ErrorCode::SUCESS.
   */
  common::ErrorCode pause(int graphId);

  /**
   * Resume all threads of Elements in a graph.
   * @param[in] graphId : Id of ElementManager.
   * @return If thread status is not ThreadStatus::PAUSE,
   * it will return common::ErrorCode::THREAD_STATUS_ERROR,
   * otherwise return common::ErrorCode::SUCESS.
   */
  common::ErrorCode resume(int graphId);

  /**
   * Add a graph.
   * @param[in] json : Configure to init ElementManager.
   * @return If ElementManager init fail or id of ElementManager has been exist,
   * it will return error,
   * otherwise return common::ErrorCode::SUCESS.
   */
  common::ErrorCode addGraph(const std::string& json);

  /**
   * Remove a graph.
   * @param[in] graphId : Id of ElementManager.
   */
  void removeGraph(int graphId);

  /**
   * Judge a graph exists or not.
   * @param[in] graphId : Id of ElementManager.
   */
  bool graphExist(int graphId);

  /**
   * Send data to a element in a graph with a input port.
   * @param[in] graphId : Id of ElementManager which will send data to.
   * @param[in] elementId : Id of Element which will send data to.
   * @param[in] inputPort : Input port of Element which will send data to.
   * @param[in] data : The data that will be send.
   * @param[in] timeout : The duration that will be wait for.
   * @return If graph id or element id is not exist or timeout, it will return
   * error, otherwise return common::ErrorCode::SUCESS.
   */
  template <class Rep, class Period>
  common::ErrorCode sendData(
      int graphId, int elementId, int inputPort, std::shared_ptr<void> data,
      const std::chrono::duration<Rep, Period>& timeout) {
    IVS_DEBUG(
        "send data, graph id: {0:d}, element id: {1:d}, input port: {2:d}, "
        "data: {3:p}",
        graphId, elementId, inputPort, data.get());

    auto graphIt = mElementManagerMap.find(graphId);
    if (mElementManagerMap.end() == graphIt) {
      IVS_ERROR("Can not find graph, graph id: {0:d}", graphId);
      return common::ErrorCode::NO_SUCH_GRAPH_ID;
    }

    auto graph = graphIt->second;
    if (!graph) {
      IVS_ERROR("Graph is null, graph id: {0:d}", graphId);
      return common::ErrorCode::UNKNOWN;
    }

    return graph->sendData(elementId, inputPort, data, timeout);
  }

  /**
   * Set callback to a element in a graph with a output port for receive data.
   * @param[in] graphId : Id of ElementManager which will receive data.
   * @param[in] elementId : Id of Element which will receive data.
   * @param[in] outputPort : Output port of Element which will receive data.
   * @param[in] dataHandler : The callback that will be call with received data.
   */
  void setDataHandler(int graphId, int elementId, int outputPort,
                      DataHandler dataHandler) {
    IVS_INFO(
        "Set data handler, graph id: {0:d}, element id: {1:d}, output port: "
        "{2:d}",
        graphId, elementId, outputPort);

    auto graphIt = mElementManagerMap.find(graphId);
    if (mElementManagerMap.end() == graphIt) {
      IVS_ERROR("Can not find graph, graph id: {0:d}", graphId);
      return;
    }

    auto graph = graphIt->second;
    if (!graph) {
      IVS_ERROR("Graph is null, graph id: {0:d}", graphId);
      return;
    }

    graph->setDataHandler(elementId, outputPort, dataHandler);
  }

  std::pair<std::string, int> getSideAndDeviceId(int graphId, int elementId) {
    IVS_INFO("Get side and device id, graph id: {0:d}, element id: {1:d}",
             graphId, elementId);

    auto graphIt = mElementManagerMap.find(graphId);
    if (mElementManagerMap.end() == graphIt) {
      IVS_ERROR("Can not find graph, graph id: {0:d}", graphId);
      return std::make_pair("", -1);
    }

    auto graph = graphIt->second;
    if (!graph) {
      IVS_ERROR("Graph is null, graph id: {0:d}", graphId);
      return std::make_pair("", -1);
    }

    return graph->getSideAndDeviceId(elementId);
  }

 private:
  friend class common::Singleton<Engine>;

  /**
   * Constructor of class Engine, only be call by common::Singleton<Engine>.
   */
  Engine();

  /**
   * Destructor of class Engine, only be call by common::Singleton<Engine>.
   */
  ~Engine();

  Engine(const Engine&) = delete;
  Engine& operator=(const Engine&) = delete;
  Engine(Engine&&) = delete;
  Engine& operator=(Engine&&) = delete;

  /**
   * ElementManager map, key is graph id, value is std::shared_ptr of
   * frameelement::ElementManager.
   */
  std::map<int /* graphId */, std::shared_ptr<framework::ElementManager> >
      mElementManagerMap;
  std::mutex mElementManagerMapLock;
};

/**
 * Type define of common::Singleton<Engine>.
 */
using SingletonEngine = common::Singleton<Engine>;

}  // namespace framework
}  // namespace sophon_stream

#endif // SOPHON_STREAM_FRAMEWORK_ENGINE_H_