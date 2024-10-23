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

#include "common/serialize.h"
namespace sophon_stream {
namespace element {
namespace encode {

Encode::Encode() {}

Encode::~Encode() {
  if (mEncodeType == EncodeType::RTSP || mEncodeType == EncodeType::RTMP ||
      mEncodeType == EncodeType::VIDEO) {
    for (auto it = mEncoderMap.begin(); it != mEncoderMap.end(); ++it) {
      it->second->release();
    }
  } else if (mEncodeType == EncodeType::WS) {
    for (auto& thread : mWSSThreads) thread.join();
  } else {
  }
}

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

    mFps = 25;
    auto fpsIt = configure.find(CONFIG_INTERNAL_FPS_FIELD);
    if (configure.end() != fpsIt) {
      mFps = fpsIt->get<double>();
      IVS_DEBUG("mFps is {0}", mFps);
    } else {
      IVS_ERROR(
          "Can not find {0} in encode json configure, "
          "json:{1}, set default 25 fps",
          CONFIG_INTERNAL_FPS_FIELD, json);
    }

    auto widthIt = configure.find(CONFIG_INTERNAL_WIDTH_FIELD);
    auto heightIt = configure.find(CONFIG_INTERNAL_HEIGHT_FIELD);
    auto wsTypeIt = configure.find(CONFIG_INTERNAL_WSENCTYPE_FIELD);
    auto wsBackendIt = configure.find(CONFIG_INTERNAL_WSS_BACKEND);
    if (widthIt != configure.end() && heightIt != configure.end()) {
      width = widthIt->get<int>();
      height = heightIt->get<int>();
    }

    if (wsTypeIt != configure.end()) {
      std::string wsEncType = wsTypeIt->get<std::string>();
      if (wsEncType == "IMG_ONLY")
        mWsEncType = WSencType::IMG_ONLY;
      else if (wsEncType == "SERIALIZED")
        mWsEncType = WSencType::SERIALIZED;
    }

