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
  /**
   * @brief sink element的数据处理函数
   */
  using SinkHandler = std::function<void(std::shared_ptr<void>)>;

  /**
   * @brief 线程工作状态
   */
  enum class ThreadStatus {
    STOP,
    RUN,
    PAUSE,
  };

  /**
   * @brief
   * 连接两个element，初始化connector并填入srcElement的mOutputConnectorMap
   * @param[in,out] srcElement : Source element
   * @param[in] srcElementPort : Output port of source element
   * @param[in,out] dstElement : Destination element
   * @param[in] dstElementPort : Input port of destination element
   */
  static void connect(Element& srcElement, int srcElementPort,
                      Element& dstElement, int dstElementPort);

  Element();

  virtual ~Element();

  /**
   * @brief 从配置文件初始化element的通用属性
   * @param[in] json : json格式的配置文件
   * @return 如果解析失败，会返回error，否则返回common::ErrorCode::SUCCESS
   */
  common::ErrorCode init(const std::string& json);

  common::ErrorCode start();

  common::ErrorCode stop();

  common::ErrorCode pause();

  common::ErrorCode resume();

  /**
   * @brief 从指定inputPort的指定dataPipe中弹出数据，用于数据处理阶段
   * @brief 若队列为空，返回nullptr
   */
  std::shared_ptr<void> popInputData(int inputPort, int dataPipeId);

  /**
   * @brief 向指定inputPort的指定dataPipe推入数据，用于启动解码任务
   * @param[in] data : sophon_stream::element::decode::ChannelTask结构体指针
   */
  common::ErrorCode pushInputData(int inputPort, int dataPipeId,
                                  std::shared_ptr<void> data);

  std::shared_ptr<void> popOutputData(int outputPort, int dataPipeId) = delete;

  /**
   * @brief 向指定outputPort的指定dataPipe推入数据，将数据传递给下一个element
   * @brief 如果当前element是sink element，那么改为使用sinkHandler处理数据
   */
  common::ErrorCode pushOutputData(int outputPort, int dataPipeId,
                                   std::shared_ptr<void> data);

  void setSinkHandler(int outputPort, SinkHandler sinkHandler);

  /**
   * @brief 在执行connect()之后进行，只有Group
   * Element才需要override该函数，用于将group的输入输出与内部preElement和postElement连接起来
   *
   * @param is_dst 对应connect方法里的dst
   * @param is_src 对应connect方法里的dst
   */
  virtual void afterConnect(bool is_dst, bool is_src) {}

  virtual bool getGroup() { return false; }

  /**
   * @brief 仅group element重写，用于向graph的elementMap注册内部各个element
   * @param mapPtr graph的elementMap
   */
  virtual void groupInsert(
      std::map<int, std::shared_ptr<framework::Element>>& mapPtr) {}

  int getId() const { return mId; }

  const std::string& getSide() const { return mSide; }

  int getDeviceId() const { return mDeviceId; }

  int getThreadNumber() const { return mThreadNumber; }

  ThreadStatus getThreadStatus() const { return mThreadStatus; }

  bool getSinkElementFlag() const { return mSinkElementFlag; }

  std::weak_ptr<framework::Connector> getOutputConnector(int outputPort) {
    return mOutputConnectorMap[outputPort];
  };
  std::weak_ptr<framework::Connector> getInputConnector(int inputPort) {
    return mInputConnectorMap[inputPort];
  };

  inline void setId(const int id) { mId = id; }
  inline void setSide(const std::string side) { mSide = side; }
  inline void setSinkFlag(const bool flag) { mSinkElementFlag = flag; }
  inline void setDeviceId(const int id) { mDeviceId = id; }
  inline void setThreadNumber(const int num) { mThreadNumber = num; }

  static constexpr const char* JSON_ID_FIELD = "id";
  static constexpr const char* JSON_SIDE_FIELD = "side";
  static constexpr const char* JSON_DEVICE_ID_FIELD = "device_id";
  static constexpr const char* JSON_THREAD_NUMBER_FIELD = "thread_number";
  static constexpr const char* JSON_CONFIGURE_FIELD = "configure";
  static constexpr const char* JSON_IS_SINK_FILED = "is_sink";
  static constexpr const char* JSON_INNER_ELEMENTS_ID = "inner_elements_id";

  std::map<int, std::shared_ptr<framework::Connector>>& getInputConnectorMap() {
    return mInputConnectorMap;
  }
  std::map<int, std::weak_ptr<framework::Connector>>& getOutputConnectorMap() {
    return mOutputConnectorMap;
  }

  void setInputConnectorMap(
      std::map<int, std::shared_ptr<framework::Connector>>& input) {
    mInputConnectorMap = input;
  }

  void setOutputConnectorMap(
      std::map<int, std::weak_ptr<framework::Connector>>& input) {
    mOutputConnectorMap = input;
  }

  void addInputPort(int port);
  void addOutputPort(int port);

 protected:
  /**
   * @brief 从配置文件初始化某个派生element的特有属性
   * @param[in] json : json格式的配置文件
   * @return 如果解析失败，会返回error，否则返回common::ErrorCode::SUCCESS
   */
  virtual common::ErrorCode initInternal(const std::string& json) = 0;

  virtual void onStart() {}
  virtual void onStop() {}

  /**
   * @brief 线程函数，循环调用doWork()，分配CPU时间片资源
   * @param[in] dataPipeId :
   * 每个线程都与一个datapipe绑定，只处理来自该datapipe的数据
   */
  void run(int dataPipeId);

  /**
   * @brief 派生element中实现自身功能
   */
  virtual common::ErrorCode doWork(int dataPipeId) = 0;

  std::vector<int> getInputPorts();
  std::vector<int> getOutputPorts();

  /**
   * @brief 获取指定outputPort对应的Connector中datapipe的数量
   */
  int getOutputConnectorCapacity(int outputPort);
  /**
   * @brief 获取指定inputPort对应的Connector中datapipe的数量
   */
  int getInputConnectorCapacity(int inputPort);

 private:
  int mId;

  std::string mSide;

  int mDeviceId;

  int mThreadNumber;

  std::vector<std::shared_ptr<std::thread>> mThreads;

  std::atomic<ThreadStatus> mThreadStatus;

  /**
   * @brief inputPort到inputConnector的映射
   * @brief inputConnector的生命周期由当前element管理
   */
  std::map<int, std::shared_ptr<framework::Connector>> mInputConnectorMap;
  /**
   * @brief outputPort到outputConnector的映射
   * @brief outputConnector的生命周期由下一个element管理
   */
  std::map<int, std::weak_ptr<framework::Connector>> mOutputConnectorMap;

  /**
   * @brief outputPort到sinkHandler的映射
   */
  std::map<int, SinkHandler> mSinkHandlerMap;

  std::vector<int> mInputPorts;
  std::vector<int> mOutputPorts;

  bool mSinkElementFlag = false;
};

}  // namespace framework
}  // namespace sophon_stream

#endif  // SOPHON_STREAM_FRAMEWORK_ELEMENT_H_