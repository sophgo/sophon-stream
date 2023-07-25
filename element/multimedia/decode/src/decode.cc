//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#include "decode.h"

#include <dlfcn.h>
#include <sys/prctl.h>

#include <nlohmann/json.hpp>

#include "common/logger.h"
#include "common/object_metadata.h"
#include "decoder.h"
#include "element_factory.h"

namespace sophon_stream {
namespace element {
namespace decode {

Decode::Decode() {}

Decode::~Decode() {
  std::lock_guard<std::mutex> lk(mThreadsPoolMtx);
  for (auto& channelInfo : mThreadsPool) {
    channelInfo.second->mThreadWrapper->stop();
  }
  mThreadsPool.clear();
}

common::ErrorCode Decode::initInternal(const std::string& json) {
  common::ErrorCode errorCode = common::ErrorCode::SUCCESS;
  do {
    auto configure = nlohmann::json::parse(json, nullptr, false);
    if (!configure.is_object()) {
      IVS_ERROR("Parse json fail or json is not object, json: {0}", json);
      errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
      break;
    }
    mChannelCount = 0;
    mFpsProfiler.config("fps_decode", 100);
  } while (false);

  return errorCode;
}

void Decode::onStart() { IVS_INFO("Decode start..."); }

void Decode::onStop() {
  IVS_INFO("Decode stop...");
  std::lock_guard<std::mutex> lk(mThreadsPoolMtx);
  for (auto& channelInfo : mThreadsPool) {
    channelInfo.second->mThreadWrapper->stop();
  }
  mThreadsPool.clear();
}

common::ErrorCode Decode::doWork(int dataPipeId) {
  common::ErrorCode errorCode = common::ErrorCode::SUCCESS;
  int inputPort = 0;
  auto data = popInputData(inputPort, dataPipeId);
  if (!data) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    return errorCode;
  }

  std::shared_ptr<ChannelTask> channelTask =
      std::static_pointer_cast<ChannelTask>(data);
  if (channelTask->request.operation ==
      ChannelOperateRequest::ChannelOperate::START) {
    errorCode = startTask(channelTask);
  } else if (channelTask->request.operation ==
             ChannelOperateRequest::ChannelOperate::STOP) {
    errorCode = stopTask(channelTask);
  } else if (channelTask->request.operation ==
             ChannelOperateRequest::ChannelOperate::PAUSE) {
    errorCode = pauseTask(channelTask);
  } else if (channelTask->request.operation ==
             ChannelOperateRequest::ChannelOperate::RESUME) {
    errorCode = resumeTask(channelTask);
  }

