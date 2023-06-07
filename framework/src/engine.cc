
#include "engine.h"

#include "common/logger.h"

namespace sophon_stream {
namespace framework {

static constexpr const char* JSON_GRAPH_ID_FIELD = "graph_id";

Engine::Engine() {}

Engine::~Engine() {}

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

void Engine::removeGraph(int graphId) {
  std::lock_guard<std::mutex> lk(mElementManagerMapLock);
  IVS_INFO("Remove graph start, graph id: {0:d}", graphId);
  mElementManagerMap.erase(graphId);
  IVS_INFO("Remove graph finish, graph id: {0:d}", graphId);
}

bool Engine::graphExist(int graphId) {
  std::lock_guard<std::mutex> lk(mElementManagerMapLock);

  if (mElementManagerMap.end() != mElementManagerMap.find(graphId)) {
    return true;
  }
  return false;
}

void Engine::setStopHandler(int graphId, int elementId, int outputPort,
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

  graph->setStopHandler(elementId, outputPort, dataHandler);
}

common::ErrorCode Engine::pushInputData(
    int graphId, int elementId, int inputPort, std::shared_ptr<void> data) {
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

  return graph->pushInputData(elementId, inputPort, data);
}

std::pair<std::string, int> Engine::getSideAndDeviceId(int graphId,
                                                       int elementId) {
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

}  // namespace framework
}  // namespace sophon_stream
