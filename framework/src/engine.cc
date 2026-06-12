
#include "engine.h"

#include "common/logger.h"

namespace sophon_stream {
namespace framework {

Engine::Engine() {}

Engine::~Engine() {}

common::ErrorCode Engine::start(int graphId) {
  IVS_INFO("Engine start graph thread start, graph id: {0:d}", graphId);

  // Engine 只负责按 graphId 找到目标 Graph；具体线程启动逻辑由 Graph 处理。
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
  // 转交给 Graph 启动其内部所有 Element 工作线程。
  return graph->start();
}

common::ErrorCode Engine::stop(int graphId) {
  IVS_INFO("Engine stop graph thread start, graph id: {0:d}", graphId);

  // 停止前先加锁查询，避免 Graph 表在并发修改时被访问。
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
  // 转交给 Graph 停止其内部所有 Element 工作线程。
  return graph->stop();
}

common::ErrorCode Engine::pause(int graphId) {
  IVS_INFO("Engine pause graph thread start, graph id: {0:d}", graphId);

  // 暂停逻辑与 start/stop 一样：Engine 校验 Graph，Graph 处理具体状态切换。
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
  // 将 Graph 内 Element 线程状态切换为暂停。
  return graph->pause();
}

common::ErrorCode Engine::resume(int graphId) {
  IVS_INFO("Engine resume graph thread start, graph id: {0:d}", graphId);

  // 恢复前确认 Graph 存在，避免对空指针或不存在的 Graph 操作。
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
  // 将 Graph 内 Element 线程状态从暂停恢复为运行。
  return graph->resume();
}

common::ErrorCode Engine::addGraph(const std::string& json) {
  IVS_INFO("Add graph start, json: {0}", json);

  common::ErrorCode errorCode = common::ErrorCode::SUCCESS;

  do {
    std::lock_guard<std::mutex> lk(mGraphMapLock);

    // 创建新 Graph，并把 Engine 持有的监听线程传递下去，用于状态上报和外部控制。
    auto graph = std::make_shared<framework::Graph>();
    graph->setListener(listenThreadPtr);

    // Graph::init 负责解析 JSON、创建 Element、注册端口并建立 Element 连接。
    errorCode = graph->init(json);
    listenThreadPtr->report_status(errorCode);
    if (common::ErrorCode::SUCCESS != errorCode) {
      IVS_ERROR("Graph init fail, json: {0}", json);
      return errorCode;
    }

    // addGraph 在本项目中不仅“添加”Graph，还会立即启动 Graph。
    errorCode = graph->start();
    listenThreadPtr->report_status(errorCode);

    if (common::ErrorCode::SUCCESS != errorCode) {
      IVS_ERROR("Graph start fail");
      return errorCode;
    }

    // Graph 成功启动后再加入管理表，后续可通过 graphId 进行控制或推送数据。
    mGraphMap[graph->getId()] = graph;
    IVS_INFO("Add graph finish, json: {0}", json);
    mGraphIds.push_back(graph->getId());

  } while (false);

  return errorCode;
}

void Engine::removeGraph(int graphId) {
  std::lock_guard<std::mutex> lk(mGraphMapLock);
  IVS_INFO("Remove graph start, graph id: {0:d}", graphId);
  // 这里只移除 Graph 对象；如果需要优雅停止，调用方应先执行 stop(graphId)。
  mGraphMap.erase(graphId);
  IVS_INFO("Remove graph finish, graph id: {0:d}", graphId);
}

bool Engine::graphExist(int graphId) {
  std::lock_guard<std::mutex> lk(mGraphMapLock);

  // 查询当前 Engine 是否已经管理指定 graphId 的 Graph。
  if (mGraphMap.end() != mGraphMap.find(graphId)) {
    return true;
  }
  return false;
}

void Engine::setSinkHandler(int graphId, int elementId, int outputPort,
                            SinkHandler sinkHandler) {
  IVS_INFO(
      "Set data handler, graph id: {0:d}, element id: {1:d}, output port: "
      "{2:d}",
      graphId, elementId, outputPort);

  // sinkHandler 用于处理图末端输出数据，例如绘制、保存、上报或统计。
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

  // 具体是否为 sink Element、outputPort 是否有效，由 Graph/Element 继续处理。
  graph->setSinkHandler(elementId, outputPort, sinkHandler);
}

common::ErrorCode Engine::pushSourceData(int graphId, int elementId,
                                         int inputPort,
                                         std::shared_ptr<void> data) {
  IVS_DEBUG(
      "send data, graph id: {0:d}, element id: {1:d}, input port: {2:d}, "
      "data: {3:p}",
      graphId, elementId, inputPort, data.get());

  // 向指定 Graph 的 source Element 推送外部输入，一般用于启动 decode 任务。
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

  // Graph 会继续定位 Element，并把 data 放入对应 inputPort 的 DataPipe。
  return graph->pushSourceData(elementId, inputPort, data);
}

std::pair<std::string, int> Engine::getSideAndDeviceId(int graphId,
                                                       int elementId) {
  IVS_INFO("Get side and device id, graph id: {0:d}, element id: {1:d}",
           graphId, elementId);

  // 查询指定 Element 所在侧和设备号，常用于外部根据配置选择运行设备。
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

// 返回已添加 Graph 的 id 列表。注意 removeGraph 当前不会同步移除 mGraphIds 中的记录。
std::vector<int> Engine::getGraphIds() { return mGraphIds; }

}  // namespace framework
}  // namespace sophon_stream
