
#include "engine.h"

#include "common/logger.h"

namespace sophon_stream {
namespace framework {

// static constexpr const char* JSON_GRAPH_ID_FIELD = "graph_id";

Engine::Engine() {}

Engine::~Engine() {}

common::ErrorCode Engine::start(int graphId) {
  IVS_INFO("Engine start graph thread start, graph id: {0:d}", graphId);
  std::lock_guard<std::mutex> lk(mGraphMapLock);
  auto graphIt = mGraphMap.find(graphId);
  if (mGraphMap.end() == graphIt) {
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

  std::lock_guard<std::mutex> lk(mGraphMapLock);
  auto graphIt = mGraphMap.find(graphId);
  if (mGraphMap.end() == graphIt) {
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

  std::lock_guard<std::mutex> lk(mGraphMapLock);
  auto graphIt = mGraphMap.find(graphId);
  if (mGraphMap.end() == graphIt) {
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

  std::lock_guard<std::mutex> lk(mGraphMapLock);
  auto graphIt = mGraphMap.find(graphId);
  if (mGraphMap.end() == graphIt) {
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
    std::lock_guard<std::mutex> lk(mGraphMapLock);

    auto graph = std::make_shared<framework::Graph>();

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

    mGraphMap[graph->getId()] = graph;
    IVS_INFO("Add graph finish, json: {0}", json);
    mGraphIds.push_back(graph->getId());

  } while (false);

  return errorCode;
}

void Engine::removeGraph(int graphId) {
  std::lock_guard<std::mutex> lk(mGraphMapLock);
  IVS_INFO("Remove graph start, graph id: {0:d}", graphId);
  mGraphMap.erase(graphId);
  IVS_INFO("Remove graph finish, graph id: {0:d}", graphId);
}

bool Engine::graphExist(int graphId) {
  std::lock_guard<std::mutex> lk(mGraphMapLock);

  if (mGraphMap.end() != mGraphMap.find(graphId)) {
    return true;
  }
  return false;
}

void Engine::setSinkHandler(int graphId, int elementId, int outputPort,
                            DataHandler dataHandler) {
  IVS_INFO(
      "Set data handler, graph id: {0:d}, element id: {1:d}, output port: "
      "{2:d}",
      graphId, elementId, outputPort);

  auto graphIt = mGraphMap.find(graphId);
  if (mGraphMap.end() == graphIt) {
    IVS_ERROR("Can not find graph, graph id: {0:d}", graphId);
    return;
  }

  auto graph = graphIt->second;
  if (!graph) {
    IVS_ERROR("Graph is null, graph id: {0:d}", graphId);
    return;
  }

  graph->setSinkHandler(elementId, outputPort, dataHandler);
}

common::ErrorCode Engine::pushSourceData(
    int graphId, int elementId, int inputPort, std::shared_ptr<void> data) {
  IVS_DEBUG(
      "send data, graph id: {0:d}, element id: {1:d}, input port: {2:d}, "
      "data: {3:p}",
      graphId, elementId, inputPort, data.get());

  auto graphIt = mGraphMap.find(graphId);
  if (mGraphMap.end() == graphIt) {
    IVS_ERROR("Can not find graph, graph id: {0:d}", graphId);
    return common::ErrorCode::NO_SUCH_GRAPH_ID;
  }

  auto graph = graphIt->second;
  if (!graph) {
    IVS_ERROR("Graph is null, graph id: {0:d}", graphId);
    return common::ErrorCode::UNKNOWN;
  }

  return graph->pushSourceData(elementId, inputPort, data);
}

std::pair<std::string, int> Engine::getSideAndDeviceId(int graphId,
                                                       int elementId) {
  IVS_INFO("Get side and device id, graph id: {0:d}, element id: {1:d}",
           graphId, elementId);

  auto graphIt = mGraphMap.find(graphId);
  if (mGraphMap.end() == graphIt) {
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


std::vector<int> Engine::getGraphIds() {
  return mGraphIds;
}

}  // namespace framework
}  // namespace sophon_stream
