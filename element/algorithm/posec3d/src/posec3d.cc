//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#include "posec3d.h"

using namespace std::chrono_literals;

namespace sophon_stream {
namespace element {
namespace posec3d {

Posec3d::Posec3d() {}

Posec3d::~Posec3d() {}

const std::string Posec3d::elementName = "posec3d";

// 解析参数，需要解析的参数应该都定义在context.h中
common::ErrorCode Posec3d::initContext(const std::string& json) {
  common::ErrorCode errorCode = common::ErrorCode::SUCCESS;
  do {
    auto configure = nlohmann::json::parse(json, nullptr, false);
    if (!configure.is_object()) {
      errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
      break;
    }
    // 模型路径
    auto modelPathIt = configure.find(CONFIG_INTERNAL_MODEL_PATH_FIELD);

    // 1. get network
    BMNNHandlePtr handle = std::make_shared<BMNNHandle>(mContext->deviceId);
    mContext->bmContext = std::make_shared<BMNNContext>(
        handle, modelPathIt->get<std::string>().c_str());
    mContext->bmNetwork = mContext->bmContext->network(0);
    mContext->handle = handle->handle();

    // 2. get input
    auto frameNum = configure.find(CONFIG_INTERNAL_FRAMES_NUM_FIELD);
    mContext->max_batch = frameNum->get<int>();
    auto inputTensor = mContext->bmNetwork->inputTensor(0);
    mContext->input_num = mContext->bmNetwork->m_netinfo->input_num;
    mContext->m_net_crops_clips = inputTensor->get_shape()->dims[0];
    mContext->m_net_channel = inputTensor->get_shape()->dims[2];
    mContext->m_net_keypoints = inputTensor->get_shape()->dims[1];
    mContext->net_h = inputTensor->get_shape()->dims[3];
    mContext->net_w = inputTensor->get_shape()->dims[4];

    // 3. get output
    mContext->output_num = mContext->bmNetwork->outputTensorNum();
    auto output_tensor = mContext->bmNetwork->outputTensor(0);
    auto output_shape = output_tensor->get_shape();
    int cls_num = output_shape->dims[1];
    auto classNamesFileIt =
        configure.find(CONFIG_INTERNAL_CLASS_NAMES_FILE_FIELD);
    std::string class_names_file = classNamesFileIt->get<std::string>();
    std::ifstream istream;
    istream.open(class_names_file);
    assert(istream.is_open());
    std::string line;
    while (std::getline(istream, line)) {
      line = line.substr(0, line.length());
      mContext->class_names.push_back(line);
    }
    istream.close();
    assert(cls_num == mContext->class_names.size());

    // 4.converto
    mContext->input_scale = inputTensor->get_scale();
  } while (false);
  return common::ErrorCode::SUCCESS;
}

common::ErrorCode Posec3d::initInternal(const std::string& json) {
  common::ErrorCode errorCode = common::ErrorCode::SUCCESS;
  do {
    // json是否正确
    auto configure = nlohmann::json::parse(json, nullptr, false);
    if (!configure.is_object()) {
      errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
      break;
    }
    // stageNameIt pre \infer \post
    auto stageNameIt = configure.find(CONFIG_INTERNAL_STAGE_NAME_FIELD);
    if (configure.end() != stageNameIt && stageNameIt->is_array()) {
      std::vector<std::string> stages =
          stageNameIt->get<std::vector<std::string>>();
      if (std::find(stages.begin(), stages.end(), "pre") != stages.end()) {
        use_pre = true;
        mFpsProfilerName = "fps_posec3d_pre";
      }
      if (std::find(stages.begin(), stages.end(), "infer") != stages.end()) {
        use_infer = true;
        mFpsProfilerName = "fps_posec3d_infer";
      }
      if (std::find(stages.begin(), stages.end(), "post") != stages.end()) {
        use_post = true;
        mFpsProfilerName = "fps_posec3d_post";
      }

      mFpsProfiler.config(mFpsProfilerName, 100);
    }

    // 新建context,预处理,推理和后处理对象
    mContext = std::make_shared<Posec3dContext>();
    mPreProcess = std::make_shared<Posec3dPreProcess>();
    mInference = std::make_shared<Posec3dInference>();
    mPostProcess = std::make_shared<Posec3dPostProcess>();

    if (!mPreProcess || !mInference || !mPostProcess || !mContext) {
      break;
    }

    mContext->deviceId = getDeviceId();
    initContext(configure.dump());
    // 前处理初始化
    mPreProcess->init(mContext);
    // 推理初始化
    mInference->init(mContext);
    // 后处理初始化
    mPostProcess->init(mContext);

  } while (false);
  return errorCode;
}

void Posec3d::process(common::ObjectMetadatas& objectMetadatas) {
  common::ErrorCode errorCode = common::ErrorCode::SUCCESS;
  // 预处理
  if (use_pre) {
    errorCode = mPreProcess->preProcess(mContext, objectMetadatas);
    if (common::ErrorCode::SUCCESS != errorCode) {
      for (unsigned i = 0; i < objectMetadatas.size(); i++) {
        objectMetadatas[i]->mErrorCode = errorCode;
      }
      return;
    }
  }
  // 推理
  if (use_infer) {
    errorCode = mInference->predict(mContext, objectMetadatas);
    if (common::ErrorCode::SUCCESS != errorCode) {
      for (unsigned i = 0; i < objectMetadatas.size(); i++) {
        objectMetadatas[i]->mErrorCode = errorCode;
      }
      return;
    }
  }
  // 后处理
  if (use_post) mPostProcess->postProcess(mContext, objectMetadatas);
}

common::ErrorCode Posec3d::doWork(int dataPipeId) {
  common::ErrorCode errorCode = common::ErrorCode::SUCCESS;

  common::ObjectMetadatas objectMetadatas;
  std::vector<int> inputPorts = getInputPorts();
  int inputPort = inputPorts[0];
  int outputPort = 0;
  if (!getSinkElementFlag()) {
    std::vector<int> outputPorts = getOutputPorts();
    outputPort = outputPorts[0];
  }

  common::ObjectMetadatas pendingObjectMetadatas;

  if (use_pre) {
    while (objectMetadatas.size() < mContext->max_batch &&
           (getThreadStatus() == ThreadStatus::RUN)) {
      // 如果队列为空则等待
      auto data = popInputData(inputPort, dataPipeId);

      if (!data) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        continue;
      }
      auto objectMetadata =
          std::static_pointer_cast<common::ObjectMetadata>(data);
      pendingObjectMetadatas.push_back(objectMetadata);
      if (objectMetadata->mFrame->mEndOfStream) {
        break;
      }
      if (!objectMetadata->mFilter) objectMetadatas.push_back(objectMetadata);
    }
  } else {
    while (pendingObjectMetadatas.size() < mContext->max_batch &&
           (getThreadStatus() == ThreadStatus::RUN)) {
      // 如果队列为空则等待
      auto data = popInputData(inputPort, dataPipeId);

      if (!data) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        continue;
      }
      auto objectMetadata =
          std::static_pointer_cast<common::ObjectMetadata>(data);
      pendingObjectMetadatas.push_back(objectMetadata);
      if (objectMetadata->mFrame->mEndOfStream) {
        break;
      }
      // all frame inputs are put into main objectMetadata, then the following
      // infer and postprocess are based on main objectMetadata
      if (!objectMetadata->mFilter && objectMetadata->is_main)
        objectMetadatas.push_back(objectMetadata);
    }
  }

