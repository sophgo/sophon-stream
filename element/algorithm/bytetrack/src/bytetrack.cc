//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#include "bytetrack.h"

#include <nlohmann/json.hpp>

#include "common/logger.h"
#include "element_factory.h"

namespace sophon_stream {
namespace element {
namespace bytetrack {

constexpr const char* CONFIG_INTERNAL_FRAME_RATE_FIELD = "frame_rate";
constexpr const char* CONFIG_INTERNAL_TRACK_BUFFER_FIELD = "track_buffer";
constexpr const char* CONFIG_INTERNAL_TRACK_THRESH_FIELD = "track_thresh";
constexpr const char* CONFIG_INTERNAL_HIGH_THRESH_FIELD = "high_thresh";
constexpr const char* CONFIG_INTERNAL_MATCH_THRESH_FIELD = "match_thresh";

Bytetrack::Bytetrack() { IVS_DEBUG("Bytetrack Element construct!!!"); }

Bytetrack::~Bytetrack() {}

common::ErrorCode Bytetrack::initContext(const std::string& json) {
  common::ErrorCode errorCode = common::ErrorCode::SUCCESS;
  do {
    IVS_DEBUG("Bytetrack::initContext");
    mContext = std::make_shared<BytetrackContext>();

    auto configure = nlohmann::json::parse(json, nullptr, false);
    if (!configure.is_object()) {
      IVS_WARN("Bytetrack::initContext: PARSE_CONFIGURE_FAIL");
      errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
      break;
    }

    auto frameRateIt = configure.find(CONFIG_INTERNAL_FRAME_RATE_FIELD);
    mContext->frameRate =
        frameRateIt != configure.end() ? frameRateIt->get<int>() : 30;

    auto trackBufferIt = configure.find(CONFIG_INTERNAL_TRACK_BUFFER_FIELD);
    mContext->trackBuffer =
        trackBufferIt != configure.end() ? trackBufferIt->get<int>() : 30;

    auto trackThreshIt = configure.find(CONFIG_INTERNAL_TRACK_THRESH_FIELD);
    mContext->trackThresh =
        trackThreshIt != configure.end() ? trackThreshIt->get<float>() : 0.6;

    auto highThreshIt = configure.find(CONFIG_INTERNAL_HIGH_THRESH_FIELD);
    mContext->highThresh =
        highThreshIt != configure.end() ? highThreshIt->get<float>() : 0.7;

    auto matchThreshIt = configure.find(CONFIG_INTERNAL_MATCH_THRESH_FIELD);
    mContext->matchThresh =
        matchThreshIt != configure.end() ? matchThreshIt->get<float>() : 0.8;

    IVS_DEBUG(
        "Bytetrack::initContext: frameRate: {0}, trackBuffer: {1}, "
        "trackThresh: {2}, "
        "highThresh: {3}, matchThresh: {4}",
        mContext->frameRate, mContext->trackBuffer, mContext->trackThresh,
        mContext->highThresh, mContext->matchThresh);

  } while (false);

  return common::ErrorCode::SUCCESS;
}

common::ErrorCode Bytetrack::initInternal(const std::string& json) {
  common::ErrorCode errorCode = common::ErrorCode::SUCCESS;

  do {
    IVS_DEBUG("Bytetrack::initInternal");
    // json是否正确
    auto configure = nlohmann::json::parse(json, nullptr, false);
    if (!configure.is_object()) {
      IVS_WARN("Bytetrack::initInternal: PARSE_CONFIGURE_FAIL");
      errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
      break;
    }

    initContext(configure.dump());

    BYTETracker bytetrackr(mContext->frameRate, mContext->trackBuffer,
                           mContext->trackThresh, mContext->highThresh,
                           mContext->matchThresh);

  } while (false);

  return errorCode;
}

/**
 * update track
 * @param[in/out] objectMetadatas:  输入数据和预测结果
 */
void Bytetrack::process(
    std::shared_ptr<common::ObjectMetadata>& objectMetadata) {
  bytetrackr.update(objectMetadata);
}

/**
 * 资源释放函数
 */
void Bytetrack::uninitInternal() {}

/**
  运行
*/
common::ErrorCode Bytetrack::doWork() {
  common::ErrorCode errorCode = common::ErrorCode::SUCCESS;
  std::vector<int> inputPorts = getInputPorts();
  std::vector<int> outputPorts = getInputPorts();
  int inputPort = inputPorts[0];
  int outputPort = outputPorts[0];

  auto data = getInputData(inputPort);
  if (!data) {
    return errorCode;
  }
  popInputData(inputPort);

  auto objectMetadata = std::static_pointer_cast<common::ObjectMetadata>(data);

  process(objectMetadata);

  errorCode =
      pushOutputData(outputPort, std::static_pointer_cast<void>(objectMetadata),
                     std::chrono::milliseconds(200));

  if (common::ErrorCode::SUCCESS != errorCode) {
    IVS_WARN(
        "Send data fail, element id: {0:d}, output port: {1:d}, data: "
        "{2:p}",
        getId(), outputPort, static_cast<void*>(objectMetadata.get()));
  }

  return common::ErrorCode::SUCCESS;
}

REGISTER_WORKER("bytetrack", Bytetrack)

}  // namespace bytetrack
}  // namespace element
}  // namespace sophon_stream
