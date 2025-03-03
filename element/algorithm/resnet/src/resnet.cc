//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#include "resnet.h"

using namespace std::chrono_literals;

namespace sophon_stream {
namespace element {
namespace resnet {

// json的task_type --> resnet的TaskType映射
std::unordered_map<std::string, TaskType> taskMap{
    {"SingleLabel", TaskType::SingleLabel},
    {"FeatureExtract", TaskType::FeatureExtract},
    {"MultiLabel", TaskType::MultiLabel}};

ResNet::ResNet() {}

ResNet::~ResNet() {}

common::ErrorCode ResNet::initContext(const std::string& json) {
  common::ErrorCode errorCode = common::ErrorCode::SUCCESS;
  do {
    auto configure = nlohmann::json::parse(json, nullptr, false);
    if (!configure.is_object()) {
      errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
      break;
    }

    auto modelPathIt = configure.find(CONFIG_INTERNAL_MODEL_PATH_FIELD);

    mContext->bgr2rgb = true;
    auto bgr2rgbIt = configure.find(CONFIG_INTERNAL_THRESHOLD_BGR2RGB_FIELD);
    mContext->bgr2rgb = bgr2rgbIt->get<bool>();

    auto bgr2grayIt = configure.find(CONFIG_INTERNAL_THRESHOLD_BGR2GRAY_FIELD);
    if (bgr2grayIt != configure.end()) {
      mContext->bgr2gray = bgr2grayIt->get<bool>();
    } else {
      mContext->bgr2gray = false;
    }

    // 如果没有这个字段，那么默认采用单分类后处理
    auto task_it = configure.find(CONFIG_INTERNAL_TASK_TYPE_FIELD);
    if (task_it != configure.end()) {
      std::string taskName = task_it->get<std::string>();
      // 保证json里的task_type字段必须是预设的值之一
      STREAM_CHECK(taskMap.count(taskName) != 0,
                   "Invalid Task Type in Resnet Config File!");
      mContext->taskType = taskMap[taskName];
    }

    auto meanIt = configure.find(CONFIG_INTERNAL_THRESHOLD_MEAN_FIELD);
    mContext->mean = meanIt->get<std::vector<float>>();
    assert(mContext->mean.size() == 3);

    auto stdIt = configure.find(CONFIG_INTERNAL_THRESHOLD_STD_FIELD);
    mContext->stdd = stdIt->get<std::vector<float>>();
    assert(mContext->stdd.size() == 3);

    // 1. get network
    BMNNHandlePtr handle = std::make_shared<BMNNHandle>(mContext->deviceId);
    mContext->bmContext = std::make_shared<BMNNContext>(
        handle, modelPathIt->get<std::string>().c_str());
    mContext->bmNetwork = mContext->bmContext->network(0);
    mContext->handle = handle->handle();

    // 2. get input
    mContext->max_batch = mContext->bmNetwork->maxBatch();
    auto inputTensor = mContext->bmNetwork->inputTensor(0);
    mContext->input_num = mContext->bmNetwork->m_netinfo->input_num;
    mContext->m_net_channel = inputTensor->get_shape()->dims[1];
    mContext->net_h = inputTensor->get_shape()->dims[2];
    mContext->net_w = inputTensor->get_shape()->dims[3];

    // 3. get output
    mContext->output_num = mContext->bmNetwork->outputTensorNum();
    assert(mContext->output_num > 0);
    mContext->min_dim =
        mContext->bmNetwork->outputTensor(0)->get_shape()->num_dims;
    mContext->class_num =
        mContext->bmNetwork->outputTensor(0)->get_shape()->dims[1];

    if (mContext->taskType == TaskType::MultiLabel) {
      // 多标签输出的任务下，可以为每个任务配置输出的阈值
      auto class_thresh_it = configure.find(CONFIG_INTERNAL_CLASS_THRESH_FIELD);
      // 如果未配置，则默认全0.5，否则按照配置值设置
      if (class_thresh_it == configure.end()) {
        mContext->class_thresh = std::vector<float>(mContext->output_num, 0.5);
      } else {
        mContext->class_thresh = class_thresh_it->get<std::vector<float>>();
        STREAM_CHECK(mContext->class_thresh.size() == mContext->output_num,
                     "Invalid Model or ClassThresh List!");
      }
    }

    // 4.converto
    float input_scale = inputTensor->get_scale();
    mContext->converto_attr.alpha_0 = 1 / (255. * mContext->stdd[0]) * input_scale;
    mContext->converto_attr.alpha_1 = 1 / (255. * mContext->stdd[1]) * input_scale;
    mContext->converto_attr.alpha_2 = 1 / (255. * mContext->stdd[2]) * input_scale;
    mContext->converto_attr.beta_0 = (-mContext->mean[0] / mContext->stdd[0]) * input_scale;
    mContext->converto_attr.beta_1 = (-mContext->mean[1] / mContext->stdd[1]) * input_scale;
    mContext->converto_attr.beta_2 = (-mContext->mean[2] / mContext->stdd[2]) * input_scale;

    // 5. roi
    auto roi_it = configure.find(CONFIG_INTERNAL_ROI_FILED);
    if (roi_it == configure.end()) {
      mContext->roi_predefined = false;
    } else {
      mContext->roi_predefined = true;
      mContext->roi.start_x =
          roi_it->find(CONFIG_INTERNAL_LEFT_FILED)->get<int>();
      mContext->roi.start_y =
          roi_it->find(CONFIG_INTERNAL_TOP_FILED)->get<int>();
      mContext->roi.crop_w =
          roi_it->find(CONFIG_INTERNAL_WIDTH_FILED)->get<int>();
      mContext->roi.crop_h =
          roi_it->find(CONFIG_INTERNAL_HEIGHT_FILED)->get<int>();
    }

  } while (false);
  return common::ErrorCode::SUCCESS;
}

common::ErrorCode ResNet::initInternal(const std::string& json) {
  common::ErrorCode errorCode = common::ErrorCode::SUCCESS;
  do {
    // json是否正确
    auto configure = nlohmann::json::parse(json, nullptr, false);
    if (!configure.is_object()) {
      errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
      break;
    }

    mFpsProfiler.config("fps_resnet", 100);

    // 新建context,预处理,推理和后处理对象
    mContext = std::make_shared<ResNetContext>();
    mMultiTask = std::make_shared<ResNetMultiTask>();

    // 新建context
    mContext->deviceId = getDeviceId();
    initContext(configure.dump());

    // 推理初始化
    mMultiTask->init(mContext);

    mBatch = mContext->max_batch;
  } while (false);
  return errorCode;
}

void ResNet::process(common::ObjectMetadatas& objectMetadatas) {
  common::ErrorCode errorCode = common::ErrorCode::SUCCESS;

  // 推理
  errorCode = mMultiTask->multiTask(mContext, objectMetadatas);
  if (common::ErrorCode::SUCCESS != errorCode) {
    for (unsigned i = 0; i < objectMetadatas.size(); i++) {
      objectMetadatas[i]->mErrorCode = errorCode;
    }
    return;
  }
}

common::ErrorCode ResNet::doWork(int dataPipeId) {
  common::ErrorCode errorCode = common::ErrorCode::SUCCESS;

  common::ObjectMetadatas objectMetadatas;
  std::vector<int> inputPorts = getInputPorts();
  int inputPort = inputPorts[0];
  int outputPort = 0;
  if (!getSinkElementFlag()) {
    std::vector<int> outputPorts = getOutputPorts();
    int outputPort = outputPorts[0];
  }

  common::ObjectMetadatas pendingObjectMetadatas;

  while (objectMetadatas.size() < mBatch &&
         (getThreadStatus() == ThreadStatus::RUN)) {
    // 如果队列为空则等待
    auto data = popInputData(inputPort, dataPipeId);
    if (!data) {
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
      continue;
    }

    auto objectMetadata =
        std::static_pointer_cast<common::ObjectMetadata>(data);

    if (!objectMetadata->mFilter) objectMetadatas.push_back(objectMetadata);

    pendingObjectMetadatas.push_back(objectMetadata);

    if (objectMetadata->mFrame->mEndOfStream) {
      break;
    }
  }
  process(objectMetadatas);

  for (auto& objectMetadata : pendingObjectMetadatas) {
    int channel_id_internal = objectMetadata->mFrame->mChannelIdInternal;
    int outDataPipeId =
        getSinkElementFlag()
            ? 0
            : (channel_id_internal % getOutputConnectorCapacity(outputPort));
    errorCode = pushOutputData(outputPort, outDataPipeId,
                               std::static_pointer_cast<void>(objectMetadata));
    if (common::ErrorCode::SUCCESS != errorCode) {
      IVS_WARN(
          "Send data fail, element id: {0:d}, output port: {1:d}, data: "
          "{2:p}",
          getId(), outputPort, static_cast<void*>(objectMetadata.get()));
    }
  }
  mFpsProfiler.add(objectMetadatas.size());

  return common::ErrorCode::SUCCESS;
}

REGISTER_WORKER("resnet", ResNet)

}  // namespace resnet
}  // namespace element
}  // namespace sophon_stream
