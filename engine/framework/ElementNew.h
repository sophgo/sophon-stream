#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "DataPipeNew.h"
#include "common/ErrorCode.h"
#include "common/Logger.h"

namespace sophon_stream {
namespace framework {

/**
 * A base class define a mode receive data from input, process the data, and
 * send result to output.
 */
class Element {
 public:
  using DataHandler = std::function<void(std::shared_ptr<void>)>;

  /**
   * Thread status.
   */
  enum class ThreadStatus {
    STOP,
    RUN,
    PAUSE,
  };

  /**
   * Make connection between a Element and another Element.
   * @param[in,out] srcElement : Source Element.
   * @param[in] srcElementPort : Output port of source Element.
   * @param[in,out] dstElement : Destination Element.
   * @param[in] dstElementPort : Input port of destination Element.
   */
  static void connect(Element& srcElement, int srcElementPort,
                      Element& dstElement, int dstElementPort);

  /**
   * Constructor of class Element.
   */
  Element();

  /**
   * Destructor of class Element.
   */
  virtual ~Element();

  Element(const Element&) = delete;
  Element& operator=(const Element&) = delete;
  Element(Element&&) = default;
  Element& operator=(Element&&) = default;

  /**
   * Init Element with configure in json format.
   * @param[in] json : Configure in json format.
   * @return If parse configure fail, it will return error,
   * otherwise return common::ErrorCode::SUCESS.
   */
  common::ErrorCode init(const std::string& json);

  /**
   * Uninit Element, will stop Work.
   */
  void uninit();

  /**
   * Start all threads in this Element.
   * @return If thread status is not ThreadStatus::STOP,
   * it will return common::ErrorCode::THREAD_STATUS_ERROR,
   * otherwise return common::ErrorCode::SUCESS.
   */
  common::ErrorCode start();

  /**
   * Stop all threads in this Element.
   * @return If thread status is ThreadStatus::STOP,
   * it will return common::ErrorCode::THREAD_STATUS_ERROR,
   * otherwise return common::ErrorCode::SUCESS.
   */
  common::ErrorCode stop();

  /**
   * Pause all threads in this Element.
   * @return If thread status is not ThreadStatus::RUN,
   * it will return common::ErrorCode::THREAD_STATUS_ERROR,
   * otherwise return common::ErrorCode::SUCESS.
   */
  common::ErrorCode pause();

  /**
   * Resume all threads in this Element.
   * @return If thread status is not ThreadStatus::PAUSE,
   * it will return common::ErrorCode::THREAD_STATUS_ERROR,
   * otherwise return common::ErrorCode::SUCESS.
   */
  common::ErrorCode resume();

  /**
   * Push data to input port of this Element.
   * @param[in] inputPort : Input port of Element which will push data to.
   * @param[in] data : The data that will be push.
   * @param[in] timeout : The duration that will be wait for.
   * @return If timeout, it will return error,
   * otherwise return common::ErrorCode::SUCESS.
   */
  template <class Rep, class Period>
  common::ErrorCode pushData(
      int inputPort, std::shared_ptr<void> data,
      const std::chrono::duration<Rep, Period>& timeout) {
    IVS_DEBUG("push data, element id: {0:d}, input port: {1:d}, data: {2:p}",
              mId, inputPort, data.get());

    auto& inputDataPipe = mInputDataPipeMap[inputPort];
    if (!inputDataPipe) {
      inputDataPipe = std::make_shared<framework::DataPipe>();
      inputDataPipe->setPushHandler(std::bind(&Element::onInputNotify, this));
    }

    return inputDataPipe->pushData(data, timeout);
  }

  /**
   * Set callback to output port of this Element.
   * @param[in] outputPort : Output port of Element which will receive data.
   * @param[in] dataHandler : The callback that will be call with received data.
   */
  void setDataHandler(int outputPort, DataHandler dataHandler) {
    IVS_INFO("Set data handler, element id: {0:d}, output port: {1:d}", mId,
             outputPort);

    std::lock_guard<std::mutex> lk(mOutputHandlerMapMtx);
    mOutputHandlerMap[outputPort] = dataHandler;
  }

  /**
   * Get id of Element.
   * @return Return id of Element.
   */
  int getId() const { return mId; }

  /**
   * Get side of Element.
   * @return Return side of Element.
   */
  const std::string& getSide() const { return mSide; }

  /**
   * Get device id of Element.
   * @return Return device id of Element.
   */
  int getDeviceId() const { return mDeviceId; }

  /**
   * Get thread number of Element.
   * @return Return thread number of Element.
   */
  int getThreadNumber() const { return mThreadNumber; }

  /**
   * Get thread status of Element.
   * @return Return thread status of Element.
   */
  ThreadStatus getThreadStatus() const { return mThreadStatus; }

  /**
   * Get milliseconds timeout of Element.
   * @return Return milliseconds timeout of Element.
   */
  bool getMillisecondsTimeout() const { return mMillisecondsTimeout; }

  /**
   * Get repeated timeout of Element.
   * @return Return repeated timeout of Element.
   */
  bool getRepeatedTimeout() const { return mRepeatedTimeout; }

