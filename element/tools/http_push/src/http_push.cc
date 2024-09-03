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
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
    auto schemeIt = configure.find(CONFIG_INTERNAL_SCHEME_FILED);
    if (schemeIt == configure.end()) {
        scheme_ = "http";
    } else {
	scheme_ = schemeIt->get<std::string>();  // 获取值
        STREAM_CHECK((scheme_ == "http" || scheme_ == "https"),
                     "Scheme must be http or https, please check your http_push element "
                     "configuration file");
    }

    auto certIt = configure.find(CONFIG_INTERNAL_CERT_FILED);
    if (certIt == configure.end()) {
        cert_ = "";
    } else {
        STREAM_CHECK(certIt->is_string(),
                 "Cert path must be string, please check your http_push element "
                 "configuration file");
        cert_ = certIt->get<std::string>();
    }

    auto keyIt = configure.find(CONFIG_INTERNAL_KEY_FILED);
    if (keyIt == configure.end()) {
        key_ = "";
    } else {
        STREAM_CHECK(keyIt->is_string(),
                 "Key path must be string, please check your http_push element "
                 "configuration file");
        key_ = keyIt->get<std::string>();
    }

    auto cacertIt = configure.find(CONFIG_INTERNAL_CACERT_FILED);
    if (cacertIt == configure.end()) {
        cacert_ = "";
    } else {
        STREAM_CHECK(cacertIt->is_string(),
                 "CACERT path must be string, please check your http_push element "
                 "configuration file");
        cacert_ = cacertIt->get<std::string>();
    }

    auto verifyIt = configure.find(CONFIG_INTERNAL_VERIFY_FILED);
    if (verifyIt == configure.end()) {
        verify_ = false;
    } else {
        verify_ = verifyIt->get<bool>();
    }
#endif

  } while (false);
  return errorCode;
}

#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
HttpPushImpl_::HttpPushImpl_(std::string& scheme, std::string& ip, int port, std::string cert, std::string key,
		std::string cacert_, bool verify_, std::string path_, int channel)
    : cli(scheme + "://" + ip + ":" + std::to_string(port), cert, key), cacert(cacert_), verify(verify_), path(path_) {
  cli.set_ca_cert_path(cacert);
  cli.enable_server_certificate_verification(verify);
  workThread = std::thread(&HttpPushImpl_::postFunc, this);
  mFpsProfilerName = "http_push_" + std::to_string(channel) + "_fps";
  mFpsProfiler.config(mFpsProfilerName, 100);
}
#else
HttpPushImpl_::HttpPushImpl_(std::string& ip, int port, std::string path_,
                             int channel)
    : cli(ip, port), path(path_) {
  workThread = std::thread(&HttpPushImpl_::postFunc, this);
  mFpsProfilerName = "http_push_" + std::to_string(channel) + "_fps";
  mFpsProfiler.config(mFpsProfilerName, 100);
}
#endif

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

    int channel_id = objectMetadata->mFrame->mChannelIdInternal;
    auto implIt = mapImpl_.find(channel_id);
    if (implIt == mapImpl_.end()) {
      std::lock_guard<std::mutex> lock(mapMtx);
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
      auto httpImpl = std::make_shared<HttpPushImpl_>(scheme_, ip_, port_, cert_, key_,
		      cacert_, verify_, path_, channel_id);
#else
      auto httpImpl =
          std::make_shared<HttpPushImpl_>(ip_, port_, path_, channel_id);
#endif
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
