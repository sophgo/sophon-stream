//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#include "converger.h"

#include <nlohmann/json.hpp>

#include "common/logger.h"
#include "element_factory.h"

namespace sophon_stream {
namespace element {
namespace converger {

Converger::Converger() {}
Converger::~Converger() {}

common::ErrorCode Converger::initInternal(const std::string& json) {
  common::ErrorCode errorCode = common::ErrorCode::SUCCESS;
  do {
    auto configure = nlohmann::json::parse(json, nullptr, false);
    if (!configure.is_object()) {
      errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
      break;
    }
    int _default_port =
        configure.find(CONFIG_INTERNAL_DEFAULT_PORT_FILED)->get<int>();
    mDefaultPort = _default_port;
  } while (false);
  return errorCode;
}

common::ErrorCode Converger::doWork(int dataPipeId) {
  common::ErrorCode errorCode = common::ErrorCode::SUCCESS;
  std::vector<int> inputPorts = getInputPorts();
  std::vector<int> outputPorts = getOutputPorts();
  int outputPort = getSinkElementFlag() ? 0 : outputPorts[0];

  // 从所有inputPort中取出数据，并且做判断
  // default_port中取出的数据，放到map里
  auto data = popInputData(mDefaultPort, dataPipeId);
  int retry_times = 0;
  while (!data && (getThreadStatus() == ThreadStatus::RUN) && retry_times < 5) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    data = popInputData(mDefaultPort, dataPipeId);
    ++retry_times;
  }
  // if (data == nullptr) return common::ErrorCode::SUCCESS;
  if (data != nullptr) {
    auto objectMetadata =
        std::static_pointer_cast<common::ObjectMetadata>(data);
    int channel_id = objectMetadata->mFrame->mChannelIdInternal;
    int frame_id = objectMetadata->mFrame->mFrameId;
    mCandidates[channel_id][frame_id] = objectMetadata;
    // mBranches[channel_id][frame_id] = objectMetadata->numBranches;
    IVS_DEBUG(
        "data recognized, channel_id = {0}, frame_id = {1}, num_branches = {2}",
        channel_id, frame_id, objectMetadata->numBranches);
  }

  // 非default_port，取出来之后更新分支数的记录
  for (int inputPort : inputPorts) {
    if (inputPort == mDefaultPort) continue;
    auto subdata = popInputData(inputPort, dataPipeId);
    // 这里不能在while里取，否则会堵住
    while (subdata != nullptr) {
      auto subObj = std::static_pointer_cast<common::ObjectMetadata>(subdata);
      int sub_channel_id = subObj->mFrame->mChannelIdInternal;
      int sub_frame_id = subObj->mFrame->mFrameId;
      IVS_DEBUG("subData recognized, channel_id = {0}, frame_id = {1}",
                sub_channel_id, sub_frame_id);
      mBranches[sub_channel_id][sub_frame_id]++;
      IVS_DEBUG(
          "data updated, channel_id = {0}, frame_id = {1}, current "
          "num_branches "
          "= {2}",
          sub_channel_id, sub_frame_id,
          mBranches[sub_channel_id][sub_frame_id]);
      subdata = popInputData(inputPort, dataPipeId);
    }
    // if (subdata == nullptr) continue;
  }

  // 遍历map，能弹出去的都弹出去
  for (auto channel_it = mCandidates.begin(); channel_it != mCandidates.end();
       ++channel_it) {
    // 第一层：遍历所有channel
    int channel_id_internal = channel_it->first;
    for (auto frame_it = mCandidates[channel_id_internal].begin();
         frame_it != mCandidates[channel_id_internal].end();) {
      // 第二层：遍历当前channel下的所有frame，有序
      int frame_id = frame_it->first;
      // 如果可以弹出，则弹出并循环至下一个
      if (mBranches[channel_id_internal][frame_id] ==
          mCandidates[channel_id_internal][frame_id]->numBranches) {
        IVS_DEBUG("Data converged! Now pop... channel_id = {0}, frame_id = {1}",
                  channel_id_internal, frame_id);
        auto obj = mCandidates[channel_id_internal][frame_id];
        int outDataPipeId = getSinkElementFlag()
                                ? 0
                                : (channel_id_internal %
                                   getOutputConnectorCapacity(outputPort));
        errorCode = pushOutputData(outputPort, outDataPipeId,
                                   std::static_pointer_cast<void>(obj));
        if (common::ErrorCode::SUCCESS != errorCode) {
          IVS_WARN(
              "Send data fail, element id: {0:d}, output port: {1:d}, data: "
              "{2:p}",
              getId(), outputPort, static_cast<void*>(obj.get()));
        }
        mCandidates[channel_id_internal].erase(frame_it++);
        // delete mBranches[channel_id_internal][frame_id]
        auto branchChannelIt = mBranches.find(channel_id_internal);
        auto channelFrameIt = branchChannelIt->second.find(frame_id);
        branchChannelIt->second.erase(channelFrameIt);
      } else {
        // 当前帧不可以弹出，为了保证时序性，后续帧也不弹出
        break;
      }
    }
  }
  return errorCode;
}
REGISTER_WORKER("converger", Converger)

}  // namespace converger
}  // namespace element
}  // namespace sophon_stream