  process(objectMetadatas);

  if (use_post && objectMetadatas.size() > 0) {
    // for all frames, they have the same action label
    for (auto& objectMetadata : pendingObjectMetadatas)
      objectMetadata->mRecognizedObjectMetadatas = objectMetadatas[0]->mRecognizedObjectMetadatas;
  }

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

void Posec3d::setStage(bool pre, bool infer, bool post) {
  use_pre = pre;
  use_infer = infer;
  use_post = post;
}

void Posec3d::initProfiler(std::string name, int interval) {
  mFpsProfiler.config(name, 100);
}

void Posec3d::setContext(
    std::shared_ptr<::sophon_stream::element::Context> context) {
  // check
  mContext = std::dynamic_pointer_cast<Posec3dContext>(context);
}

void Posec3d::setPreprocess(
    std::shared_ptr<::sophon_stream::element::PreProcess> pre) {
  mPreProcess = std::dynamic_pointer_cast<Posec3dPreProcess>(pre);
}

void Posec3d::setInference(
    std::shared_ptr<::sophon_stream::element::Inference> infer) {
  mInference = std::dynamic_pointer_cast<Posec3dInference>(infer);
}

void Posec3d::setPostprocess(
    std::shared_ptr<::sophon_stream::element::PostProcess> post) {
  mPostProcess = std::dynamic_pointer_cast<Posec3dPostProcess>(post);
}

REGISTER_WORKER("posec3d", Posec3d)
REGISTER_GROUP_WORKER("posec3d_group",
                         sophon_stream::framework::Group<Posec3d>, Posec3d)

}  // namespace posec3d
}  // namespace element
}  // namespace sophon_stream
