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

Encode::Encode() {}

Encode::~Encode() {}

common::ErrorCode Encode::initInternal(const std::string& json) {
  common::ErrorCode errorCode = common::ErrorCode::SUCCESS;
  do {
    auto configure = nlohmann::json::parse(json, nullptr, false);
    if (!configure.is_object()) {
      errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
      IVS_ERROR("json parse failed! json:{0}", json);
      break;
    }

    auto encodeTypeIt = configure.find(CONFIG_INTERNAL_ENCODE_TYPE_FIELD);
    if (configure.end() != encodeTypeIt) {
      std::string encodeType = encodeTypeIt->get<std::string>();
      mEncodeType = EncodeType::UNKNOWN;
      if (encodeType == "RTSP") mEncodeType = EncodeType::RTSP;
      if (encodeType == "RTMP") mEncodeType = EncodeType::RTMP;
      if (encodeType == "VIDEO") mEncodeType = EncodeType::VIDEO;
      if (encodeType == "IMG_DIR") mEncodeType = EncodeType::IMG_DIR;
      if (encodeType == "WS") mEncodeType = EncodeType::WS;
      IVS_DEBUG("EncodeType is {0}", encodeType);
    } else {
      errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
      IVS_ERROR(
          "Can not find {0} with string type in worker json configure, json: "
          "{1}",
          CONFIG_INTERNAL_ENCODE_TYPE_FIELD, json);
      break;
    }
    if (mEncodeType == EncodeType::RTSP) {
      auto rtspPortIt = configure.find(CONFIG_INTERNAL_RTSP_PORT_FIELD);
      if (configure.end() != rtspPortIt) {
        mRtspPort = rtspPortIt->get<std::string>();
      } else {
        errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
        IVS_ERROR(
            "Can not find {0} with string type in worker json configure, "
            "json: "
            "{1}",
            CONFIG_INTERNAL_RTSP_PORT_FIELD, json);
        break;
      }
    }
    if (mEncodeType == EncodeType::RTMP) {
      auto rtmpPortIt = configure.find(CONFIG_INTERNAL_RTMP_PORT_FIELD);
      if (configure.end() != rtmpPortIt) {
        mRtmpPort = rtmpPortIt->get<std::string>();
      } else {
        errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
        IVS_ERROR(
            "Can not find {0} with string type in worker json configure, "
            "json: "
            "{1}",
            CONFIG_INTERNAL_RTMP_PORT_FIELD, json);
        break;
      }
    }
    if (mEncodeType == EncodeType::RTSP || mEncodeType == EncodeType::RTMP ||
        mEncodeType == EncodeType::VIDEO) {
      auto encFmtIt = configure.find(CONFIG_INTERNAL_ENC_FMT_FIELD);
      if (configure.end() != encFmtIt) {
        encFmt = encFmtIt->get<std::string>();
        if (encFmt != "h264_bm" && encFmt != "h265_bm") {
          IVS_ERROR("Encode format error, please input h264_bm or h265_bm");
        }
      } else {
        errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
        IVS_ERROR(
            "Can not find {0} with string type in worker json configure, "
            "json: "
            "{1}",
            CONFIG_INTERNAL_ENC_FMT_FIELD, json);
        break;
      }
      auto pixFmtIt = configure.find(CONFIG_INTERNAL_PIX_FMT_FIELD);
      if (configure.end() != pixFmtIt) {
        pixFmt = pixFmtIt->get<std::string>();
        if (pixFmt != "I420" && pixFmt != "NV12") {
          IVS_ERROR("Encode format error, please input I420 or NV12");
        }
      } else {
        errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
        IVS_ERROR(
            "Can not find {0} with string type in worker json configure, "
            "json: "
            "{1}",
            CONFIG_INTERNAL_PIX_FMT_FIELD, json);
        break;
      }
      std::string enParams = "gop=32:gop_preset=3:framerate=30:bitrate=2000";

      int dev_id = getDeviceId();
      bm_dev_request(&m_handle, dev_id);

      int threadNumber = getThreadNumber();
      for (int i = 0; i < threadNumber; ++i) {
        mEncoderMap[i] =
            std::make_shared<Encoder>(m_handle, encFmt, pixFmt, enParams);
      }
    } else if (mEncodeType == EncodeType::IMG_DIR) {
      const char* dir_path = "./results";
      struct stat info;
      if (stat(dir_path, &info) == 0 && S_ISDIR(info.st_mode)) {
        IVS_INFO("Directory already exists.");
      } else {
        if (mkdir(dir_path, 0777) == 0) {
          IVS_INFO("Directory created successfully.");
        } else {
          IVS_INFO("Error creating directory.");
        }
      }
    } else if (mEncodeType == EncodeType::WS) {
      auto wssPortIt = configure.find(CONFIG_INTERNAL_WSS_PORT_FIELD);
      if (configure.end() != wssPortIt) {
        mWSSPort = wssPortIt->get<std::string>();
      } else {
        errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
        IVS_ERROR(
            "Can not find {0} with string type in worker json configure, "
            "json: "
            "{1}",
            CONFIG_INTERNAL_WSS_PORT_FIELD, json);
        break;
      }
    }

    mFpsProfiler.config("fps_encode", 100);
  } while (false);
  return errorCode;
}