  return errorCode;
}

common::ErrorCode Decode::parse_channel_task(
    std::shared_ptr<ChannelTask>& channelTask) {
  common::ErrorCode errorCode = common::ErrorCode::SUCCESS;
  const std::string json = channelTask->request.json;
  do {
    auto configure = nlohmann::json::parse(json, nullptr, false);
    if (!configure.is_object()) {
      errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
      IVS_ERROR("json parse failed! json:{0}", json);
      break;
    }

    auto urlIt = configure.find(JSON_URL);
    if (configure.end() == urlIt || !urlIt->is_string()) {
      IVS_ERROR(
          "Can not find {0} with string type in worker json configure, json: "
          "{1}",
          JSON_URL, json);
      errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
      break;
    }
    channelTask->request.url = urlIt->get<std::string>();

    auto channelIdIt = configure.find(JSON_CHANNEL_ID);
    if (configure.end() == channelIdIt || !channelIdIt->is_number_integer()) {
      IVS_ERROR(
          "Can not find {0} with int type in worker json configure, json: "
          "{1}",
          JSON_CHANNEL_ID, json);
      errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
      break;
    }
    channelTask->request.channelId = channelIdIt->get<int>();

    auto skipElementIt = configure.find(JSON_SKIP_ELEMENT);
    if (configure.end() != skipElementIt && skipElementIt->is_array()) {
      // 不填写skip_element，视为不跳过
      channelTask->request.skip_element =
          skipElementIt->get<std::vector<int>>();
    }

    auto sourceTypeIdIt = configure.find(JSON_SOURCE_TYPE);
    if (configure.end() == sourceTypeIdIt || !sourceTypeIdIt->is_string()) {
      IVS_ERROR(
          "Can not find {0} with string type in worker json configure, json: "
          "{1}",
          JSON_SOURCE_TYPE, json);
      errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
      break;
    }
    auto sourceType = sourceTypeIdIt->get<std::string>();
    channelTask->request.sourceType =
        ChannelOperateRequest::SourceType::UNKNOWN;
    if (sourceType == "RTSP") {
      IVS_INFO("Source type is {}", sourceType);
      if (channelTask->request.url.compare(0, 7, "rtsp://") != 0) {
        IVS_ERROR("RTSP format error");
        errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
        break;
      }
      channelTask->request.sourceType = ChannelOperateRequest::SourceType::RTSP;
    } else if (sourceType == "RTMP") {
      IVS_INFO("Source type is {0}", sourceType);
      if (channelTask->request.url.compare(0, 7, "rtmp://") != 0) {
        IVS_ERROR("RTMP format error");
        errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
        break;
      }
      channelTask->request.sourceType = ChannelOperateRequest::SourceType::RTMP;
    } else if (sourceType == "VIDEO") {
      IVS_INFO("Source type is {0}", sourceType);
      channelTask->request.sourceType =
          ChannelOperateRequest::SourceType::VIDEO;
    } else if (sourceType == "IMG_DIR") {
      IVS_INFO("Source type is {0}", sourceType);
      channelTask->request.sourceType =
          ChannelOperateRequest::SourceType::IMG_DIR;
    } else if (sourceType == "BASE64") {
      IVS_INFO("Source type is {0}", sourceType);
      channelTask->request.sourceType =
          ChannelOperateRequest::SourceType::BASE64;
    } else {
      IVS_ERROR("{0} error, please input RTSP, RTMP, VIDEO, IMG_DIR or BASE64",
                JSON_SOURCE_TYPE);
      errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
      break;
    }

    channelTask->request.base64Port = 12348;
    if (channelTask->request.sourceType ==
        ChannelOperateRequest::SourceType::BASE64) {
      auto base64PortIt = configure.find(JSON_BASE64_PORT);
      if (configure.end() == base64PortIt ||
          !channelIdIt->is_number_integer()) {
        IVS_WARN(
            "Can not find {0} with int type in worker json configure, json: "
            "{1}",
            JSON_LOOP_NUM, json);
        errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
        break;
      }
      channelTask->request.base64Port = base64PortIt->get<int>();
    }

    channelTask->request.loopNum = 1;
    if (channelTask->request.sourceType ==
            ChannelOperateRequest::SourceType::VIDEO ||
        channelTask->request.sourceType ==
            ChannelOperateRequest::SourceType::IMG_DIR) {
      auto loopNumIt = configure.find(JSON_LOOP_NUM);
      if (configure.end() == loopNumIt || !channelIdIt->is_number_integer()) {
        IVS_WARN(
            "Can not find {0} with int type in worker json configure, json: "
            "{1}",
            JSON_LOOP_NUM, json);
        errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
        break;
      }
      // if loopNum is 0, infinite loop
      channelTask->request.loopNum =
          loopNumIt->get<int>() == 0 ? 2147483647 : loopNumIt->get<int>();
    }

    channelTask->request.fps = 1;
    auto fpsIt = configure.find(JSON_FPS);
    if (configure.end() == fpsIt || !channelIdIt->is_number_integer()) {
    } else {
      channelTask->request.fps = fpsIt->get<double>();
    }

    channelTask->request.sampleInterval = 1;
    auto sampleIntervalIt = configure.find(JSON_SAMPLE_INTERVAL);
    if (configure.end() == sampleIntervalIt ||
        !channelIdIt->is_number_integer()) {
    } else {
      channelTask->request.sampleInterval = sampleIntervalIt->get<int>();
    }

    channelTask->request.sampleStrategy =
        ChannelOperateRequest::SampleStrategy::DROP;
    auto strategyIt = configure.find(JSON_SAMPLE_STRATEGY);
    if (configure.end() != strategyIt && strategyIt->is_string()) {
      channelTask->request.sampleStrategy =
          strategyIt->get<std::string>() == "KEEP"
              ? ChannelOperateRequest::SampleStrategy::KEEP
              : ChannelOperateRequest::SampleStrategy::DROP;
    }

  } while (false);

  return errorCode;
}

common::ErrorCode Decode::startTask(std::shared_ptr<ChannelTask>& channelTask) {
  IVS_INFO("add one channel task");
  parse_channel_task(channelTask);
  std::lock_guard<std::mutex> lk(mThreadsPoolMtx);

  if (mThreadsPool.find(channelTask->request.channelId) != mThreadsPool.end()) {
    channelTask->response.errorCode = common::ErrorCode::SUCCESS;
    std::string error = "this channel is used! channel id is " +
                        std::to_string(channelTask->request.channelId);
    channelTask->response.errorInfo = error;
    IVS_WARN("{0}", error);
    return common::ErrorCode::SUCCESS;
  }

  std::shared_ptr<ChannelInfo> channelInfo = std::make_shared<ChannelInfo>();

  channelInfo->mThreadWrapper = std::make_shared<ThreadWrapper>();
  channelInfo->mMtx = std::make_shared<std::mutex>();
  channelInfo->mCv = std::make_shared<std::condition_variable>();
  channelInfo->mThreadWrapper->init(
      [this, channelInfo, channelTask]() -> common::ErrorCode {
        prctl(PR_SET_NAME,
              std::to_string(channelTask->request.channelId).c_str());
        IVS_DEBUG("Decoder initialized! Channel Id is {0}",
                  channelTask->request.channelId);
        channelInfo->mSpDecoder = std::make_shared<Decoder>();
        if (!channelInfo->mSpDecoder) {
          channelTask->response.errorCode =
              common::ErrorCode::MAKE_ALGORITHM_API_FAIL;
          std::string error = "Make multimedia api failed! channel id is " +
                              std::to_string(channelTask->request.channelId);
          channelTask->response.errorInfo = error;
          IVS_ERROR("{0}", error);
          channelInfo->mThreadWrapper->stop(false);
          channelInfo->mSpDecoder->uninit();
          return common::ErrorCode(-1);
        }

        IVS_INFO("channel info decoder address: {0:p}",
                 static_cast<void*>(channelInfo->mSpDecoder.get()));

        common::ErrorCode ret =
            channelInfo->mSpDecoder->init(getDeviceId(), channelTask->request);

        channelTask->response.errorCode = common::ErrorCode::SUCCESS;
        channelInfo->mCv->notify_one();

        return ret;
      },
      [this, channelInfo, channelTask]() -> common::ErrorCode {
        return process(channelTask, channelInfo);
      },
      [this, channelInfo, channelTask]() -> common::ErrorCode {
        return process(channelTask, channelInfo);
      });
  channelInfo->mThreadWrapper->start();
  std::unique_lock<std::mutex> uq(*channelInfo->mMtx);
  std::cv_status currentNoTimeout =
      channelInfo->mCv->wait_for(uq, std::chrono::seconds(120));
  if (currentNoTimeout == std::cv_status::timeout) {
    channelTask->response.errorCode = common::ErrorCode::TIMEOUT;
    return common::ErrorCode::TIMEOUT;
  }
  mThreadsPool.insert(
      std::make_pair(channelTask->request.channelId, channelInfo));

  int channel_id = channelTask->request.channelId;
  if (mChannelIdInternal.find(channel_id) == mChannelIdInternal.end()) {
    mChannelIdInternal[channel_id] = mChannelCount++;
  }

  IVS_INFO("add one channel task finished!");
  return channelTask->response.errorCode;
}

common::ErrorCode Decode::stopTask(std::shared_ptr<ChannelTask>& channelTask) {
  std::lock_guard<std::mutex> lk(mThreadsPoolMtx);
  auto itTask = mThreadsPool.find(channelTask->request.channelId);
  if (itTask == mThreadsPool.end()) {
    channelTask->response.errorCode =
        common::ErrorCode::DECODE_CHANNEL_NOT_FOUND;
    std::string error = "this channel is not found! channel id is " +
                        std::to_string(channelTask->request.channelId);
    IVS_ERROR("{0}", error);
    return common::ErrorCode::DECODE_CHANNEL_NOT_FOUND;
  }
  common::ErrorCode errorCode = itTask->second->mThreadWrapper->stop();
  itTask->second->mSpDecoder->uninit();
  mThreadsPool.erase(itTask);
  channelTask->response.errorCode = errorCode;
  return errorCode;
}

common::ErrorCode Decode::pauseTask(std::shared_ptr<ChannelTask>& channelTask) {
  std::lock_guard<std::mutex> lk(mThreadsPoolMtx);
  auto itTask = mThreadsPool.find(channelTask->request.channelId);
  if (itTask == mThreadsPool.end()) {
    channelTask->response.errorCode =
        common::ErrorCode::DECODE_CHANNEL_NOT_FOUND;
    return common::ErrorCode::DECODE_CHANNEL_NOT_FOUND;
  }
  common::ErrorCode errorCode = itTask->second->mThreadWrapper->pause();
  mThreadsPool.erase(itTask);
  channelTask->response.errorCode = errorCode;
  return errorCode;
}

common::ErrorCode Decode::resumeTask(
    std::shared_ptr<ChannelTask>& channelTask) {
  std::lock_guard<std::mutex> lk(mThreadsPoolMtx);
  auto itTask = mThreadsPool.find(channelTask->request.channelId);
  if (itTask == mThreadsPool.end()) {
    channelTask->response.errorCode =
        common::ErrorCode::DECODE_CHANNEL_NOT_FOUND;
    return common::ErrorCode::DECODE_CHANNEL_NOT_FOUND;
  }
  common::ErrorCode errorCode = itTask->second->mThreadWrapper->resume();
  mThreadsPool.erase(itTask);
  channelTask->response.errorCode = errorCode;
  return errorCode;
}

common::ErrorCode Decode::process(
    const std::shared_ptr<ChannelTask>& channelTask,
    const std::shared_ptr<ChannelInfo>& channelInfo) {
  std::shared_ptr<common::ObjectMetadata> objectMetadata;
  common::ErrorCode ret = channelInfo->mSpDecoder->process(objectMetadata);
  mFpsProfiler.add(1);
  if (ret == common::ErrorCode::STREAM_END) {
    // end of stream , detach thread and erase in mThreadsPool,
    std::lock_guard<std::mutex> lk(mThreadsPoolMtx);
    channelTask->response.errorCode = ret;
    channelInfo->mThreadWrapper->stop(false);
    channelInfo->mSpDecoder->uninit();
    auto iter = mThreadsPool.find(channelTask->request.channelId);
    if (iter != mThreadsPool.end()) {
      mThreadsPool.erase(iter);
    }
  }
  int channel_id = channelTask->request.channelId;
  std::vector<int> skip_elements = channelTask->request.skip_element;
  objectMetadata->mSkipElements = skip_elements;
  objectMetadata->mFrame->mChannelId = channel_id;
  objectMetadata->mFrame->mChannelIdInternal = mChannelIdInternal[channel_id];

  // push data to next element
  if (objectMetadata->mFilter && !objectMetadata->mFrame->mEndOfStream &&
      channelTask->request.sampleStrategy ==
          ChannelOperateRequest::SampleStrategy::DROP) {
    return common::ErrorCode::SUCCESS;
  }
  int channel_id_internal = objectMetadata->mFrame->mChannelIdInternal;
  int outputPort = 0;
  if (!getSinkElementFlag()) {
    std::vector<int> outputPorts = getOutputPorts();
    outputPort = outputPorts[0];
  }
  int dataPipeId =
      getSinkElementFlag()
          ? 0
          : (channel_id_internal % getOutputConnectorCapacity(outputPort));
  common::ErrorCode errorCode = pushOutputData(
      outputPort, dataPipeId, std::static_pointer_cast<void>(objectMetadata));
  if (common::ErrorCode::SUCCESS != errorCode) {
    IVS_WARN("Send data fail, element id: {0}, output port: {1}, data: {2:p}",
             getId(), 0, static_cast<void*>(objectMetadata.get()));
    return errorCode;
  }
  return ret;
}

REGISTER_WORKER("decode", Decode)

}  // namespace decode
}  // namespace element
}  // namespace sophon_stream
