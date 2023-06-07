#include "DecoderElement.h"

#include <dlfcn.h>
#include <sys/prctl.h>

#include <nlohmann/json.hpp>

#include "common/ObjectMetadata.h"
#include "common/logger.h"
#include "element_factory.h"

namespace sophon_stream {
namespace element {

DecoderElement::DecoderElement() {}

DecoderElement::~DecoderElement() {}

constexpr const char* DecoderElement::X;
constexpr const char* DecoderElement::Y;
constexpr const char* DecoderElement::W;
constexpr const char* DecoderElement::H;
constexpr const char* DecoderElement::NAME;
constexpr const char* DecoderElement::ROI;
constexpr const char* DecoderElement::JSON_SHARED_OBJECT_FIELD;

constexpr const char* DecoderElement::JSON_URL;
constexpr const char* DecoderElement::JSON_REOPEN_TIMES;
constexpr const char* DecoderElement::JSON_TIMEOUT;
constexpr const char* DecoderElement::JSON_SOURCE_TYPE;
constexpr const char* DecoderElement::JSON_SKIP_COUNT;
constexpr const char* DecoderElement::JSON_BATCH_SIZE;

#define JSON_MULTIMEDIA_NAME_FIELD "multimedia_name"

common::ErrorCode DecoderElement::initInternal(const std::string& json) {
  common::ErrorCode errorCode = common::ErrorCode::SUCCESS;
  do {
    auto configure = nlohmann::json::parse(json, nullptr, false);
    if (!configure.is_object()) {
      IVS_ERROR("Parse json fail or json is not object, json: {0}", json);
      errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
      break;
    }

    auto skipcountIter = configure.find(JSON_SKIP_COUNT);
    if (configure.end() != skipcountIter &&
        skipcountIter->is_number_integer()) {
      mSkipCount = skipcountIter->get<int>();
      IVS_INFO("skip count is :{0}", mSkipCount);
    }

    auto batchsizeIter = configure.find(JSON_BATCH_SIZE);
    if (configure.end() != batchsizeIter &&
        batchsizeIter->is_number_integer()) {
      mBatchSize = batchsizeIter->get<int>();
      IVS_INFO("batch size is :{0}", mBatchSize);
    }

    auto multimediaNameIt = configure.find(JSON_MULTIMEDIA_NAME_FIELD);
    if (configure.end() == multimediaNameIt || !multimediaNameIt->is_string()) {
      errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
      break;
    }
    // 根据设备类型和算法名字找到对应的工厂
    mMultiMediaName = multimediaNameIt->get<std::string>();
    //        if (sharedObjectSet.end() ==
    //        sharedObjectSet.find(*sharedObjectIt)) {
    //            const auto& sharedObject = sharedObjectIt->get<std::string>();
    //            sharedObjectSet.insert(sharedObject);
    //
    //            void* sharedObjectHandle = dlopen(sharedObject.c_str(),
    //            RTLD_NOW | RTLD_GLOBAL); if (NULL == sharedObjectHandle) {
    //                IVS_ERROR("Load dynamic shared object file fail, element
    //                id: {0:d}, "
    //                          "shared object: {1}  error info:{2}",
    //                          getId(),
    //                          sharedObject,dlerror());
    //                errorCode = common::ErrorCode::DLOPEN_FAIL;
    //                break;
    //            }
    //
    //            mSharedObjectHandles.push_back(std::shared_ptr<void>(sharedObjectHandle,
    //            [](void* sharedObjectHandle) {
    //                dlclose(sharedObjectHandle);
    //            }));
    //        }

  } while (false);

  return errorCode;
}

void DecoderElement::uninitInternal() {
  std::lock_guard<std::mutex> lk(mThreadsPoolMtx);
  for (auto& channelInfo : mThreadsPool) {
    channelInfo.second->mThreadWrapper->stop();
  }
  mThreadsPool.clear();
}

void DecoderElement::onStart() { IVS_INFO("DecoderElement start..."); }

void DecoderElement::onStop() {
  IVS_INFO("DecoderElement stop...");
  std::lock_guard<std::mutex> lk(mThreadsPoolMtx);
  for (auto& channelInfo : mThreadsPool) {
    channelInfo.second->mThreadWrapper->stop();
  }
  mThreadsPool.clear();
}

common::ErrorCode DecoderElement::doWork(int DataPipeId) {
  common::ErrorCode errorCode = common::ErrorCode::SUCCESS;
  auto data = getInputData(0, DataPipeId);
  if (!data) {
    // popInputData(0);
    return common::ErrorCode::SUCCESS;
  }
  // popInputData(0);


  std::shared_ptr<ChannelTask> channelOperate =
      std::static_pointer_cast<ChannelTask>(data);
  if (channelOperate->request.operation ==
      ChannelOperateRequest::ChannelOperate::START) {
    errorCode = startTask(channelOperate);
  } else if (channelOperate->request.operation ==
             ChannelOperateRequest::ChannelOperate::STOP) {
    errorCode = stopTask(channelOperate);
  } else if (channelOperate->request.operation ==
             ChannelOperateRequest::ChannelOperate::PAUSE) {
    errorCode = pauseTask(channelOperate);
  } else if (channelOperate->request.operation ==
             ChannelOperateRequest::ChannelOperate::RESUME) {
    errorCode = resumeTask(channelOperate);
  }

  return errorCode;
}

common::ErrorCode DecoderElement::startTask(
    std::shared_ptr<ChannelTask>& channelTask) {
  IVS_INFO("add one channel task");
  std::lock_guard<std::mutex> lk(mThreadsPoolMtx);
  // if channel exists, return success
  if (mThreadsPool.find(channelTask->request.channelId) != mThreadsPool.end()) {
    channelTask->response.errorCode = common::ErrorCode::SUCCESS;
    std::string error = "this channel is used! channel id is " +
                        std::to_string(channelTask->request.channelId);
    channelTask->response.errorInfo = error;
    IVS_WARN("{0}", error);
    return common::ErrorCode::SUCCESS;
  }

  std::string url;
  int reopentimes = -1;
  int sourceType = 0;
  common::ErrorCode ret =
      parseJson(channelTask->request.json, url, sourceType, reopentimes);
  if (ret != common::ErrorCode::SUCCESS) {
    channelTask->response.errorCode = ret;
    return ret;
  }

  int capacity = -1;
  common::ErrorCode errorCode = getOutputDatapipeCapacity(0, capacity);
  if (errorCode == common::ErrorCode::SUCCESS && capacity != -1) {
    IVS_INFO("decoder element output 0 capacity is: {0}", capacity);
  }
  std::shared_ptr<ChannelInfo> channelInfo = std::make_shared<ChannelInfo>();

  channelInfo->mThreadWrapper = std::make_shared<ThreadWrapper>();
  channelInfo->mMtx = std::make_shared<std::mutex>();
  channelInfo->mCv = std::make_shared<std::condition_variable>();
  channelInfo->mThreadWrapper->init(
      [this, channelInfo, channelTask]() -> common::ErrorCode {
        prctl(PR_SET_NAME,
              std::to_string(channelTask->request.channelId).c_str());

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

        common::ErrorCode ret = channelInfo->mSpDecoder->init(
            getSide(), getDeviceId(), channelTask->request.json);
        if (common::ErrorCode::SUCCESS != ret) {
          ret = reopen(5, channelTask, channelInfo);
          if (ret == common::ErrorCode::SUCCESS) {
            channelTask->response.errorCode = common::ErrorCode::SUCCESS;
            channelInfo->mCv->notify_one();
          } else {
            channelTask->response.errorCode = common::ErrorCode::TIMEOUT;
            std::string error = "time out! channel id is " +
                                std::to_string(channelTask->request.channelId);
            channelTask->response.errorInfo = error;
            IVS_ERROR("{0}", error);
            channelInfo->mThreadWrapper->stop(false);
          }
        } else {
          channelTask->response.errorCode = common::ErrorCode::SUCCESS;
          channelInfo->mCv->notify_one();
        }
        return ret;
      },
      [this, channelInfo, channelTask, sourceType, reopentimes,
       capacity]() -> common::ErrorCode {
        return process(false, sourceType, reopentimes, capacity, channelTask,
                       channelInfo);
      },
      [this, channelInfo, channelTask, sourceType, reopentimes,
       capacity]() -> common::ErrorCode {
        return process(true, sourceType, reopentimes, capacity, channelTask,
                       channelInfo);
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

  IVS_INFO("add one channel task finished!");
  return channelTask->response.errorCode;
}

common::ErrorCode DecoderElement::stopTask(
    std::shared_ptr<ChannelTask>& channelTask) {
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

common::ErrorCode DecoderElement::pauseTask(
    std::shared_ptr<ChannelTask>& channelTask) {
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

common::ErrorCode DecoderElement::resumeTask(
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

common::ErrorCode DecoderElement::parseJson(const std::string& json,
                                            std::string& url, int& sourceType,
                                            int& reopentimes) {
  common::ErrorCode errorCode = common::ErrorCode::SUCCESS;
  do {
    auto configure = nlohmann::json::parse(json, nullptr, false);
    if (!configure.is_object()) {
      IVS_ERROR("Parse json fail or json is not object, json: {0}", json);
      errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
      break;
    }

    auto urlIt = configure.find(JSON_URL);
    if (configure.end() == urlIt || !urlIt->is_string()) {
      IVS_ERROR(
          "Can not find {0} with string type in element json configure, json: "
          "{1}",
          JSON_URL, json);
      errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
      break;
    }
    url = urlIt->get<std::string>();

    auto reopenIter = configure.find(JSON_REOPEN_TIMES);
    if (configure.end() == reopenIter || !reopenIter->is_number_integer()) {
      IVS_ERROR(
          "Can not find {0} with integer type in element json configure, json: "
          "{1}",
          JSON_REOPEN_TIMES, json);
      errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
      break;
    }
    reopentimes = reopenIter->get<int>();

    auto timeoutIt = configure.find(JSON_TIMEOUT);
    if (configure.end() != timeoutIt && timeoutIt->is_number_integer()) {
      int timeout = timeoutIt->get<int>();
    }

    auto sourceTypeIter = configure.find(JSON_SOURCE_TYPE);
    if (configure.end() == sourceTypeIter ||
        !sourceTypeIter->is_number_integer()) {
      IVS_ERROR(
          "Can not find {0} with integer type in element json configure, json: "
          "{1}",
          JSON_SOURCE_TYPE, json);
      errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
      break;
    }

    sourceType = sourceTypeIter->get<int>();

  } while (false);

  return errorCode;
}

common::ErrorCode DecoderElement::process(
    const bool lastFrame, const int sourceType, const int reopentimes,
    const int capacity, const std::shared_ptr<ChannelTask>& channelTask,
    const std::shared_ptr<ChannelInfo>& channelInfo) {
  std::shared_ptr<common::ObjectMetadata> objectMetadata;
  common::ErrorCode ret = channelInfo->mSpDecoder->process(objectMetadata);
  if (ret == common::ErrorCode::SUCCESS ||
      ret == common::ErrorCode::STREAM_END) {
    if (ret == common::ErrorCode::STREAM_END) {
      if (sourceType == 2) {
        ret = reopen(reopentimes, channelTask, channelInfo);
      }
      if (ret == common::ErrorCode::SUCCESS) return ret;
      // end of stream , detach thread and erase in mThreadsPool,
      std::lock_guard<std::mutex> lk(mThreadsPoolMtx);
      channelTask->response.errorCode = ret;
      channelInfo->mThreadWrapper->stop(false);
      channelInfo->mSpDecoder->uninit();
      auto iter = mThreadsPool.find(channelTask->request.channelId);
      if (iter != mThreadsPool.end()) {
        mThreadsPool.erase(iter);
      }
      // lastFrame = true;
    }
    if (objectMetadata->mFrame == nullptr) {
      if (ret == common::ErrorCode::STREAM_END) {
        IVS_CRITICAL("last frame! but not send!");
      }
      return common::ErrorCode::SUCCESS;
    }
    objectMetadata->mFrame->mChannelId = channelTask->request.channelId;
    if (lastFrame) objectMetadata->mFrame->mEndOfStream = true;

    if (sourceType == 2) {
      channelInfo->mFrameCount++;
      if (channelInfo->mFrameCount % (mSkipCount + 1) != 0 &&
          !objectMetadata->mFrame->mEndOfStream) {
        return common::ErrorCode::SUCCESS;
      }
    }

    int outputDatapipeSize = -1;
    getOutputDatapipeSize(0, objectMetadata->mFrame->mChannelId,
                          outputDatapipeSize);
    if (sourceType == 2 && outputDatapipeSize >= capacity) {
      IVS_WARN(
          "output datapipe0 size is too high, size is:{0}, capacity is:{1}",
          outputDatapipeSize, capacity);
      return common::ErrorCode::SUCCESS;
    }
    int dataPipeId = objectMetadata->mFrame->mChannelId % getOutputConnector(0)->getDataPipeCount();
    // push data to next element
    common::ErrorCode errorCode =
        pushOutputData(0, dataPipeId, std::static_pointer_cast<void>(objectMetadata));
    if (common::ErrorCode::SUCCESS != errorCode) {
      IVS_WARN("Send data fail, element id: {0}, output port: {1}, data: {2:p}",
               getId(), 0, static_cast<void*>(objectMetadata.get()));
      return errorCode;
    }
  } else {
    if (ret == common::ErrorCode::NOT_VIDEO_CHANNEL) {
      return ret;
    }
    IVS_ERROR("getFrame failed,error code is : {0}----channel id:{1}", (int)ret,
              channelTask->request.channelId);
    channelTask->response.errorCode = ret;
    pushOutputData(2, 0, channelTask);
    // 和master协定, 报错即退出
    std::lock_guard<std::mutex> lk(mThreadsPoolMtx);
    channelInfo->mThreadWrapper->stop(false);
    channelInfo->mSpDecoder->uninit();
    auto iter = mThreadsPool.find(channelTask->request.channelId);
    if (iter != mThreadsPool.end()) {
      mThreadsPool.erase(iter);
    }
  }
  return ret;
}

common::ErrorCode DecoderElement::reopen(
    const int reopentimes, const std::shared_ptr<ChannelTask>& channelTask,
    const std::shared_ptr<ChannelInfo>& channelInfo) {
  int times = 0;

  common::ErrorCode ret = common::ErrorCode::ERR_FFMPEG_INPUT_CTX_OPEN;

  while (channelInfo->mThreadWrapper->mThreadStatus !=
             ThreadWrapper::ThreadStatus::STOP &&
         ret != common::ErrorCode::SUCCESS) {
    if (times >= reopentimes && reopentimes >= 0) {
      break;
    }

    channelInfo->mSpDecoder->uninit();
    ret = channelInfo->mSpDecoder->init(getSide(), getDeviceId(),
                                        channelTask->request.json);
    if (ret != common::ErrorCode::SUCCESS) {
      IVS_ERROR("reopen failed! errorcode:{0}, reopentimes:{1}", (int)ret,
                reopentimes);
      channelTask->response.errorCode = ret;
    }
    if (reopentimes >= 0) times++;
  }
  if (ret != common::ErrorCode::SUCCESS) {
    pushOutputData(2, 0, channelTask);
  }
  return ret;
}

REGISTER_WORKER(DecoderElement::NAME, DecoderElement)

}  // namespace element
}  // namespace sophon_stream