    if (wsBackendIt != configure.end()) {
      std::string wsBackend = wsBackendIt->get<std::string>();
      if (wsBackend == "WEBSOCKETPP")
        mWssBackend = WSSBackend::WEBSOCKETPP;
      else if (wsBackend == "BOOST")
        mWssBackend = WSSBackend::BOOST;
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

      auto ipIt = configure.find(CONFIG_INTERNAL_IP_FIELD);
      if (ipIt != configure.end()) {
        ip = ipIt->get<std::string>();
      }

      std::map<std::string, int> mEncodeParams;
      mEncodeParams["framerate"] = mFps;

      int dev_id = getDeviceId();
      // bm_dev_request(&m_handle, dev_id);

      int threadNumber = getThreadNumber();
      for (int i = 0; i < threadNumber; ++i) {
        mEncoderMap[i] =
            std::make_shared<Encoder>(dev_id, encFmt, pixFmt, mEncodeParams, i);
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

  } while (false);
  return errorCode;
}

void create_wss(WSS* wss, int server_port, double fps) {
  wss->init(server_port, fps);
}

void send_wss(WSS* wss) { wss->send(); };

common::ErrorCode Encode::doWork(int dataPipeId) {
  common::ErrorCode errorCode = common::ErrorCode::SUCCESS;
  common::ObjectMetadatas objectMetadatas;
  std::vector<int> inputPorts = getInputPorts();
  int inputPort = inputPorts[0];
  int outputPort = 0;
  if (!getSinkElementFlag()) {
    std::vector<int> outputPorts = getOutputPorts();
    outputPort = outputPorts[0];
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
  int curChannelIdInternal = objectMetadata->mFrame->mChannelIdInternal;
  if (!mFpsProfilers.count(curChannelIdInternal)) {
    mFpsProfilers[curChannelIdInternal] =
        std::make_shared<common::FpsProfiler>();
    mFpsProfilers[curChannelIdInternal]->config(
        "fps_encode" + std::to_string(curChannelIdInternal), 100);
  }
  mFpsProfilers[curChannelIdInternal]->add(1);
  if (objectMetadata->mFrame->mEndOfStream) {
    if (mEncodeType == EncodeType::RTSP || mEncodeType == EncodeType::RTMP ||
        mEncodeType == EncodeType::VIDEO) {
      IVS_DEBUG("Encode receive end of stream, dataPipeId: {0}", dataPipeId);
    }
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
  } else {
    // WS发送停止标识
    if (mEncodeType == EncodeType::WS &&
        mWssBackend == WSSBackend::WEBSOCKETPP) {
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
          output_path = "rtsp://" + ip + ":" + mRtspPort + "/" +
                        std::to_string(objectMetadata->mGraphId) + "_" +
                        std::to_string(channel_id);
          break;
        case EncodeType::RTMP:
          output_path = "rtmp://" + ip + ":" + mRtmpPort + "/" +
                        std::to_string(objectMetadata->mGraphId) + "_" +
                        std::to_string(channel_id);
          break;
        case EncodeType::VIDEO: {
          std::string dir_path_ = "./results/";
          struct stat info;
          if (stat(dir_path_.c_str(), &info) == 0 && S_ISDIR(info.st_mode)) {
            IVS_INFO("Directory already exists.");
          } else {
            if (mkdir(dir_path_.c_str(), 0777) == 0) {
              IVS_INFO("Directory created successfully.");
            } else {
              IVS_INFO("Error creating directory.");
            }
          }
          output_path = dir_path_ + std::to_string(objectMetadata->mGraphId) +
                        "_" + std::to_string(channel_id) +
                        (encFmt == "h265_bm" ? ".mp4" : ".avi");
        } break;
        default:
          IVS_ERROR("Encode type error, please input RTSP, RTMP or VIDEO");
      }

      mChannelOutputPath[channel_id] = output_path;
      encodeIt->second->set_output_path(output_path);
      encodeIt->second->set_enc_params_width(
          width == -1 ? objectMetadata->mFrame->mWidth : width);
      encodeIt->second->set_enc_params_height(
          height == -1 ? objectMetadata->mFrame->mHeight : height);
      encodeIt->second->init_writer();
    }
    if (objectMetadata->mFrame->mSpDataOsd) {
      encodeIt->second->video_write(*(objectMetadata->mFrame->mSpDataOsd));
    } else {
      encodeIt->second->video_write(*(objectMetadata->mFrame->mSpData));
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
  int width = this->width == -1 ? objectMetadata->mFrame->mWidth : this->width;
  int height =
      this->height == -1 ? objectMetadata->mFrame->mHeight : this->height;
  bm_image image = objectMetadata->mFrame->mSpDataOsd
                       ? *objectMetadata->mFrame->mSpDataOsd
                       : *objectMetadata->mFrame->mSpData;
  bm_image imageStorage;
  bm_image_create(objectMetadata->mFrame->mHandle, height, width,
                  FORMAT_YUV420P, image.data_type, &imageStorage);
  bmcv_rect_t crop_rect = {0, 0, image.width, image.height};
  bmcv_image_vpp_convert(objectMetadata->mFrame->mHandle, 1, image,
                         &imageStorage, &crop_rect);
  // save image
  void* jpeg_data = NULL;
  size_t out_size = 0;
  int ret = bmcv_image_jpeg_enc(objectMetadata->mFrame->mHandle, 1,
                                &imageStorage, &jpeg_data, &out_size);
  if (ret == BM_SUCCESS) {
    std::string img_file =
        "./results/" + std::to_string(objectMetadata->mGraphId) + "_" +
        std::to_string(objectMetadata->mFrame->mChannelId) + "/" +
        std::to_string(objectMetadata->mFrame->mFrameId) + ".jpg";
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

    if (mWssBackend == WSSBackend::WEBSOCKETPP) {
      std::shared_ptr<WSS> wss = std::make_shared<WSS>();
      std::thread t(create_wss, wss.get(), server_port, mFps);
      std::thread s(send_wss, wss.get());
      std::lock_guard<std::mutex> lk(mWSSThreadsMutex);
      mWSSThreads.push_back(std::move(s));
      mWSSThreads.push_back(std::move(t));
      mWSSMap[dataPipeId] = std::make_shared<WSSManager>(wss);
      serverIt = mWSSMap.find(dataPipeId);
    } else if (mWssBackend == WSSBackend::BOOST) {
      auto wss = std::make_shared<WebSocketServer>(server_port, mFps);
      std::thread t([wss]() { wss->run(); });
      std::lock_guard<std::mutex> lk(mWSSThreadsMutex);
      mWSSThreads.push_back(std::move(t));
      mWSSMap[dataPipeId] = std::make_shared<WSSManager>(wss);
      serverIt = mWSSMap.find(dataPipeId);
    }
  }

  if (!serverIt->second->getConnectionsNum()) {
    return;
  }

  std::string data;
  if (mWsEncType == WSencType::IMG_ONLY) {
    void* jpeg_data = NULL;
    size_t out_size = 0;
    std::shared_ptr<bm_image> img = objectMetadata->mFrame->mSpDataOsd
                                        ? objectMetadata->mFrame->mSpDataOsd
                                        : objectMetadata->mFrame->mSpData;
    std::shared_ptr<bm_image> img_to_enc;
    img_to_enc.reset(new bm_image, [&](bm_image* img) {
      bm_image_destroy(*img);
      delete img;
      img = nullptr;
    });
    bm_image image = *(objectMetadata->mFrame->mSpData);
    int width =
        this->width == -1 ? objectMetadata->mFrame->mWidth : this->width;
    int height =
        this->height == -1 ? objectMetadata->mFrame->mHeight : this->height;
    // 判断需不需要做convert。如果不用resize也不用转format，就直接走到jpeg_enc
    if (width != -1 || height != -1 || img->image_format == FORMAT_YUV420P) {
      bm_image_create(objectMetadata->mFrame->mHandle, height, width,
                      FORMAT_YUV420P, image.data_type, &(*img_to_enc));
      bmcv_rect_t crop_rect = {0, 0, img->width, img->height};
      bm_image_alloc_dev_mem(*img_to_enc, 1);
      bmcv_image_vpp_convert(objectMetadata->mFrame->mHandle, 1, *img,
                             img_to_enc.get(), &crop_rect);
    } else {
      img_to_enc = img;
    }

    bmcv_image_jpeg_enc(objectMetadata->mFrame->mHandle, 1, img_to_enc.get(),
                        &jpeg_data, &out_size);
#if BASE64_CPU
    // for cpu
    data = common::base64_encode((const unsigned char*)jpeg_data, out_size);
#else
    data = common::base64_encode_bmcv(objectMetadata->mFrame->mHandle,
                                      (unsigned char*)jpeg_data, out_size);
#endif
    free(jpeg_data);
  }
  if (mWsEncType == WSencType::SERIALIZED) {
    objectMetadata->fps =
        mFpsProfilers[objectMetadata->mFrame->mChannelIdInternal]->getTmpFps();
    nlohmann::json serializedObj = objectMetadata;
    data = serializedObj.dump();
  }
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
