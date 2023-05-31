//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_FRAMEWORK_ELEMENT_MANAGER_H_
#define SOPHON_STREAM_FRAMEWORK_ELEMENT_MANAGER_H_

#include <atomic>
#include <chrono>
#include <map>
#include <memory>
#include <string>

#include "element.h"
#include "common/ErrorCode.h"
#include "common/logger.h"

namespace sophon_stream {
namespace framework {

/**
 * A class manages a lot of Elements, some of those Elements will connect with
 * others. A ElementManager is just a graph, a Element is just a node.
 */
class ElementManager {
 public:
  using DataHandler = framework::Element::DataHandler;
  using ThreadStatus = framework::Element::ThreadStatus;

  /**
   * Constructor of class ElementManager.
   */
  ElementManager();

  /**
   * Destructor of class ElementManager.
   */
  ~ElementManager();

  ElementManager(const ElementManager&) = delete;
  ElementManager& operator=(const ElementManager&) = delete;
  ElementManager(ElementManager&&) = default;
  ElementManager& operator=(ElementManager&&) = default;

  /**
   * Init ElementManager with configure in json format.
   * @param[in] json : Configure in json format.
   * @return If parse configure fail, it will return error,
   * otherwise return common::ErrorCode::SUCESS.
   */
  common::ErrorCode init(const std::string& json);

  /**
   * Uninit ElementManager, will stop WorkManager.
   */
  void uninit();

  /**
   * Start all threads of Elements in this ElementManager.
   * @return If thread status is not ThreadStatus::STOP,
   * it will return common::ErrorCode::THREAD_STATUS_ERROR,
   * otherwise return common::ErrorCode::SUCESS.
   */
  common::ErrorCode start();

  /**
   * Stop all threads of Elements in this ElementManager.
   * @return If thread status is ThreadStatus::STOP,
   * it will return common::ErrorCode::THREAD_STATUS_ERROR,
   * otherwise return common::ErrorCode::SUCESS.
   */
  common::ErrorCode stop();

  /**
   * Pause all threads of Elements in this ElementManager.
   * @return If thread status is not ThreadStatus::RUN,
   * it will return common::ErrorCode::THREAD_STATUS_ERROR,
   * otherwise return common::ErrorCode::SUCESS.
   */
  common::ErrorCode pause();

  /**
   * Resume all threads of Elements in this ElementManager.
   * @return If thread status is not ThreadStatus::PAUSE,
   * it will return common::ErrorCode::THREAD_STATUS_ERROR,
   * otherwise return common::ErrorCode::SUCESS.
   */
  common::ErrorCode resume();

  /**
   * Send data to a Element in this ElementManager with a input port.
   * @param[in] elementId : Id of Element which will send data to.
   * @param[in] inputPort : Input port of Element which will send data to.
   * @param[in] data : The data that will be send.
   * @param[in] timeout : The duration that will be wait for.
   * @return If element id is not exist or timeout, it will return error,
   * otherwise return common::ErrorCode::SUCESS.
   */
  common::ErrorCode sendData(
      int elementId, int inputPort, std::shared_ptr<void> data,
      const std::chrono::milliseconds& timeout) {
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

    return element->pushData(inputPort, data, timeout);
  }

  void setStopHandler(int elementId, int outputPort, DataHandler dataHandler);

  std::pair<std::string, int> getSideAndDeviceId(int elementId) {
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

  /**
   * Get id of ElementManager.
   * @return Return id of ElementManager.
   */
  int getId() const { return mId; }

  static constexpr const char* JSON_GRAPH_ID_FIELD = "graph_id";

 private:
  /**
   * Information of Module.
   */
  struct Module {
    /**
     * Constructor of struct Module.
     */
    Module();

    /**
     * Get first element id of Module.
     * @return Return first element id of Module.
     */
    int getFirstElementId() const;

    /**
     * Get last element id of Module.
     * @return Return last element id of Module.
     */
    int getLastElementId() const;

    /**
     * Id of Module.
     */
    int mId;

    /**
     * Id of PreModuleElement in this Module.
     */
    int mPreModuleElementId;

    /**
     * Id of PostModuleElement in this Module.
     */
    int mPostModuleElementId;

    /**
     * Ids of PreElements in this Module.
     */
    std::vector<int> mPreElementIds;

    /**
     * Ids of PostElements in this Module.
     */
    std::vector<int> mPostElementIds;

    /**
     * Ids of SubModules in this Module.
     */
    std::vector<int> mSubModuleIds;
  };

  /**
   * Init Elements with configure in json format.
   * @param[in] json : Configure in json format.
   * @return If parse configure fail, it will return error,
   * otherwise return common::ErrorCode::SUCESS.
   */
  common::ErrorCode initElements(const std::string& json);

  /**
   * Init Modules with configure in json format.
   * @param[in] json : Configure in json format.
   * @return If parse configure fail, it will return error,
   * otherwise return common::ErrorCode::SUCESS.
   */
  common::ErrorCode initModules(const std::string& json);

  /**
   * Init Connections with configure in json format.
   * @param[in] json : Configure in json format.
   * @return If parse configure fail, it will return error,
   * otherwise return common::ErrorCode::SUCESS.
   */
  common::ErrorCode initConnections(const std::string& json);
  /**
   * Make a connectiton between a Element/Module and another Element/Module.
   * @param[in] srcId : Id of source Element/Module,
   * if it is Module's id, will use last Element of the Module instead.
   * @param[in] srcPort : Output port of source Element or last Element of
   * source Module.
   * @param[in] dstId : Id of destination Element/Module,
   * if it is Module's id, will use first Element of the Module instead.
   * @param[in] dstPort : Input port of destination Element or first Element of
   * destination Module.
   * @return If can not find Element, Module or Element of Module, it will
   * return error, otherwise return common::ErrorCode::SUCESS.
   */
  common::ErrorCode connect(int srcId, int srcPort, int dstId, int dstPort);

  /**
   * Id of ElementManager.
   */
  int mId;

  /**
   * Thread status of all Elements in ElementManager.
   */
  std::atomic<ThreadStatus> mThreadStatus;

  /**
   * @brief 要加载的so句柄集合
   */
  std::vector<std::shared_ptr<void> > mSharedObjectHandles;

  /**
   * Element map of ElementManager, key is id of element, value is
   * std::shared_ptr of framework::Element.
   */
  std::map<int /* elementId */, std::shared_ptr<framework::Element> >
      mElementMap;

};

}  // namespace framework
}  // namespace sophon_stream

#endif // SOPHON_STREAM_FRAMEWORK_ELEMENT_MANAGER_H_