void Encode::uninitInternal() {
  if (mEncodeType != EncodeType::WS) {
    for (auto it = mEncoderMap.begin(); it != mEncoderMap.end(); ++it)
      it->second->release();
  } else {
    for (auto it = mWSSMap.begin(); it != mWSSMap.end(); ++it)
      it->second->stop();
    for (auto& thread : mWSSThreads) thread.join();
  }
}

void create_wss(WSS* wss, int server_port) { wss->init(server_port); }

void send_wss(WSS* wss) { wss->send(); };

common::ErrorCode Encode::doWork(int dataPipeId) {
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

  if (objectMetadata->mFrame->mEndOfStream) {
    auto encodeIt = mEncoderMap.find(dataPipeId);
    encodeIt->second->isRunning = false;
  }

  if (!(objectMetadata->mFrame->mEndOfStream) &&
      std::find(objectMetadata->mSkipElements.begin(),
                objectMetadata->mSkipElements.end(),
                getId()) == objectMetadata->mSkipElements.end()) {
    if (mEncodeType == EncodeType::RTSP || mEncodeType == EncodeType::RTMP ||
        mEncodeType == EncodeType::VIDEO) {
      processVideoStream(dataPipeId, objectMetadata);
    } else if (mEncodeType == EncodeType::IMG_DIR) {
      processImgDir(dataPipeId, objectMetadata);
    } else if (mEncodeType == EncodeType::WS) {
      processWS(dataPipeId, objectMetadata);
    } else {
    }
    mFpsProfiler.add(1);
  } else {
    // WS发送停止标识
    if (mEncodeType == EncodeType::WS) {
      stopWS(dataPipeId);
    }
  }

  // mFpsProfiler.add(1);
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

// 处理RTSP、RTMP、VIDEO
void Encode::processVideoStream(
    int dataPipeId, std::shared_ptr<common::ObjectMetadata> objectMetadata) {
  auto encodeIt = mEncoderMap.find(dataPipeId);
  if (mEncoderMap.end() != encodeIt) {
    int channel_id = objectMetadata->mFrame->mChannelId;
    if (mChannelOutputPath.find(channel_id) == mChannelOutputPath.end()) {
      std::string output_path;
      switch (mEncodeType) {
        case EncodeType::RTSP:
          output_path = "rtsp://localhost:" + mRtspPort + "/" +
                        std::to_string(channel_id);
          break;
        case EncodeType::RTMP:
          output_path = "rtmp://localhost:" + mRtmpPort + "/" +
                        std::to_string(channel_id);
          break;
        case EncodeType::VIDEO:
          output_path = std::to_string(channel_id) + ".avi";
          break;
        default:
          IVS_ERROR("Encode type error, please input RTSP, RTMP or VIDEO");
      }
      if (mChannelOutputPath.find(channel_id) == mChannelOutputPath.end()) {
        mChannelOutputPath[channel_id] = output_path;
        encodeIt->second->set_output_path(output_path);
        encodeIt->second->set_enc_params_width(objectMetadata->mFrame->mWidth);
        encodeIt->second->set_enc_params_height(
            objectMetadata->mFrame->mHeight);
        encodeIt->second->init_writer();
      }
    }
    if (objectMetadata->mFrame->mSpDataOsd) {
      encodeIt->second->isRunning = true;
      encodeIt->second->pushQueue(objectMetadata->mFrame->mSpDataOsd);
      // encodeIt->second->video_write(*(objectMetadata->mFrame->mSpDataOsd));
    } else {
      encodeIt->second->isRunning = true;
      encodeIt->second->pushQueue(objectMetadata->mFrame->mSpData);
      // encodeIt->second->video_write(*(objectMetadata->mFrame->mSpData));
    }
  }
}

// 处理IMG_DIR
void Encode::processImgDir(
    int dataPipeId, std::shared_ptr<common::ObjectMetadata> objectMetadata) {
  const char* dir_path =
      ("./results/" + std::to_string(objectMetadata->mFrame->mChannelId))
          .c_str();
  struct stat info;
  if (stat(dir_path, &info) == 0 && S_ISDIR(info.st_mode)) {
    IVS_INFO("Directory already exists.");
  } else {
    if (mkdir(dir_path, 0777) == 0) {
      IVS_INFO("Directory created successfully.");
    } else {
      IVS_INFO("Error creating directory.");
    }
  }
  int width = objectMetadata->mFrame->mWidth;
  int height = objectMetadata->mFrame->mHeight;
  bm_image image = objectMetadata->mFrame->mSpDataOsd
                       ? *objectMetadata->mFrame->mSpDataOsd
                       : *objectMetadata->mFrame->mSpData;
  bm_image imageStorage;
  bm_image_create(objectMetadata->mFrame->mHandle, height, width,
                  FORMAT_YUV420P, image.data_type, &imageStorage);
  bmcv_image_storage_convert(objectMetadata->mFrame->mHandle, 1, &image,
                             &imageStorage);
  // save image
  void* jpeg_data = NULL;
  size_t out_size = 0;
  int ret = bmcv_image_jpeg_enc(objectMetadata->mFrame->mHandle, 1,
                                &imageStorage, &jpeg_data, &out_size);
  if (ret == BM_SUCCESS) {
    std::string img_file =
        "./results/" + std::to_string(objectMetadata->mFrame->mChannelId) +
        "/" + std::to_string(objectMetadata->mFrame->mFrameId) + ".jpg";
    FILE* fp = fopen(img_file.c_str(), "wb");
    fwrite(jpeg_data, out_size, 1, fp);
    fclose(fp);
  }
  free(jpeg_data);
  bm_image_destroy(imageStorage);
}

// 处理WS
void Encode::processWS(int dataPipeId,
                       std::shared_ptr<common::ObjectMetadata> objectMetadata) {
  auto serverIt = mWSSMap.find(dataPipeId);
  if (mWSSMap.end() == serverIt) {
    int channel_id = objectMetadata->mFrame->mChannelId;
    int server_port = std::stoi(mWSSPort) + channel_id;
    std::shared_ptr<WSS> wss = std::make_shared<WSS>();
    std::thread t(create_wss, wss.get(), server_port);
    std::thread s(send_wss, wss.get());
    std::lock_guard<std::mutex> lk(mWSSThreadsMutex);
    mWSSThreads.push_back(std::move(s));
    mWSSThreads.push_back(std::move(t));
    mWSSMap[dataPipeId] = wss;
    serverIt = mWSSMap.find(dataPipeId);
  }
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
    bmcv_image_storage_convert(objectMetadata->mFrame->mHandle, 1, img.get(),
                               img_to_enc.get());
  }
  bmcv_image_jpeg_enc(objectMetadata->mFrame->mHandle, 1, img_to_enc.get(),
                      &jpeg_data, &out_size);
  std::string data =
      "data:image/jpeg;base64," +
      websocketpp::base64_encode((const unsigned char*)jpeg_data, out_size);
  // base64 img 存入队列
  serverIt->second->pushImgDataQueue(data);
}

// WS发送停止标识
void Encode::stopWS(int dataPipeId) {
  // 从map中获取wss对象
  auto serverIt = mWSSMap.find(dataPipeId);
  // 队列数据停止flag
  serverIt->second->pushImgDataQueue(WS_STOP_FLAG);
}

REGISTER_WORKER("encode", Encode)

}  // namespace encode
}  // namespace element
}  // namespace sophon_stream
