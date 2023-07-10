#include "element.h"

#include <sys/prctl.h>

#include <iostream>
#include <nlohmann/json.hpp>

#include "common/logger.h"

namespace sophon_stream {
namespace framework {

void Element::connect(Element& srcElement, int srcElementPort,
                      Element& dstElement, int dstElementPort) {
  auto& inputConnector = dstElement.mInputConnectorMap[dstElementPort];
  if (!inputConnector) {
    inputConnector =
        std::make_shared<framework::Connector>(dstElement.getThreadNumber());
    IVS_DEBUG(
        "InputConnector initialized, mId = {0}, inputPort = {1}, dataPipeNum = "
        "{2}",
        dstElement.getId(), dstElementPort, dstElement.getThreadNumber());
  }
  dstElement.addInputPort(dstElementPort);
  srcElement.addOutputPort(srcElementPort);
  srcElement.mOutputConnectorMap[srcElementPort] = inputConnector;
}

Element::Element()
    : mId(-1),
      mDeviceId(-1),
      mThreadNumber(1),
      mThreadStatus(ThreadStatus::STOP) {}

Element::~Element() { uninit(); }

common::ErrorCode Element::init(const std::string& json) {
  IVS_INFO("Init start, json: {0}", json);

  common::ErrorCode errorCode = common::ErrorCode::SUCCESS;

  do {
    auto configure = nlohmann::json::parse(json, nullptr, false);
    if (!configure.is_object()) {
      IVS_ERROR("Parse json fail or json is not object, json: {0}", json);
      errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
      break;
    }

    auto idIt = configure.find(JSON_ID_FIELD);
    if (configure.end() == idIt || !idIt->is_number_integer()) {
      IVS_ERROR(
          "Can not find {0} with integer type in element json configure, json: "
          "{1}",
          JSON_ID_FIELD, json);
      errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
      break;
    }

    mId = idIt->get<int>();

    auto sideIt = configure.find(JSON_SIDE_FIELD);
    if (configure.end() != sideIt && sideIt->is_string()) {
      mSide = sideIt->get<std::string>();
    }

    auto sinkIt = configure.find(JSON_IS_SINK_FILED);
    if (configure.end() != sinkIt && sinkIt->is_boolean()) {
      mSinkElementFlag = sinkIt->get<bool>();
    }

    auto deviceIdIt = configure.find(JSON_DEVICE_ID_FIELD);
    if (configure.end() != deviceIdIt && deviceIdIt->is_number_integer()) {
      mDeviceId = deviceIdIt->get<int>();
    }

    auto threadNumberIt = configure.find(JSON_THREAD_NUMBER_FIELD);
    if (configure.end() != threadNumberIt &&
        threadNumberIt->is_number_integer()) {
      mThreadNumber = threadNumberIt->get<int>();
    }

    std::string internalConfigure;
    auto internalConfigureIt = configure.find(JSON_CONFIGURE_FIELD);
    if (configure.end() != internalConfigureIt) {
      internalConfigure = internalConfigureIt->dump();
    }

    errorCode = initInternal(internalConfigure);
    if (common::ErrorCode::SUCCESS != errorCode) {
      IVS_ERROR("Init internal fail, json: {0}", internalConfigure);
      break;
    }

  } while (false);

  if (common::ErrorCode::SUCCESS != errorCode) {
    uninit();
  }

  IVS_INFO("Init finish, json: {0}", json);
  return errorCode;
}

void Element::uninit() {
  int id = mId;
  IVS_INFO("Uninit start, element id: {0:d}", id);

  uninitInternal();

  mId = -1;
  mDeviceId = -1;
  mThreadNumber = 1;
  IVS_INFO("Uninit finish, element id: {0:d}", id);
}

common::ErrorCode Element::start() {
  IVS_INFO("Start element thread start, element id: {0:d}", mId);

  if (ThreadStatus::STOP != mThreadStatus) {
    IVS_ERROR("Can not start, current thread status is not stop");
    return common::ErrorCode::THREAD_STATUS_ERROR;
  }

  mThreadStatus = ThreadStatus::RUN;

  mThreads.reserve(mThreadNumber);
  for (int i = 0; i < mThreadNumber; ++i) {
    mThreads.push_back(
        std::make_shared<std::thread>(std::bind(&Element::run, this, i)));
  }

  IVS_INFO("Start element thread finish, element id: {0:d}", mId);
  return common::ErrorCode::SUCCESS;
}

common::ErrorCode Element::stop() {
  IVS_INFO("Stop element thread start, element id: {0:d}", mId);

  if (ThreadStatus::STOP == mThreadStatus) {
    IVS_ERROR("Can not stop, current thread status is stop");
    return common::ErrorCode::THREAD_STATUS_ERROR;
  }

  mThreadStatus = ThreadStatus::STOP;

  for (auto thread : mThreads) {
    thread->join();
  }
  mThreads.clear();

  IVS_INFO("Stop element thread finish, element id: {0:d}", mId);
  return common::ErrorCode::SUCCESS;
}

common::ErrorCode Element::pause() {
  IVS_INFO("Pause element thread start, element id: {0:d}", mId);

  if (ThreadStatus::RUN != mThreadStatus) {
    IVS_ERROR("Can not pause, current thread status is not run");
    return common::ErrorCode::THREAD_STATUS_ERROR;
  }

  mThreadStatus = ThreadStatus::PAUSE;

  IVS_INFO("Pause element thread finish, element id: {0:d}", mId);
  return common::ErrorCode::SUCCESS;
}

common::ErrorCode Element::resume() {
  IVS_INFO("Resume element thread start, element id: {0:d}", mId);

  if (ThreadStatus::PAUSE != mThreadStatus) {
    IVS_ERROR("Can not resume, current thread status is not pause");
    return common::ErrorCode::THREAD_STATUS_ERROR;
  }

  mThreadStatus = ThreadStatus::RUN;

  IVS_INFO("Resume element thread finish, element id: {0:d}", mId);
  return common::ErrorCode::SUCCESS;
}

void Element::run(int dataPipeId) {
  onStart();
  prctl(PR_SET_NAME, std::to_string(mId).c_str());
  while (ThreadStatus::RUN == mThreadStatus) {
    doWork(dataPipeId);
    std::this_thread::yield();
  }
  onStop();
}

common::ErrorCode Element::pushInputData(int inputPort, int dataPipeId,
                                         std::shared_ptr<void> data) {
  IVS_DEBUG("push data, element id: {0:d}, input port: {1:d}, data: {2:p}", mId,
            inputPort, data.get());

  auto& inputConnector = mInputConnectorMap[inputPort];
  if (!inputConnector) {
    inputConnector = std::make_shared<framework::Connector>(mThreadNumber);
    IVS_DEBUG(
        "InputConnector initialized, mId = {0}, inputPort = {1}, dataPipeNum = "
        "{2}",
        mId, inputPort, mThreadNumber);
  }
  return mInputConnectorMap[inputPort]->pushData(dataPipeId, data);
}

std::shared_ptr<void> Element::popInputData(int inputPort, int dataPipeId) {
  if (mInputConnectorMap[inputPort] == nullptr)
    mInputConnectorMap[inputPort] =
        std::make_shared<framework::Connector>(mThreadNumber);
  return mInputConnectorMap[inputPort]->popData(dataPipeId);
}

void Element::setSinkHandler(int outputPort, SinkHandler dataHandler) {
  IVS_INFO("Set data handler, element id: {0:d}, output port: {1:d}", mId,
           outputPort);
  if (mSinkElementFlag) mSinkHandlerMap[outputPort] = dataHandler;
}

common::ErrorCode Element::pushOutputData(int outputPort, int dataPipeId,
                                          std::shared_ptr<void> data) {
  IVS_DEBUG("send data, element id: {0:d}, output port: {1:d}, data:{2:p}", mId,
            outputPort, data.get());
  if (mSinkElementFlag) {
    auto handlerIt = mSinkHandlerMap.find(outputPort);
    if (mSinkHandlerMap.end() != handlerIt) {
      auto dataHandler = handlerIt->second;
      if (dataHandler) {
        dataHandler(data);
        return common::ErrorCode::SUCCESS;
      }
    }
  }
  while (mOutputConnectorMap[outputPort].lock()->pushData(dataPipeId, data) !=
         common::ErrorCode::SUCCESS) {
    IVS_DEBUG("DataPipe is full, now sleeping...");
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  return common::ErrorCode::SUCCESS;

  IVS_ERROR(
      "Can not find data handler or data pipe on output port, output port: "
      "{0:d}--element id:{1}",
      outputPort, mId);
  return common::ErrorCode::NO_SUCH_WORKER_PORT;
}

int Element::getOutputConnectorCapacity(int outputPort) {
  return mOutputConnectorMap[outputPort].lock()->getCapacity();
}

void Element::addInputPort(int port) { mInputPorts.push_back(port); }
void Element::addOutputPort(int port) { mOutputPorts.push_back(port); }

std::vector<int> Element::getInputPorts() { return mInputPorts; }
std::vector<int> Element::getOutputPorts() { return mOutputPorts; };

}  // namespace framework
}  // namespace sophon_stream
