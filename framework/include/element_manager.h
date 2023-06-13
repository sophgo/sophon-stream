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

#include "common/error_code.h"
#include "common/logger.h"
#include "common/no_copyable.h"
#include "element.h"

namespace sophon_stream {
namespace framework {

class ElementManager : public ::sophon_stream::common::NoCopyable {
 public:
  using DataHandler = framework::Element::DataHandler;
  using ThreadStatus = framework::Element::ThreadStatus;

  ElementManager();

  ~ElementManager();

  common::ErrorCode init(const std::string& json);

  void uninit();

  common::ErrorCode start();

  common::ErrorCode stop();

  common::ErrorCode pause();

  common::ErrorCode resume();

  common::ErrorCode pushInputData(int elementId, int inputPort,
                                  std::shared_ptr<void> data);

  void setStopHandler(int elementId, int outputPort, DataHandler dataHandler);

  std::pair<std::string, int> getSideAndDeviceId(int elementId);

  int getId() const;

  static constexpr const char* JSON_GRAPH_ID_FIELD = "graph_id";
  static constexpr const char* JSON_WORKERS_FIELD = "elements";
  static constexpr const char* JSON_CONNECTIONS_FIELD = "connections";
  static constexpr const char* JSON_MODEL_SHARED_OBJECT_FIELD = "shared_object";
  static constexpr const char* JSON_WORKER_NAME_FIELD = "name";
  static constexpr const char* JSON_CONNECTION_SRC_ID_FIELD = "src_id";
  static constexpr const char* JSON_CONNECTION_SRC_PORT_FIELD = "src_port";
  static constexpr const char* JSON_CONNECTION_DST_ID_FIELD = "dst_id";
  static constexpr const char* JSON_CONNECTION_DST_PORT_FIELD = "dst_port";

 private:
  common::ErrorCode initElements(const std::string& json);
  common::ErrorCode initConnections(const std::string& json);
  common::ErrorCode connect(int srcId, int srcPort, int dstId, int dstPort);

  int mId;

  std::atomic<ThreadStatus> mThreadStatus;

  std::vector<std::shared_ptr<void> > mSharedObjectHandles;

  std::map<int /* elementId */, std::shared_ptr<framework::Element> >
      mElementMap;
};

}  // namespace framework
}  // namespace sophon_stream

#endif  // SOPHON_STREAM_FRAMEWORK_ELEMENT_MANAGER_H_