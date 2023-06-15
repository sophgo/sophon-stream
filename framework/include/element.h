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

#include "common/error_code.h"
#include "common/logger.h"
#include "common/no_copyable.h"
#include "connector.h"
#include "datapipe.h"

namespace sophon_stream {
namespace framework {

class Element : public ::sophon_stream::common::NoCopyable {
 public:
  using SinkHandler = std::function<void(std::shared_ptr<void>)>;

  enum class ThreadStatus {
    STOP,
    RUN,
    PAUSE,
  };

  static void connect(Element& srcElement, int srcElementPort,
                      Element& dstElement, int dstElementPort);

  Element();

  virtual ~Element();

  common::ErrorCode init(const std::string& json);

  void uninit();

  common::ErrorCode start();

  common::ErrorCode stop();

  common::ErrorCode pause();

  common::ErrorCode resume();

  std::shared_ptr<void> popInputData(int inputPort, int dataPipeId);

  common::ErrorCode pushInputData(int inputPort, int dataPipeId,
                                  std::shared_ptr<void> data);

  std::shared_ptr<void> popOutputData(int outputPort, int dataPipeId) = delete;
  common::ErrorCode pushOutputData(int outputPort, int dataPipeId,
                                   std::shared_ptr<void> data);

  void setSinkHandler(int outputPort, SinkHandler sinkHandler);

  int getId() const { return mId; }

  const std::string& getSide() const { return mSide; }

  int getDeviceId() const { return mDeviceId; }

  int getThreadNumber() const { return mThreadNumber; }

  ThreadStatus getThreadStatus() const { return mThreadStatus; }

  bool getSinkElementFlag() const { return mSinkElementFlag; }

  static constexpr const char* JSON_ID_FIELD = "id";
  static constexpr const char* JSON_SIDE_FIELD = "side";
  static constexpr const char* JSON_DEVICE_ID_FIELD = "device_id";
  static constexpr const char* JSON_THREAD_NUMBER_FIELD = "thread_number";
  static constexpr const char* JSON_CONFIGURE_FIELD = "configure";
  static constexpr const char* JSON_IS_SINK_FILED = "is_sink";

 protected:
  virtual common::ErrorCode initInternal(const std::string& json) = 0;
  virtual void uninitInternal() = 0;

  virtual void onStart() {}
  virtual void onStop() {}

  void run(int dataPipeId);

  virtual common::ErrorCode doWork(int dataPipeId) = 0;

  std::vector<int> getInputPorts();
  std::vector<int> getOutputPorts();

  int getOutputConnectorCapacity(int outputPort);

 private:
  int mId;

  std::string mSide;

  int mDeviceId;

  int mThreadNumber;

  std::vector<std::shared_ptr<std::thread>> mThreads;

  std::atomic<ThreadStatus> mThreadStatus;

  // port_id -> shared_ptr::connector
  std::map<int, std::shared_ptr<framework::Connector>> mInputConnectorMap;
  // port_id -> weak_ptr::connector
  std::map<int, std::weak_ptr<framework::Connector>> mOutputConnectorMap;

  std::map<int /* outputPort */, SinkHandler> mSinkHandlerMap;

  std::vector<int> mInputPorts;
  std::vector<int> mOutputPorts;
  void addInputPort(int port);
  void addOutputPort(int port);

  bool mSinkElementFlag = false;
};

}  // namespace framework
}  // namespace sophon_stream

#endif  // SOPHON_STREAM_FRAMEWORK_ELEMENT_H_