
#include "engine.h"

#include "common/Logger.h"

namespace sophon_stream {
namespace framework {

static constexpr const char* JSON_GRAPH_ID_FIELD = "graph_id";

/**
 * Constructor of class Engine, only be call by common::Singleton<Engine>.
 */
Engine::Engine() {}

/**
 * Destructor of class Engine, only be call by common::Singleton<Engine>.
 */
Engine::~Engine() {}

/**
 * Start all threads of Elements in a graph,
 * @param[in] graphId : Id of ElementManager.
 * @return If thread status is not ThreadStatus::STOP,
 * it will return common::ErrorCode::THREAD_STATUS_ERROR,
 * otherwise return common::ErrorCode::SUCESS.
 */
common::ErrorCode Engine::start(int graphId) {
  IVS_INFO("Engine start graph thread start, graph id: {0:d}", graphId);
  std::lock_guard<std::mutex> lk(mElementManagerMapLock);
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

  IVS_INFO("Engine start graph thread finish, graph id: {0:d}", graphId);
  return graph->start();
}

/**
 * Stop all threads of Elements in a graph.
 * @param[in] graphId : Id of ElementManager.
 * @return If thread status is ThreadStatus::STOP,
 * it will return common::ErrorCode::THREAD_STATUS_ERROR,
 * otherwise return common::ErrorCode::SUCESS.
 */
common::ErrorCode Engine::stop(int graphId) {
  IVS_INFO("Engine stop graph thread start, graph id: {0:d}", graphId);

  std::lock_guard<std::mutex> lk(mElementManagerMapLock);
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

  IVS_INFO("Engine stop graph thread finish, graph id: {0:d}", graphId);
  return graph->stop();
}

/**
 * Pause all threads of Elements in a graph.
 * @param[in] graphId : Id of ElementManager.
 * @return If thread status is not ThreadStatus::RUN,
 * it will return common::ErrorCode::THREAD_STATUS_ERROR,
 * otherwise return common::ErrorCode::SUCESS.
 */
common::ErrorCode Engine::pause(int graphId) {
  IVS_INFO("Engine pause graph thread start, graph id: {0:d}", graphId);

  std::lock_guard<std::mutex> lk(mElementManagerMapLock);
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

  IVS_INFO("Engine pause graph thread finish, graph id: {0:d}", graphId);
  return graph->pause();
}

/**
 * Resume all threads of Elements in a graph.
 * @param[in] graphId : Id of ElementManager.
 * @return If thread status is not ThreadStatus::PAUSE,
 * it will return common::ErrorCode::THREAD_STATUS_ERROR,
 * otherwise return common::ErrorCode::SUCESS.
 */
common::ErrorCode Engine::resume(int graphId) {
  IVS_INFO("Engine resume graph thread start, graph id: {0:d}", graphId);

  std::lock_guard<std::mutex> lk(mElementManagerMapLock);
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

  IVS_INFO("Engine resume graph thread finish, graph id: {0:d}", graphId);
  return graph->resume();
}

/**
 * Add a graph.
 * @param[in] json : Configure to init ElementManager.
 * @return If ElementManager init fail or id of ElementManager has been exist,
 * it will return error,
 * otherwise return common::ErrorCode::SUCESS.
 */
common::ErrorCode Engine::addGraph(const std::string& json) {
  IVS_INFO("Add graph start, json: {0}", json);

  common::ErrorCode errorCode = common::ErrorCode::SUCCESS;

  do {
    std::lock_guard<std::mutex> lk(mElementManagerMapLock);

    auto graph = std::make_shared<framework::ElementManager>();

    errorCode = graph->init(json);
    if (common::ErrorCode::SUCCESS != errorCode) {
      IVS_ERROR("Graph init fail, json: {0}", json);
      return errorCode;
    }

    errorCode = graph->start();
    if (common::ErrorCode::SUCCESS != errorCode) {
      IVS_ERROR("Graph start fail");
      return errorCode;
    }

    mElementManagerMap[graph->getId()] = graph;
    IVS_INFO("Add graph finish, json: {0}", json);

  } while (false);

  return errorCode;
}

/**
 * Remove a graph.
 * @param[in] graphId : Id of ElementManager.
 */
void Engine::removeGraph(int graphId) {
  std::lock_guard<std::mutex> lk(mElementManagerMapLock);
  IVS_INFO("Remove graph start, graph id: {0:d}", graphId);
  mElementManagerMap.erase(graphId);
  IVS_INFO("Remove graph finish, graph id: {0:d}", graphId);
}

/**
 * Judge a graph exists or not.
 * @param[in] graphId : Id of ElementManager.
 */
bool Engine::graphExist(int graphId) {
  std::lock_guard<std::mutex> lk(mElementManagerMapLock);

  if (mElementManagerMap.end() != mElementManagerMap.find(graphId)) {
    return true;
  }
  return false;
}

}  // namespace framework
}  // namespace sophon_stream
