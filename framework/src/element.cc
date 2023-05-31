#include "element.h"

#include <sys/prctl.h>

#include <iostream>
#include <nlohmann/json.hpp>

#include "common/logger.h"

namespace sophon_stream {
namespace framework {

/**
 * Make connection between a Element and another Element.
 * @param[in,out] srcElement : Source Element.
 * @param[in] srcElementPort : Output port of source Element.
 * @param[in,out] dstElement : Destination Element.
 * @param[in] dstElementPort : Input port of destination Element.
 */
void Element::connect(Element& srcElement, int srcElementPort,
                      Element& dstElement, int dstElementPort) {
  auto& inputDataPipe = dstElement.mInputDataPipeMap[dstElementPort];
  if (!inputDataPipe) {
    inputDataPipe = std::make_shared<framework::DataPipe>();
    inputDataPipe->setPushHandler(
        std::bind(&Element::onInputNotify, &dstElement));
  }
  dstElement.addInputPort(dstElementPort);
  srcElement.addOutputPort(srcElementPort);
  srcElement.mOutputDataPipeMap[srcElementPort] = inputDataPipe;
}

/**
 * Constructor of class Element.
 */
Element::Element()
    : mId(-1),
      mDeviceId(-1),
      mThreadNumber(1),
      mThreadStatus(ThreadStatus::STOP),
      mNotifyCount(0),
      mMillisecondsTimeout(0),
      mRepeatedTimeout(false) {}

/**
 * Destructor of class Element.
 */
Element::~Element() {
  // stop();
}

/**
 * Init Element with configure in json format.
 * @param[in] json : Configure in json format.
 * @return If parse configure fail, it will return error,
 * otherwise return common::ErrorCode::SUCESS.
 */
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

    auto deviceIdIt = configure.find(JSON_DEVICE_ID_FIELD);
    if (configure.end() != deviceIdIt && deviceIdIt->is_number_integer()) {
      mDeviceId = deviceIdIt->get<int>();
    }

    auto threadNumberIt = configure.find(JSON_THREAD_NUMBER_FIELD);
    if (configure.end() != threadNumberIt &&
        threadNumberIt->is_number_integer()) {
      mThreadNumber = threadNumberIt->get<int>();
    }

    auto millisecondsTimeoutIt =
        configure.find(JSON_MILLISECONDS_TIMEOUT_FIELD);
    if (configure.end() != millisecondsTimeoutIt &&
        millisecondsTimeoutIt->is_number_integer()) {
      mMillisecondsTimeout = millisecondsTimeoutIt->get<int>();
    }

    auto repeatedTimeoutIt = configure.find(JSON_REPEATED_TIMEOUT_FIELD);
    if (configure.end() != repeatedTimeoutIt &&
        repeatedTimeoutIt->is_boolean()) {
      mRepeatedTimeout = repeatedTimeoutIt->get<bool>();
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

/**
 * Uninit Element, will stop Work.
 */
void Element::uninit() {
  int id = mId;
  IVS_INFO("Uninit start, element id: {0:d}", id);

  // stop();

  uninitInternal();

  mId = -1;
  mDeviceId = -1;
  mThreadNumber = 1;
  mMillisecondsTimeout = 0;
  mRepeatedTimeout = false;

  IVS_INFO("Uninit finish, element id: {0:d}", id);
}

/**
 * Start all threads in this Element.
 * @return If thread status is not ThreadStatus::STOP,
 * it will return common::ErrorCode::THREAD_STATUS_ERROR,
 * otherwise return common::ErrorCode::SUCESS.
 */
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
        std::make_shared<std::thread>(std::bind(&Element::run, this)));
  }

  IVS_INFO("Start element thread finish, element id: {0:d}", mId);
  return common::ErrorCode::SUCCESS;
}

/**
 * Stop all threads in this Element.
 * @return If thread status is ThreadStatus::STOP,
 * it will return common::ErrorCode::THREAD_STATUS_ERROR,
 * otherwise return common::ErrorCode::SUCESS.
 */
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

/**
 * Pause all threads in this Element.
 * @return If thread status is not ThreadStatus::RUN,
 * it will return common::ErrorCode::THREAD_STATUS_ERROR,
 * otherwise return common::ErrorCode::SUCESS.
 */
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

/**
 * Resume all threads in this Element.
 * @return If thread status is not ThreadStatus::PAUSE,
 * it will return common::ErrorCode::THREAD_STATUS_ERROR,
 * otherwise return common::ErrorCode::SUCESS.
 */
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

constexpr const char* Element::JSON_ID_FIELD;
constexpr const char* Element::JSON_SIDE_FIELD;
constexpr const char* Element::JSON_DEVICE_ID_FIELD;
constexpr const char* Element::JSON_THREAD_NUMBER_FIELD;
constexpr const char* Element::JSON_MILLISECONDS_TIMEOUT_FIELD;
constexpr const char* Element::JSON_REPEATED_TIMEOUT_FIELD;
constexpr const char* Element::JSON_CONFIGURE_FIELD;

static constexpr int DEFAULT_MILLISECONDS_TIMEOUT = 200;
/**
 * Thread function.
 */
