//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_FRAMEWORK_GRAPH_H_
#define SOPHON_STREAM_FRAMEWORK_GRAPH_H_

#include <atomic>
#include <chrono>
#include <map>
#include <memory>
#include <string>

#include "common/error_code.h"
#include "common/logger.h"
#include "common/no_copyable.h"
#include "element.h"
#include "listen_thread.h"

namespace sophon_stream {
namespace framework {

class Graph : public ::sophon_stream::common::NoCopyable {
 public:
  using SinkHandler = framework::Element::SinkHandler;
  using ThreadStatus = framework::Element::ThreadStatus;

  Graph();

  ~Graph();

  /**
   * @brief 从配置文件初始化所有element和element之间的连接状态
   */
  common::ErrorCode init(const std::string& json);

  void uninit();

  common::ErrorCode start();

  common::ErrorCode stop();

  common::ErrorCode pause();

  common::ErrorCode resume();

  /**
   * @brief 向指定element推入数据，用于向decode
   * element发送启动任务的信号
   */
  common::ErrorCode pushSourceData(int elementId, int inputPort,
                                   std::shared_ptr<void> data);

  /**
   * @brief
   * 为指定element设置sinkHandler，sinkHandler当且仅当指定element是sink
   * element时才生效
   */
  void setSinkHandler(int elementId, int outputPort, SinkHandler sinkHandler);

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

  // friend class ListenThread;
  ListenThread* listenThreadPtr;

  int defaultPort = 8000;
};

}  // namespace framework
}  // namespace sophon_stream

#endif  // SOPHON_STREAM_FRAMEWORK_GRAPH_H_