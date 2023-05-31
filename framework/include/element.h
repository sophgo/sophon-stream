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

class Element {
 public:
  using DataHandler = std::function<void(std::shared_ptr<void>)>;

  enum class ThreadStatus {
    STOP,
    RUN,
    PAUSE,
  };

  static void connect(Element& srcElement, int srcElementPort,
                      Element& dstElement, int dstElementPort);

  Element();

  virtual ~Element();

  Element(const Element&) = delete;
  Element& operator=(const Element&) = delete;
  Element(Element&&) = default;
  Element& operator=(Element&&) = default;

  common::ErrorCode init(const std::string& json);

  void uninit();

  common::ErrorCode start();

  common::ErrorCode stop();

  common::ErrorCode pause();

  common::ErrorCode resume();

  common::ErrorCode pushInputData(int inputPort, std::shared_ptr<void> data,
                                  const std::chrono::milliseconds& timeout);

  void setStopHandler(int outputPort, DataHandler dataHandler);
  void setLastElementFlag();

  int getId() const { return mId; }

  const std::string& getSide() const { return mSide; }

  int getDeviceId() const { return mDeviceId; }

  int getThreadNumber() const { return mThreadNumber; }

  ThreadStatus getThreadStatus() const { return mThreadStatus; }

  bool getMillisecondsTimeout() const { return mMillisecondsTimeout; }

  bool getRepeatedTimeout() const { return mRepeatedTimeout; }

  static constexpr const char* JSON_ID_FIELD = "id";
  static constexpr const char* JSON_SIDE_FIELD = "side";
  static constexpr const char* JSON_DEVICE_ID_FIELD = "device_id";
  static constexpr const char* JSON_THREAD_NUMBER_FIELD = "thread_number";
  static constexpr const char* JSON_MILLISECONDS_TIMEOUT_FIELD =
      "milliseconds_timeout";
  static constexpr const char* JSON_REPEATED_TIMEOUT_FIELD = "repeated_timeout";
  static constexpr const char* JSON_CONFIGURE_FIELD = "configure";
  static constexpr const int DEFAULT_MILLISECONDS_TIMEOUT = 200;

 protected:
  virtual common::ErrorCode initInternal(const std::string& json) = 0;
  virtual void uninitInternal() = 0;

  virtual void onStart() {}
  virtual void onStop() {}

  void run();

  virtual common::ErrorCode doWork() = 0;

  std::size_t getInputDataCount(int inputPort) const;

  std::shared_ptr<void> getInputData(int inputPort) const;

  void popInputData(int inputPort);

  common::ErrorCode pushOutputData(int outputPort, std::shared_ptr<void> data,
                                   const std::chrono::milliseconds& timeout);

  common::ErrorCode getOutputDatapipeCapacity(int outputPort, int& capacity);

  common::ErrorCode getOutputDatapipeSize(int outputPort, int& size);

  std::vector<int> getInputPorts();
  std::vector<int> getOutputPorts();

 private:
  void onInputNotify();

  int mId;

  std::string mSide;

  int mDeviceId;

  int mThreadNumber;

  std::vector<std::shared_ptr<std::thread> > mThreads;

  std::atomic<ThreadStatus> mThreadStatus;

  int mMillisecondsTimeout;

  bool mRepeatedTimeout;

  std::atomic<int> mNotifyCount;

  std::mutex mMutex;

  std::condition_variable mCond;

  std::map<int /* inputPort */, std::shared_ptr<framework::DataPipe> >
      mInputDataPipeMap;

  std::map<int /* outputPort */, std::weak_ptr<framework::DataPipe> >
      mOutputDataPipeMap;

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