void Element::run() {
  onStart();
  prctl(PR_SET_NAME, std::to_string(mId).c_str());
  {
    bool currentNoTimeout = true;
    bool lastNoTimeout = true;
    while (ThreadStatus::STOP != mThreadStatus) {
      std::chrono::milliseconds millisecondsTimeout(
          DEFAULT_MILLISECONDS_TIMEOUT);
      if (ThreadStatus::PAUSE != mThreadStatus && 0 != mMillisecondsTimeout) {
        millisecondsTimeout = std::chrono::milliseconds(mMillisecondsTimeout);
      }

      lastNoTimeout = currentNoTimeout;
      {
        std::unique_lock<std::mutex> lock(mMutex);

        currentNoTimeout = mCond.wait_for(
            lock, millisecondsTimeout, [this]() { return mNotifyCount > 0; });
      }

      if (ThreadStatus::PAUSE == mThreadStatus ||
          (!currentNoTimeout && (0 == mMillisecondsTimeout ||
                                 (!mRepeatedTimeout && !lastNoTimeout)))) {
        // 上一次timeout: continue, 上一次noTimeout: dowork
        // std::cout<<static_cast<int>(mThreadStatus.load())<<"--"
        // <<currentNoTimeout<<"--"
        // <<mMillisecondsTimeout
        // <<"--"<<mRepeatedTimeout<<"--"<<lastNoTimeout<<std::endl;
        continue;
      }
      if (currentNoTimeout) doWork();
    }
  }

  onStop();
}

common::ErrorCode Element::pushInputData(
    int inputPort, std::shared_ptr<void> data,
    const std::chrono::milliseconds& timeout) {
  IVS_DEBUG("push data, element id: {0:d}, input port: {1:d}, data: {2:p}", mId,
            inputPort, data.get());

  auto& inputDataPipe = mInputDataPipeMap[inputPort];
  if (!inputDataPipe) {
    inputDataPipe = std::make_shared<framework::DataPipe>();
    inputDataPipe->setPushHandler(std::bind(&Element::onInputNotify, this));
  }

  return inputDataPipe->pushData(data, timeout);
}

std::shared_ptr<void> Element::getInputData(int inputPort) const {
  auto dataPipeIt = mInputDataPipeMap.find(inputPort);
  if (mInputDataPipeMap.end() == dataPipeIt) {
    return std::shared_ptr<void>();
  }

  auto inputDataPipe = dataPipeIt->second;
  if (!inputDataPipe) {
    return std::shared_ptr<void>();
  }

  return inputDataPipe->getData();
}

void Element::popInputData(int inputPort) {
  IVS_DEBUG("pop data, element id: {0:d}, input port: {1:d}", mId, inputPort);

  auto dataPipeIt = mInputDataPipeMap.find(inputPort);
  if (mInputDataPipeMap.end() == dataPipeIt) {
    return;
  }

  auto inputDataPipe = dataPipeIt->second;
  if (!inputDataPipe) {
    return;
  }

  inputDataPipe->popData();
  mNotifyCount--;
}

void Element::setStopHandler(int outputPort, DataHandler dataHandler) {
  IVS_INFO("Set data handler, element id: {0:d}, output port: {1:d}", mId,
           outputPort);

  mStopHandlerMap[outputPort] = dataHandler;
}

common::ErrorCode Element::pushOutputData(
    int outputPort, std::shared_ptr<void> data,
    const std::chrono::milliseconds& timeout) {
  IVS_DEBUG("send data, element id: {0:d}, output port: {1:d}, data:{2:p}", mId,
            outputPort, data.get());
  if (mLastElementFlag) {
    auto handlerIt = mStopHandlerMap.find(outputPort);
    if (mStopHandlerMap.end() != handlerIt) {
      auto dataHandler = handlerIt->second;
      if (dataHandler) {
        dataHandler(data);
        return common::ErrorCode::SUCCESS;
      }
    }
  }

  auto dataPipeIt = mOutputDataPipeMap.find(outputPort);
  if (mOutputDataPipeMap.end() != dataPipeIt) {
    auto outputDataPipe = dataPipeIt->second.lock();
    if (outputDataPipe) {
      return outputDataPipe->pushData(data, timeout);
    }
  }

  IVS_ERROR(
      "Can not find data handler or data pipe on output port, output port: "
      "{0:d}--element id:{1}",
      outputPort, mId);
  return common::ErrorCode::NO_SUCH_WORKER_PORT;
}

/**
 * When push data to any input DataPipe of this Element, the DataPipe will call
 * this function.
 */
void Element::onInputNotify() {
  ++mNotifyCount;
  mCond.notify_one();
}

void Element::addInputPort(int port) { mInputPorts.push_back(port); }
void Element::addOutputPort(int port) { mOutputPorts.push_back(port); }

std::vector<int> Element::getInputPorts() { return mInputPorts; }
std::vector<int> Element::getOutputPorts() { return mOutputPorts; };

void Element::setLastElementFlag() { mLastElementFlag = true; }

std::size_t Element::getInputDataCount(int inputPort) const {
  auto dataPipeIt = mInputDataPipeMap.find(inputPort);
  if (mInputDataPipeMap.end() == dataPipeIt) {
    return 0;
  }

  auto inputDataPipe = dataPipeIt->second;
  if (!inputDataPipe) {
    return 0;
  }

  return inputDataPipe->getSize();
}

}  // namespace framework
}  // namespace sophon_stream
