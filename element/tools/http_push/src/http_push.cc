//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#include "http_push.h"

#include "common/common_defs.h"
#include "common/logger.h"
#include "common/serialize.h"
#include "element_factory.h"

namespace sophon_stream {
namespace element {
namespace http_push {
HttpPush::HttpPush() {}
HttpPush::~HttpPush() {
  for (auto [k, v] : mapImpl_) {
    v->release();
  }
}

common::ErrorCode HttpPush::initInternal(const std::string& json) {
  common::ErrorCode errorCode = common::ErrorCode::SUCCESS;
  do {
    auto configure = nlohmann::json::parse(json, nullptr, false);
    if (!configure.is_object()) {
      errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
      break;
    }
    auto ipIt = configure.find(CONFIG_INTERNAL_IP_FILED);
    STREAM_CHECK((ipIt != configure.end() && ipIt->is_string()),
                 "IP must be std::string, please check your http_push element "
                 "configuration file");
    ip_ = ipIt->get<std::string>();
    auto portIt = configure.find(CONFIG_INTERNAL_PORT_FILED);
    STREAM_CHECK((portIt != configure.end() && portIt->is_number_integer()),
                 "Port must be integer, please check your http_push element "
                 "configuration file");
    port_ = portIt->get<int>();

    auto pathIt = configure.find(CONFIG_INTERNAL_PATH_FILED);
    STREAM_CHECK((pathIt != configure.end() && pathIt->is_string()),
                 "Port must be string, please check your http_push element "
                 "configuration file");
    path_ = pathIt->get<std::string>();

  } while (false);
  return errorCode;
}

HttpPushImpl_::HttpPushImpl_(std::string& ip, int port, std::string path_,
                             int channel)
    : cli(ip, port), path(path_) {
  workThread = std::thread(&HttpPushImpl_::postFunc, this);
  mFpsProfilerName = "http_push_" + std::to_string(channel) + "_fps";
  mFpsProfiler.config(mFpsProfilerName, 100);
}

void HttpPushImpl_::release() {
  isRunning = false;
  workThread.join();
}

void HttpPushImpl_::postFunc() {
  while (isRunning) {
    auto ptr = popQueue();
    if (ptr == nullptr) {
      std::this_thread::sleep_for(std::chrono::milliseconds(5));
      continue;
    }
    mFpsProfiler.add(1);
    cli.Post(path.c_str(), ptr->dump(), "application/json");
  }
}

bool HttpPushImpl_::pushQueue(std::shared_ptr<nlohmann::json> j) {
  int len = getQueueSize();
  if (len >= maxQueueLen) return false;
  {
    std::lock_guard<std::mutex> lock(mtx);
    objQueue.push(j);
  }
  return true;
}

std::shared_ptr<nlohmann::json> HttpPushImpl_::popQueue() {
  std::lock_guard<std::mutex> lock(mtx);
  std::shared_ptr<nlohmann::json> j = nullptr;
  if (objQueue.empty()) return j;
  j = objQueue.front();
  objQueue.pop();
  return j;
}

size_t HttpPushImpl_::getQueueSize() {
  int len = -1;
  {
    std::lock_guard<std::mutex> lock(mtx);
    len = objQueue.size();
  }
  return len;
}

common::ErrorCode HttpPush::doWork(int dataPipeId) {
  std::vector<int> inputPorts = getInputPorts();
  int inputPort = inputPorts[0];
  int outputPort = 0;
  if (!getSinkElementFlag()) {
    std::vector<int> outputPorts = getOutputPorts();
    outputPort = outputPorts[0];
  }

  auto data = popInputData(inputPort, dataPipeId);
  while (!data && (getThreadStatus() == ThreadStatus::RUN)) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    data = popInputData(inputPort, dataPipeId);
  }
  if (data == nullptr) return common::ErrorCode::SUCCESS;

  auto objectMetadata = std::static_pointer_cast<common::ObjectMetadata>(data);

  if (!objectMetadata->mFrame->mEndOfStream) {
    nlohmann::json serializedObj = objectMetadata;

    int channel_id = objectMetadata->mFrame->mChannelId;
    auto implIt = mapImpl_.find(channel_id);
    if (implIt == mapImpl_.end()) {
      std::lock_guard<std::mutex> lock(mapMtx);
      auto httpImpl =
          std::make_shared<HttpPushImpl_>(ip_, port_, path_, channel_id);
      mapImpl_[channel_id] = httpImpl;
      mapImpl_[channel_id]->pushQueue(
          std::make_shared<nlohmann::json>(serializedObj));
    } else
      implIt->second->pushQueue(
          std::make_shared<nlohmann::json>(serializedObj));
  }

  int channel_id_internal = objectMetadata->mFrame->mChannelIdInternal;
  int outDataPipeId =
      getSinkElementFlag()
          ? 0
          : (channel_id_internal % getOutputConnectorCapacity(outputPort));
  common::ErrorCode errorCode =
      pushOutputData(outputPort, outDataPipeId,
                     std::static_pointer_cast<void>(objectMetadata));
  if (common::ErrorCode::SUCCESS != errorCode) {
    IVS_WARN(
        "Send data fail, element id: {0:d}, output port: {1:d}, data: "
        "{2:p}",
        getId(), outputPort, static_cast<void*>(objectMetadata.get()));
  }
  return common::ErrorCode::SUCCESS;
}

REGISTER_WORKER("http_push", HttpPush)
}  // namespace http_push
}  // namespace element
}  // namespace sophon_stream