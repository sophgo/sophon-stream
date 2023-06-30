//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#include "ws.h"

#include <nlohmann/json.hpp>
#include <websocketpp/base64/base64.hpp>

#include "common/logger.h"
#include "element_factory.h"

namespace sophon_stream {
namespace element {
namespace ws {

WS::WS() {}

WS::~WS() { uninitInternal(); }

common::ErrorCode WS::initInternal(const std::string& json) {
  common::ErrorCode errorCode = common::ErrorCode::SUCCESS;
  do {
    auto configure = nlohmann::json::parse(json, nullptr, false);
    if (!configure.is_object()) {
      errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
      IVS_ERROR("json parse failed! json:{0}", json);
      break;
    }
    mFpsProfiler.config("fps_ws", 100);
    auto startPortIt = configure.find(CONFIG_INTERNAL_START_PORT_FIELD);
    if (configure.end() != startPortIt) {
      mStartPort = startPortIt->get<int>();
    } else {
      errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
      IVS_ERROR(
          "Can not find {0} with string type in worker json configure, json: "
          "{1}",
          CONFIG_INTERNAL_START_PORT_FIELD, json);
      break;
    }

  } while (false);
  return errorCode;
}

void WS::uninitInternal() {
  for (auto it = mServerMap.begin(); it != mServerMap.end(); ++it)
    it->second->stop();
  for (auto& thread : mServerThreads) thread.join();
}

void create_wss(WSS* wss, int server_port) { wss->init(server_port); }

void send_wss(WSS* wss) { wss->send(); };

common::ErrorCode WS::doWork(int dataPipeId) {
  common::ErrorCode errorCode = common::ErrorCode::SUCCESS;

  common::ObjectMetadatas objectMetadatas;
  std::vector<int> inputPorts = getInputPorts();
  int inputPort = inputPorts[0];
  int outputPort = 0;
  if (!getSinkElementFlag()) {
    std::vector<int> outputPorts = getOutputPorts();
    int outputPort = outputPorts[0];
  }

  std::shared_ptr<void> data;
  while (getThreadStatus() == ThreadStatus::RUN) {
    data = popInputData(inputPort, dataPipeId);
    if (!data) {
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
      continue;
    }
    break;
  }

  if (!data) return common::ErrorCode::SUCCESS;

  auto objectMetadata = std::static_pointer_cast<common::ObjectMetadata>(data);

  auto encodeIt = mServerMap.find(dataPipeId);
  if (!(objectMetadata->mFrame->mEndOfStream)) {
    if (mServerMap.end() == encodeIt) {
      int channel_id = objectMetadata->mFrame->mChannelId;
      int server_port = mStartPort + channel_id;
      std::shared_ptr<WSS> wss = std::make_shared<WSS>();
      std::thread t(create_wss, wss.get(), server_port);
      std::thread s(send_wss, wss.get());
      std::lock_guard<std::mutex> lk(mServerThreadsMutex);
      mServerThreads.push_back(std::move(s));
      mServerThreads.push_back(std::move(t));
      mServerMap[dataPipeId] = wss;
    } else {
      void* jpeg_data = NULL;
      size_t out_size = 0;
      std::shared_ptr<bm_image> img = objectMetadata->mFrame->mSpDataOsd
                                          ? objectMetadata->mFrame->mSpDataOsd
                                          : objectMetadata->mFrame->mSpData;
      std::shared_ptr<bm_image> img_to_enc = img;
      if (img->image_format != FORMAT_YUV420P) {
        img_to_enc.reset(new bm_image,
                         [&](bm_image* img) { bm_image_destroy(*img); });
        bm_image image = *(objectMetadata->mFrame->mSpData);
        bm_image_create(objectMetadata->mFrame->mHandle,
                        objectMetadata->mFrame->mHeight,
                        objectMetadata->mFrame->mWidth, FORMAT_YUV420P,
                        image.data_type, &(*img_to_enc));
        bmcv_image_storage_convert(objectMetadata->mFrame->mHandle, 1,
                                   img.get(), img_to_enc.get());
      }
      bmcv_image_jpeg_enc(objectMetadata->mFrame->mHandle, 1, img_to_enc.get(),
                          &jpeg_data, &out_size);
      std::string data =
          "data:image/jpeg;base64," +
          websocketpp::base64_encode((const unsigned char*)jpeg_data, out_size);
      // base64 img 存入队列
      encodeIt->second->pushImgDataQueue(data);
      mFpsProfiler.add(1);
    }
  } else {
    // 队列数据停止flag
    encodeIt->second->pushImgDataQueue(WS_STOP_FLAG);
  }

  int channel_id_internal = objectMetadata->mFrame->mChannelIdInternal;
  int outDataPipeId =
      getSinkElementFlag()
          ? 0
          : (channel_id_internal % getOutputConnectorCapacity(outputPort));
  errorCode = pushOutputData(outputPort, outDataPipeId, objectMetadata);
  if (common::ErrorCode::SUCCESS != errorCode) {
    IVS_WARN(
        "Send data fail, element id: {0:d}, output port: {1:d}, data: "
        "{2:p}",
        getId(), outputPort, static_cast<void*>(objectMetadata.get()));
  }

  return common::ErrorCode::SUCCESS;
}

REGISTER_WORKER("ws", WS)

}  // namespace ws
}  // namespace element
}  // namespace sophon_stream
