//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#include "encode.h"

#include <nlohmann/json.hpp>

#include "common/logger.h"
#include "element_factory.h"

namespace sophon_stream {
namespace element {
namespace encode {

constexpr const char* CONFIG_INTERNAL_RTSP_PORT_FIELD = "rtsp_port";

Encode::Encode() {}

Encode::~Encode() {}

common::ErrorCode Encode::initInternal(const std::string& json) {
  common::ErrorCode errorCode = common::ErrorCode::SUCCESS;
  do {
    auto configure = nlohmann::json::parse(json, nullptr, false);
    if (!configure.is_object()) {
      errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
      break;
    }

    auto rtspPortIt = configure.find(CONFIG_INTERNAL_RTSP_PORT_FIELD);
    mRtspPort = rtspPortIt->get<int>();
    std::string enc_fmt = "h264_bm";
    std::string pix_fmt = "I420";
    std::string enc_params =
        "gop=32:gop_preset=3:framerate=25:bitrate=2000";

    int dev_id = getDeviceId();
    bm_dev_request(&m_handle, dev_id);

    int threadNumber = getThreadNumber();
    for (int i = 0; i < threadNumber; ++i) {
      mEncoderMap[i] =
          std::make_shared<Encoder>(m_handle, enc_fmt, pix_fmt, enc_params);
    }

  } while (false);
  return errorCode;
}

void Encode::uninitInternal() {
  for (auto it = mEncoderMap.begin(); it != mEncoderMap.end(); ++it)
    it->second->release();
}

common::ErrorCode Encode::doWork(int dataPipeId) {
  common::ErrorCode errorCode = common::ErrorCode::SUCCESS;

  common::ObjectMetadatas objectMetadatas;
  std::vector<int> inputPorts = getInputPorts();
  int inputPort = inputPorts[0];
  int outputPort = 0;
  if (!getLastElementFlag()) {
    std::vector<int> outputPorts = getOutputPorts();
    int outputPort = outputPorts[0];
  }

  std::shared_ptr<void> data;
  while (getThreadStatus() == ThreadStatus::RUN) {
    data = getInputData(inputPort, dataPipeId);
    if (!data) {
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
      continue;
    }
    break;
  }

  if (!data) return common::ErrorCode::SUCCESS;

  auto objectMetadata = std::static_pointer_cast<common::ObjectMetadata>(data);

  if (!(objectMetadata->mFrame->mEndOfStream)) {
    auto encodeIt = mEncoderMap.find(dataPipeId);
    if (mEncoderMap.end() != encodeIt) {
      int channel_id = objectMetadata->mFrame->mChannelId;
      if (mChannelOutputPath.find(channel_id) == mChannelOutputPath.end()) {
        std::string output_path =
            "rtsp://localhost:" + std::to_string(mRtspPort) + "/" +
            std::to_string(channel_id);
        mChannelOutputPath[channel_id] = output_path;
        encodeIt->second->set_output_path(output_path);
        encodeIt->second->set_enc_params_width(objectMetadata->mFrame->mWidth);
        encodeIt->second->set_enc_params_height(objectMetadata->mFrame->mHeight);
        encodeIt->second->init_writer();
      }
      if (objectMetadata->mFrame->mSpDataOsd) {
        encodeIt->second->video_write(*(objectMetadata->mFrame->mSpDataOsd));
      } else {
        encodeIt->second->video_write(*(objectMetadata->mFrame->mSpData));
      }
    }
  }

  int channel_id_internal = objectMetadata->mFrame->mChannelIdInternal;
  int outDataPipeId =
      getLastElementFlag()
          ? 0
          : (channel_id_internal % getOutputConnector(outputPort)->getDataPipeCount());
  errorCode = pushOutputData(outputPort, outDataPipeId, objectMetadata);
  if (common::ErrorCode::SUCCESS != errorCode) {
    IVS_WARN(
        "Send data fail, element id: {0:d}, output port: {1:d}, data: "
        "{2:p}",
        getId(), outputPort, static_cast<void*>(objectMetadata.get()));
  }

  return common::ErrorCode::SUCCESS;
}

REGISTER_WORKER("encode", Encode)

}  // namespace encode
}  // namespace element
}  // namespace sophon_stream
