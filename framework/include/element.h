//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_FRAMEWORK_ELEMENT_H_
#define SOPHON_STREAM_FRAMEWORK_ELEMENT_H_

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

#include "common/ErrorCode.h"
#include "common/logger.h"
#include "datapipe.h"

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

  common::ErrorCode pushInputData(int inputPort, std::shared_ptr<void> data,
                             const std::chrono::milliseconds& timeout);

  void setStopHandler(int outputPort, DataHandler dataHandler);
  void setLastElementFlag();

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
  std::size_t getInputDataCount(int inputPort) const;

  std::shared_ptr<void> getInputData(int inputPort) const;

  void popInputData(int inputPort);

  common::ErrorCode pushOutputData(int outputPort, std::shared_ptr<void> data,
                             const std::chrono::milliseconds& timeout);

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

  std::vector<int> getInputPorts();
  std::vector<int> getOutputPorts();
  
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
   * Pending intput notify count, onInputNotify() increase the value and success
   * doWork() decrease the value.
   */
  std::atomic<int> mNotifyCount;

  /**
   * Mutex used by condition.
   */
  std::mutex mMutex;

  /**
   * Condition used by run() wait for input data.
   */
  std::condition_variable mCond;

  /**
   * Input DataPipe map of Element, key is input port, value is std::shared_ptr
   * of framework::DataPipe.
   */
  std::map<int /* inputPort */, std::shared_ptr<framework::DataPipe> >
      mInputDataPipeMap;

  /**
   * Output DataPipe map of Element, key is output port, value is
   * std::shared_ptr of framework::DataPipe.
   */
  std::map<int /* outputPort */, std::weak_ptr<framework::DataPipe> >
      mOutputDataPipeMap;

  /**
   * Output DataHandler map of Element, key is output port, value is
   * Element::DataHandler.
   */
  std::map<int /* outputPort */, DataHandler> mStopHandlerMap;

  std::vector<int> mInputPorts;
  std::vector<int> mOutputPorts;
  void addInputPort(int port);
  void addOutputPort(int port);

  bool mLastElementFlag = false;

};

}  // namespace framework
}  // namespace sophon_stream

#endif  // SOPHON_STREAM_FRAMEWORK_ELEMENT_H_