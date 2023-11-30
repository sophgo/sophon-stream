//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#include "blank.h"

#include "common/logger.h"
#include "element_factory.h"

namespace sophon_stream {
namespace element {
namespace blank {
Blank::Blank() {}
Blank::~Blank() {}

common::ErrorCode Blank::initInternal(const std::string& json) {
  return common::ErrorCode::SUCCESS;
}

void Blank::registListenFunc(sophon_stream::framework::ListenThread* listener) {
  std::string mIdStr = std::to_string(getId());
  std::string handlerName = postNameSetIdx + "/" + mIdStr;
  listener->setHandler(handlerName.c_str(),
                       std::bind(&Blank::listenerSetIdx, this,
                                 std::placeholders::_1, std::placeholders::_2));
}

void Blank::listenerSetIdx(const httplib::Request& request,
                           httplib::Response& response) {
  auto listener = getListener();
  common::Response resp;
  common::RequestSingleInt rsi;
  common::str_to_object(request.body, rsi);
  printIdx = rsi.idx;
  resp.code = 0;
  resp.msg = "success";
  nlohmann::json json_res = resp;
  response.set_content(json_res.dump(), "application/json");
  return;
}

common::ErrorCode Blank::doWork(int dataPipeId) {
  std::vector<int> inputPorts = getInputPorts();
  int inputPort = inputPorts[0];
  int outputPort = 0;
  if (!getSinkElementFlag()) {
    std::vector<int> outputPorts = getOutputPorts();
    int outputPort = outputPorts[0];
  }

  auto data = popInputData(inputPort, dataPipeId);
  while (!data && (getThreadStatus() == ThreadStatus::RUN)) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    data = popInputData(inputPort, dataPipeId);
  }
  if (data == nullptr) return common::ErrorCode::SUCCESS;

  auto objectMetadata = std::static_pointer_cast<common::ObjectMetadata>(data);

  IVS_INFO("Recv data, mChannelId is {0}, mFrameId is {1}, printIdx is {2}",
           objectMetadata->mFrame->mChannelId, objectMetadata->mFrame->mFrameId,
           printIdx);

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

REGISTER_WORKER("blank", Blank)
}  // namespace blank
}  // namespace element
}  // namespace sophon_stream