  static constexpr const char* JSON_ID_FIELD = "id";
  static constexpr const char* JSON_SIDE_FIELD = "side";
  static constexpr const char* JSON_DEVICE_ID_FIELD = "device_id";
  static constexpr const char* JSON_THREAD_NUMBER_FIELD = "thread_number";
  static constexpr const char* JSON_MILLISECONDS_TIMEOUT_FIELD =
      "milliseconds_timeout";
  static constexpr const char* JSON_REPEATED_TIMEOUT_FIELD = "repeated_timeout";
  static constexpr const char* JSON_CONFIGURE_FIELD = "configure";

 protected:
  virtual common::ErrorCode initInternal(const std::string& json) = 0;
  virtual void uninitInternal() = 0;

  virtual void onStart() {}
  virtual void onStop() {}

  /**
   * Thread function.
   */
  void run();

  /**
   * When push data to input port of this Element, will call this function in
   * run(). Each push data will cause one success call. If milliseconds timeout
   * is not 0, timeout will also call this function, and if repeated timeout is
   * true, this function will be call repeated with repeated timeout.
   */
  virtual common::ErrorCode doWork() = 0;

  /**
   * Get data count of specified input port, call by doWork() in derived class.
   * @param inputPort : Input port.
   * @return Return data count.
   */
  std::size_t getDataCount(int inputPort) const {
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

  /**
   * Get next data of specified input port, call by doWork() in derived class.
   * @param inputPort : Input port.
   * @return Return data.
   */
  std::shared_ptr<void> getData(int inputPort) const {
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

  /**
   * Pop next data of specified input port, call by doWork() in derived class.
   * @param inputPort : Input port.
   */
  void popData(int inputPort) {
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
  }

  /**
   * Send data to output port of this Element, call by doWork() in derived
   * class.
   * @param[in] outputPort : Output port of Element which will send data to.
   * @param[in] data : The data that will be send.
   * @param[in] timeout : The duration that will be wait for.
   * @return If can not find DataHandler or DataPipe on this output port or
   * timeout, it will return error, otherwise return common::ErrorCode::SUCESS.
   */
  template <class Rep, class Period>
  common::ErrorCode sendData(
      int outputPort, std::shared_ptr<void> data,
      const std::chrono::duration<Rep, Period>& timeout) {
    IVS_DEBUG("send data, element id: {0:d}, output port: {1:d}, data: {2:p}",
              mId, outputPort, data.get());
    {
      std::lock_guard<std::mutex> lk(mOutputHandlerMapMtx);
      auto handlerIt = mOutputHandlerMap.find(outputPort);
      if (mOutputHandlerMap.end() != handlerIt) {
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

  common::ErrorCode getOutputDatapipeCapacity(int outputPort, int& capacity) {
    auto dataPipeIt = mOutputDataPipeMap.find(outputPort);
    if (mOutputDataPipeMap.end() != dataPipeIt) {
      auto outputDataPipe = dataPipeIt->second.lock();
      if (outputDataPipe) {
        capacity = outputDataPipe->getCapacity();
        return common::ErrorCode::SUCCESS;
      }
    }
    return common::ErrorCode::NO_SUCH_WORKER_PORT;
  }

  common::ErrorCode getOutputDatapipeSize(int outputPort, int& size) {
    auto dataPipeIt = mOutputDataPipeMap.find(outputPort);
    if (mOutputDataPipeMap.end() != dataPipeIt) {
      auto outputDataPipe = dataPipeIt->second.lock();
      if (outputDataPipe) {
        size = outputDataPipe->getSize();
        return common::ErrorCode::SUCCESS;
      }
    }
    return common::ErrorCode::NO_SUCH_WORKER_PORT;
  }

 private:
  /**
   * When push data to any input DataPipe of this Element, the DataPipe will
   * call this function.
   */
  void onInputNotify();

  /**
   * Id of Element.
   */
  int mId;

  /**
   * Side of Element, used by derived class.
   */
  std::string mSide;

  /**
   * Device id of Element, used by derived class.
   */
  int mDeviceId;

  /**
   * Thread number of Element.
   */
  int mThreadNumber;

  /**
   * Thread objects of Element.
   */
  std::vector<std::shared_ptr<std::thread> > mThreads;

  /**
   * Thread status of Element.
   */
  std::atomic<ThreadStatus> mThreadStatus;

  /**
   * Milliseconds timeout of doWork() wait for input data.
   */
  int mMillisecondsTimeout;

  /**
   * Whether repeated timeout of doWork() wait for input data.
   */
  bool mRepeatedTimeout;

  /**
   * Mutex used by condition.
   */
  std::mutex mMutex;

  /**
   * Condition used by run() wait for input data.
   */
  std::condition_variable mCond;

  /**
   * Pending intput notify count, onInputNotify() increase the value and success
   * doWork() decrease the value.
   */
  std::atomic<int> mNotifyCount;

  /**
   * Input DataPipe map of Element, key is input port, value is std::shared_ptr
   * of framework::DataPipe.
   */
  std::map<int /* inputPort */, std::shared_ptr<framework::DataPipe> >
      mInputDataPipeMap;

  /**
   * Output DataHandler map of Element, key is output port, value is
   * Element::DataHandler.
   */
  std::map<int /* outputPort */, DataHandler> mOutputHandlerMap;
  std::mutex mOutputHandlerMapMtx;

  /**
   * Output DataPipe map of Element, key is output port, value is
   * std::shared_ptr of framework::DataPipe.
   */
  std::map<int /* outputPort */, std::weak_ptr<framework::DataPipe> >
      mOutputDataPipeMap;
};

}  // namespace framework
}  // namespace sophon_stream
