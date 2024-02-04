//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#include "graph.h"

#include <dlfcn.h>

#include <nlohmann/json.hpp>
#include <set>
#include <string>

#include "common/logger.h"
#include "element_factory.h"

namespace sophon_stream {
namespace framework {

Graph::Graph() : mId(-1), mThreadStatus(ThreadStatus::STOP) {}

Graph::~Graph() {
  auto& elementFactory = framework::SingletonElementFactory::getInstance();
  elementFactory.~ElementFactory();
  // uninit();
}

common::ErrorCode Graph::init(const std::string& json) {
  IVS_INFO("Init start, json: {0}", json);

  common::ErrorCode errorCode = common::ErrorCode::SUCCESS;

  do {
    auto configure = nlohmann::json::parse(json, nullptr, false);
    if (!configure.is_object()) {
      IVS_ERROR("Parse json fail or json is not object, json: {0}", json);
      errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
      break;
    }

    auto graphIdIt = configure.find(JSON_GRAPH_ID_FIELD);
    if (configure.end() == graphIdIt || !graphIdIt->is_number_integer()) {
      IVS_ERROR(
          "Can not find {0} with integer type in graph json configure, json: "
          "{1}",
          JSON_GRAPH_ID_FIELD, json);
      errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
      break;
    }

    mId = graphIdIt->get<int>();

    auto elementsIt = configure.find(JSON_WORKERS_FIELD);
    if (configure.end() != elementsIt) {
      errorCode = initElements(elementsIt->dump());
      if (common::ErrorCode::SUCCESS != errorCode) {
        break;
      }
    }

    auto connectionsIt = configure.find(JSON_CONNECTIONS_FIELD);
    if (configure.end() != connectionsIt) {
      errorCode = initConnections(connectionsIt->dump());
      if (common::ErrorCode::SUCCESS != errorCode) {
        break;
      }
    }

  } while (false);

  if (common::ErrorCode::SUCCESS != errorCode) {
    uninit();
  }

  IVS_INFO("Init finish, json: {0}", json);
  return errorCode;
}

void Graph::uninit() {
  int id = mId;
  IVS_INFO("Uninit start, graph id: {0:d}", id);

  stop();

  mElementMap.clear();
  mId = -1;

  mSharedObjectHandles.clear();

  IVS_INFO("Uninit finish, graph id: {0:d}", id);
}

common::ErrorCode Graph::start() {
  IVS_INFO("Start graph thread start, graph id: {0:d}", mId);

  if (ThreadStatus::STOP != mThreadStatus) {
    IVS_ERROR(
        "Can not start, current thread status is not stop, graph id: {0:d}",
        mId);
    return common::ErrorCode::THREAD_STATUS_ERROR;
  }

  for (auto pair : mElementMap) {
    auto element = pair.second;
    if (!element) {
      continue;
    }

    element->start();
  }

  mThreadStatus = ThreadStatus::RUN;

  IVS_INFO("Start graph thread finish, graph id: {0:d}", mId);
  return common::ErrorCode::SUCCESS;
}

common::ErrorCode Graph::stop() {
  IVS_INFO("Stop graph thread start, graph id: {0:d}", mId);

  if (ThreadStatus::STOP == mThreadStatus) {
    IVS_ERROR("Can not stop, current thread status is stop");
    return common::ErrorCode::THREAD_STATUS_ERROR;
  }

  for (auto pair : mElementMap) {
    auto element = pair.second;
    if (!element) {
      continue;
    }

    element->stop();
  }

  mThreadStatus = ThreadStatus::STOP;

  IVS_INFO("Stop graph thread finish, graph id: {0:d}", mId);
  return common::ErrorCode::SUCCESS;
}

common::ErrorCode Graph::pause() {
  IVS_INFO("Pause graph thread start, graph id: {0:d}", mId);

  if (ThreadStatus::RUN != mThreadStatus) {
    IVS_ERROR(
        "Can not pause, current thread status is not run, graph id: {0:d}",
        mId);
    return common::ErrorCode::THREAD_STATUS_ERROR;
  }

  for (auto pair : mElementMap) {
    auto element = pair.second;
    if (!element) {
      continue;
    }

    element->pause();
  }

  mThreadStatus = ThreadStatus::PAUSE;

  IVS_INFO("Pause graph thread finish, graph id: {0:d}", mId);
  return common::ErrorCode::SUCCESS;
}

common::ErrorCode Graph::resume() {
  IVS_INFO("Resume graph thread start, graph id: {0:d}", mId);

  if (ThreadStatus::PAUSE != mThreadStatus) {
    IVS_ERROR(
        "Can not resume, current thread status is not pause, graph id: {0:d}",
        mId);
    return common::ErrorCode::THREAD_STATUS_ERROR;
  }

  for (auto pair : mElementMap) {
    auto element = pair.second;
    if (!element) {
      continue;
    }

    element->resume();
  }

  mThreadStatus = ThreadStatus::RUN;

  IVS_INFO("Resume graph thread finish, graph id: {0:d}", mId);
  return common::ErrorCode::SUCCESS;
}

common::ErrorCode Graph::initElements(const std::string& json) {
  IVS_INFO("Init elements start, graph id: {0:d}, json: {1}", mId, json);

  common::ErrorCode errorCode = common::ErrorCode::SUCCESS;

  do {
    auto elementsConfigure = nlohmann::json::parse(json, nullptr, false);
    if (!elementsConfigure.is_array()) {
      IVS_ERROR(
          "Parse json fail or json is not array, graph id: {0:d}, json: {1}",
          mId, json);
      errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
      break;
    }

    int numElements = elementsConfigure.size();
    for (int elementIndex = 0; elementIndex < numElements; elementIndex++) {
      auto& elementConfigure = elementsConfigure[elementIndex];
      std::cout << elementConfigure.dump() << "\n";
      if (!elementConfigure.is_object()) {
        IVS_ERROR(
            "Element json configure is not object, graph id: {0:d}, json: {1}",
            mId, elementConfigure.dump());
        errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
        break;
      }

      auto sharedObjectIt =
          elementConfigure.find(JSON_MODEL_SHARED_OBJECT_FIELD);
      if (elementConfigure.end() != sharedObjectIt &&
          sharedObjectIt->is_string() && !sharedObjectIt->empty()) {
        const auto& sharedObject = sharedObjectIt->get<std::string>();
        void* sharedObjectHandle =
            dlopen(sharedObject.c_str(), RTLD_NOW | RTLD_GLOBAL);
        if (NULL == sharedObjectHandle) {
          IVS_ERROR(
              "Load dynamic shared object file fail, element id: {0:d}, "
              "shared object: {1}  error info:{2}",
              getId(), sharedObject, dlerror());
          errorCode = common::ErrorCode::DLOPEN_FAIL;
          break;
        }

        mSharedObjectHandles.push_back(std::shared_ptr<void>(
            sharedObjectHandle,
            [](void* sharedObjectHandle) { dlclose(sharedObjectHandle); }));
      }

      auto nameIt = elementConfigure.find(JSON_WORKER_NAME_FIELD);
      if (elementConfigure.end() == nameIt || !nameIt->is_string()) {
        IVS_ERROR(
            "Can not find {0} with string type in element json configure, "
            "graph id: {1:d}, json: {2}",
            JSON_WORKER_NAME_FIELD, mId, elementConfigure.dump());
        errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
        break;
      }

      auto& elementFactory = framework::SingletonElementFactory::getInstance();
      auto element = elementFactory.make(nameIt->get<std::string>());
      if (!element) {
        IVS_ERROR("Make element fail, graph id: {0:d}, name: {1}", mId,
                  nameIt->get<std::string>());
        errorCode = common::ErrorCode::NO_SUCH_WORKER;
        break;
      }

      errorCode = element->init(elementConfigure.dump());
      if (common::ErrorCode::SUCCESS != errorCode) {
        IVS_ERROR("Init element fail, graph id: {0:d}, name: {1}", mId,
                  nameIt->get<std::string>());
        break;
      }

      if (mElementMap.end() != mElementMap.find(element->getId())) {
        IVS_ERROR("Repeated element id, graph id: {0:d}, element id: {1:d}",
                  mId, element->getId());
        errorCode = common::ErrorCode::REPEATED_WORKER_ID;
        break;
      }

      element->setListener(listenThreadPtr);
      element->registListenFunc(listenThreadPtr);

      if (element->getGroup()) {
        element->groupInsert(mElementMap);
      }

      mElementMap[element->getId()] = element;
    }
    if (common::ErrorCode::SUCCESS != errorCode) {
      break;
    }

  } while (false);

  IVS_INFO("Init elements finish, graph id: {0:d}, json: {1}", mId, json);
  return errorCode;
}

common::ErrorCode Graph::initConnections(const std::string& json) {
  IVS_INFO("Init connections start, graph id: {0:d}, json: {1}", mId, json);

  common::ErrorCode errorCode = common::ErrorCode::SUCCESS;

  do {
    auto connectionsConfigure = nlohmann::json::parse(json, nullptr, false);
    if (!connectionsConfigure.is_array()) {
      IVS_ERROR(
          "Parse json fail or json is not array, graph id: {0:d}, json: {1}",
          mId, json);
      errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
      break;
    }

    for (auto connectionConfigure : connectionsConfigure) {
      if (!connectionConfigure.is_object()) {
        IVS_ERROR(
            "Connection json configure is not object, graph id: {0:d}, json: "
            "{1}",
            mId, connectionConfigure.dump());
        errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
        break;
      }

      auto srcElementIdIt =
          connectionConfigure.find(JSON_CONNECTION_SRC_ID_FIELD);
      if (connectionConfigure.end() == srcElementIdIt ||
          !srcElementIdIt->is_number_integer()) {
        IVS_ERROR(
            "Can not find {0} with integer type in connection json configure, "
            "graph id: {1:d}, json: {2}",
            JSON_CONNECTION_SRC_ID_FIELD, mId, connectionConfigure.dump());
        errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
        break;
      }

      int srcElementPort = 0;
      auto srcElementPortIt =
          connectionConfigure.find(JSON_CONNECTION_SRC_PORT_FIELD);
      if (connectionConfigure.end() != srcElementPortIt &&
          srcElementPortIt->is_number_integer()) {
        srcElementPort = srcElementPortIt->get<int>();
      }

      auto dstElementIdIt =
          connectionConfigure.find(JSON_CONNECTION_DST_ID_FIELD);
      if (connectionConfigure.end() == dstElementIdIt ||
          !dstElementIdIt->is_number_integer()) {
        IVS_ERROR(
            "Can not find {0} with integer type in connection json configure, "
            "graph id: {1:d}, json: {2}",
            JSON_CONNECTION_DST_ID_FIELD, mId, connectionConfigure.dump());
        errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
        break;
      }

      int dstElementPort = 0;
      auto dstElementPortIt =
          connectionConfigure.find(JSON_CONNECTION_DST_PORT_FIELD);
      if (connectionConfigure.end() != dstElementPortIt &&
          dstElementPortIt->is_number_integer()) {
        dstElementPort = dstElementPortIt->get<int>();
      }

      errorCode = connect(srcElementIdIt->get<int>(), srcElementPort,
                          dstElementIdIt->get<int>(), dstElementPort);

      if (common::ErrorCode::SUCCESS != errorCode) {
        break;
      }
    }
    if (common::ErrorCode::SUCCESS != errorCode) {
      break;
    }

  } while (false);

  IVS_INFO("Init connections finish, graph id: {0:d}, json: {1}", mId, json);
  return errorCode;
}

common::ErrorCode Graph::connect(int srcId, int srcPort, int dstId,
                                 int dstPort) {
  auto srcElementIt = mElementMap.find(srcId);
  if (mElementMap.end() == srcElementIt) {
    IVS_ERROR("Can not find element, graphd id: {0:d}, element id: {1:d}", mId,
              srcId);
    return common::ErrorCode::NO_SUCH_WORKER_ID;
  }

  auto srcElement = srcElementIt->second;
  if (!srcElement) {
    IVS_ERROR("Element is null, graph id: {0:d}, element id: {1:d}", mId,
              srcId);
    return common::ErrorCode::UNKNOWN;
  }

  auto dstElementIt = mElementMap.find(dstId);
  if (mElementMap.end() == dstElementIt) {
    IVS_ERROR(
        "Can not find element, graph id: {0:d}, element "
        "id: {1:d}",
        mId, dstId);
    return common::ErrorCode::NO_SUCH_WORKER_ID;
  }

  auto dstElement = dstElementIt->second;
  if (!dstElement) {
    IVS_ERROR("Can not find element, graph id: {0:d}, element id: {0:d}", mId,
              dstId);
    return common::ErrorCode::UNKNOWN;
  }

  framework::Element::connect(*srcElement, srcPort, *dstElement, dstPort);

  srcElement->afterConnect(false, true);
  dstElement->afterConnect(true, false);

  IVS_INFO("{0}~~~~~~~~~~~~~~~~~~{1}", srcId, dstId);

  return common::ErrorCode::SUCCESS;
}

void Graph::setSinkHandler(int elementId, int outputPort,
                           SinkHandler sinkHandler) {
  IVS_INFO(
      "Set data handler, graph id: {0:d}, element id: {1:d}, output port: "
      "{2:d}",
      mId, elementId, outputPort);

  auto elementIt = mElementMap.find(elementId);
  if (mElementMap.end() == elementIt) {
    IVS_ERROR("Can not find element, graph id: {0:d}, element id: {1:d}", mId,
              elementId);
    return;
  }

  auto element = elementIt->second;
  if (!element) {
    IVS_ERROR("Element is null, graph id: {0:d}, element id: {1:d}", mId,
              elementId);
    return;
  }

  element->setSinkHandler(outputPort, sinkHandler);
}

common::ErrorCode Graph::pushSourceData(int elementId, int inputPort,
                                        std::shared_ptr<void> data) {
  IVS_DEBUG(
      "send data, graph id: {0:d}, element id: {1:d}, input port: {2:d}, "
      "data: {3:p}",
      mId, elementId, inputPort, data.get());

  auto elementIt = mElementMap.find(elementId);
  if (mElementMap.end() == elementIt) {
    IVS_ERROR("Can not find element, graph id: {0:d}, element id: {1:d}", mId,
              elementId);
    return common::ErrorCode::NO_SUCH_WORKER_ID;
  }

  auto element = elementIt->second;
  if (!element) {
    IVS_ERROR("Element is null, graph id: {0:d}, element id: {1:d}", mId,
              elementId);
    return common::ErrorCode::UNKNOWN;
  }

  return element->pushInputData(inputPort, 0, data);
}

std::pair<std::string, int> Graph::getSideAndDeviceId(int elementId) {
  IVS_INFO("Get side and device id, graph id: {0:d}, element id: {1:d}", mId,
           elementId);

  auto elementIt = mElementMap.find(elementId);
  if (mElementMap.end() == elementIt) {
    IVS_ERROR("Can not find element, graph id: {0:d}, element id: {1:d}", mId,
              elementId);
    return std::make_pair("", -1);
  }

  auto element = elementIt->second;
  if (!element) {
    IVS_ERROR("Element is null, graph id: {0:d}, element id: {1:d}", mId,
              elementId);
    return std::make_pair("", -1);
  }

  return std::make_pair(element->getSide(), element->getId());
}

int Graph::getId() const { return mId; }
}  // namespace framework
}  // namespace sophon_stream
