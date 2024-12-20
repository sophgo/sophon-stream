//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#include "lightstereo.h"
using namespace std::chrono_literals;

namespace sophon_stream {
namespace element {
namespace lightstereo {

  Lightstereo::Lightstereo() {}

  Lightstereo::~Lightstereo() {}

  const std::string Lightstereo::elementName = "lightstereo";

  common::ErrorCode Lightstereo::initContext(const std::string& json) {
    common::ErrorCode errorCode = common::ErrorCode::SUCCESS;
    do {
      auto configure = nlohmann::json::parse(json, nullptr, false);
      if (!configure.is_object()) {
        errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
        break;
      }

      auto modelPathIt = configure.find(CONFIG_INTERNAL_MODEL_PATH_FIELD);
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

      // 4.converto
      float input_scale_left = mContext->bmNetwork->inputTensor(0)->get_scale();
      float input_scale_right = mContext->bmNetwork->inputTensor(1)->get_scale();
      auto std = mContext->std;
      auto mean = mContext->mean;
      mContext->converto_attr_left.alpha_0 = 1 / (255. * std[0]) * input_scale_left;
      mContext->converto_attr_left.alpha_1 = 1 / (255. * std[1]) * input_scale_left;
      mContext->converto_attr_left.alpha_2 = 1 / (255. * std[2]) * input_scale_left;
      mContext->converto_attr_left.beta_0 = (-mean[0] / std[0]) * input_scale_left;
      mContext->converto_attr_left.beta_1 = (-mean[1] / std[1]) * input_scale_left;
      mContext->converto_attr_left.beta_2 = (-mean[2] / std[2]) * input_scale_left;
      mContext->converto_attr_right.alpha_0 = 1 / (255. * std[0]) * input_scale_right;
      mContext->converto_attr_right.alpha_1 = 1 / (255. * std[1]) * input_scale_right;
      mContext->converto_attr_right.alpha_2 = 1 / (255. * std[2]) * input_scale_right;
      mContext->converto_attr_right.beta_0 = (-mean[0] / std[0]) * input_scale_right;
      mContext->converto_attr_right.beta_1 = (-mean[1] / std[1]) * input_scale_right;
      mContext->converto_attr_right.beta_2 = (-mean[2] / std[2]) * input_scale_right;

    } while (false);
    return common::ErrorCode::SUCCESS;
  }

  common::ErrorCode Lightstereo::initInternal(const std::string& json) {
    common::ErrorCode errorCode = common::ErrorCode::SUCCESS;
    do {
      // json是否正确
      auto configure = nlohmann::json::parse(json, nullptr, false);
      if (!configure.is_object()) {
        errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
        break;
      }

      auto stageNameIt = configure.find(CONFIG_INTERNAL_STAGE_NAME_FIELD);
      if (configure.end() != stageNameIt && stageNameIt->is_array()) {
        std::vector<std::string> stages =
            stageNameIt->get<std::vector<std::string>>();
        if (std::find(stages.begin(), stages.end(), "pre") != stages.end()) {
          use_pre = true;
          mFpsProfilerName = "fps_lightstereo_pre";
        }
        if (std::find(stages.begin(), stages.end(), "infer") != stages.end()) {
          use_infer = true;
          mFpsProfilerName = "fps_lightstereo_infer";
        }
        if (std::find(stages.begin(), stages.end(), "post") != stages.end()) {
          use_post = true;
          mFpsProfilerName = "fps_lightstereo_post";
        }

        mFpsProfiler.config(mFpsProfilerName, 100);
      }
      // 新建context,预处理,推理和后处理对象
      mContext = std::make_shared<LightstereoContext>();
      mPreProcess = std::make_shared<LightstereoPreProcess>();
      mInference = std::make_shared<LightstereoInference>();
      mPostProcess = std::make_shared<LightstereoPostProcess>();

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

  void Lightstereo::process(common::ObjectMetadatas & leftObjectMetadatas,
                            common::ObjectMetadatas & rightObjectMetadatas,
                            common::ObjectMetadatas & outputObjectMetadatas) {
    common::ErrorCode errorCode = common::ErrorCode::SUCCESS;
    if (use_pre) {
      errorCode = mPreProcess->preProcess(mContext, leftObjectMetadatas, true);
      if (common::ErrorCode::SUCCESS != errorCode) {
        for (unsigned i = 0; i < leftObjectMetadatas.size(); i++) {
          leftObjectMetadatas[i]->mErrorCode = errorCode;
        }
        return;
      }
      errorCode = mPreProcess->preProcess(mContext, rightObjectMetadatas, false);
      if (common::ErrorCode::SUCCESS != errorCode) {
        for (unsigned i = 0; i < rightObjectMetadatas.size(); i++) {
          rightObjectMetadatas[i]->mErrorCode = errorCode;
        }
        return;
      }
    }
    // 推理
    if (use_infer) {
      errorCode = mInference->predict(mContext, leftObjectMetadatas, rightObjectMetadatas, outputObjectMetadatas);
      if (common::ErrorCode::SUCCESS != errorCode) {
        for (unsigned i = 0; i < outputObjectMetadatas.size(); i++) {
          outputObjectMetadatas[i]->mErrorCode = errorCode;
        }
        return;
      }
    }
    // 后处理
    if (use_post) mPostProcess->postProcess(mContext, outputObjectMetadatas);
  }

  common::ErrorCode Lightstereo::doWork(int dataPipeId) {
    common::ErrorCode errorCode = common::ErrorCode::SUCCESS;

    common::ObjectMetadatas leftObjectMetadatas, rightObjectMetadatas;
    std::vector<int> inputPorts = getInputPorts();
    std::vector<int> outputPorts;
    if (!getSinkElementFlag()) {
      outputPorts = getOutputPorts();
    }
    common::ObjectMetadatas outputObjectMetadatas;
    if(use_pre || use_infer) {
      while (leftObjectMetadatas.size() < mContext->max_batch &&
            (getThreadStatus() == ThreadStatus::RUN)) {
        // 如果队列为空则等待
        auto data0 = popInputData(inputPorts[0], dataPipeId);
        if (!data0) {
          std::this_thread::sleep_for(std::chrono::milliseconds(10));
          continue;
        }
        auto objectMetadata0 = std::static_pointer_cast<common::ObjectMetadata>(data0);
        if (!objectMetadata0->mFilter) {
          leftObjectMetadatas.push_back(objectMetadata0);
          if(use_infer) {
            std::shared_ptr<common::ObjectMetadata> outputObj = std::make_shared<common::ObjectMetadata>();
            outputObj->mFrame = std::make_shared<sophon_stream::common::Frame>();
            outputObj->mFrame->mWidth = objectMetadata0->mFrame->mWidth;
            outputObj->mFrame->mHeight = objectMetadata0->mFrame->mHeight;
            outputObj->mFrame->mChannelId = objectMetadata0->mFrame->mChannelId;
            outputObj->mFrame->mFrameId = objectMetadata0->mFrame->mFrameId;
            outputObj->mFrame->mChannelIdInternal = objectMetadata0->mFrame->mChannelIdInternal;
            outputObj->mFrame->mHandle = objectMetadata0->mFrame->mHandle;
            outputObj->mFrame->mEndOfStream = objectMetadata0->mFrame->mEndOfStream;
            outputObjectMetadatas.push_back(outputObj);
          }
        }
        if (objectMetadata0->mFrame->mEndOfStream) {
          break;
        }
      }

      while (rightObjectMetadatas.size() < mContext->max_batch &&
            (getThreadStatus() == ThreadStatus::RUN)) {
        // 如果队列为空则等待
        auto data1 = popInputData(inputPorts[1], dataPipeId);
        if (!data1) {
          std::this_thread::sleep_for(std::chrono::milliseconds(10));
          continue;
        }
        auto objectMetadata1 = std::static_pointer_cast<common::ObjectMetadata>(data1);
        if (!objectMetadata1->mFilter) {
          rightObjectMetadatas.push_back(objectMetadata1);
        }
        if (objectMetadata1->mFrame->mEndOfStream) {
          break;
        }
      }

    } else if(use_post) {
      while (outputObjectMetadatas.size() < mContext->max_batch &&
            (getThreadStatus() == ThreadStatus::RUN)) {
        // 如果队列为空则等待
        auto data0 = popInputData(inputPorts[0], dataPipeId);
        if (!data0) {
          std::this_thread::sleep_for(std::chrono::milliseconds(10));
          continue;
        }
        auto objectMetadata0 = std::static_pointer_cast<common::ObjectMetadata>(data0);
        if (!objectMetadata0->mFilter) {
          outputObjectMetadatas.push_back(objectMetadata0);
        }
        if (objectMetadata0->mFrame->mEndOfStream) {
          //Before the pipeline stop, the condition "finishedChannelCount == num_channels" in main.cc must be triggered.
          //But lightstereo's input num is twice as large as output num, so the variable finishedChannelCount will be only half of num_channels.
          //Here we push another eof obj to double the finishedChannelCount, so the pipeline will stop.
          outputObjectMetadatas.push_back(objectMetadata0);
          break;
        }
      }
    }

    process(leftObjectMetadatas, rightObjectMetadatas, outputObjectMetadatas);

    if(use_pre) {
      for (auto& objectMetadata : leftObjectMetadatas) {
        int channel_id_internal = objectMetadata->mFrame->mChannelIdInternal;
        int outDataPipeId =
            getSinkElementFlag()
                ? 0
                : (channel_id_internal % getOutputConnectorCapacity(outputPorts[0]));
        errorCode =
            pushOutputData(outputPorts[0], outDataPipeId,
                          std::static_pointer_cast<void>(objectMetadata));
        if (common::ErrorCode::SUCCESS != errorCode) {
          IVS_WARN(
              "Send data fail, element id: {0:d}, output port: {1:d}, data: "
              "{2:p}",
              getId(), outputPorts[0], static_cast<void*>(objectMetadata.get()));
        }
      }
      for (auto& objectMetadata : rightObjectMetadatas) {
        int channel_id_internal = objectMetadata->mFrame->mChannelIdInternal;
        int outDataPipeId =
            getSinkElementFlag()
                ? 0
                : (channel_id_internal % getOutputConnectorCapacity(outputPorts[1]));
        errorCode =
            pushOutputData(outputPorts[1], outDataPipeId,
                          std::static_pointer_cast<void>(objectMetadata));
        if (common::ErrorCode::SUCCESS != errorCode) {
          IVS_WARN(
              "Send data fail, element id: {0:d}, output port: {1:d}, data: "
              "{2:p}",
              getId(), outputPorts[1], static_cast<void*>(objectMetadata.get()));
        }
      }
    }
    else if(use_infer || use_post) {
      for (auto& objectMetadata : outputObjectMetadatas) {
        int channel_id_internal = objectMetadata->mFrame->mChannelIdInternal;
        int outDataPipeId =
            getSinkElementFlag()
                ? 0
                : (channel_id_internal % getOutputConnectorCapacity(outputPorts[0]));
        errorCode =
            pushOutputData(outputPorts[0], outDataPipeId,
                          std::static_pointer_cast<void>(objectMetadata));
        if (common::ErrorCode::SUCCESS != errorCode) {
          IVS_WARN(
              "Send data fail, element id: {0:d}, output port: {1:d}, data: "
              "{2:p}",
              getId(), outputPorts[0], static_cast<void*>(objectMetadata.get()));
        }
      }
    }

    mFpsProfiler.add(outputObjectMetadatas.size());

    return common::ErrorCode::SUCCESS;
  }

  void Lightstereo::setStage(bool pre, bool infer, bool post) {
    use_pre = pre;
    use_infer = infer;
    use_post = post;
  }

  void Lightstereo::initProfiler(std::string name, int interval) {
    mFpsProfiler.config(name, 100);
  }

  void Lightstereo::setContext(
      std::shared_ptr<::sophon_stream::element::Context> context) {
    // check
    mContext = std::dynamic_pointer_cast<LightstereoContext>(context);
  }

  void Lightstereo::setPreprocess(
      std::shared_ptr<::sophon_stream::element::PreProcess> pre) {
    mPreProcess = std::dynamic_pointer_cast<LightstereoPreProcess>(pre);
  }

  void Lightstereo::setInference(
      std::shared_ptr<::sophon_stream::element::Inference> infer) {
    mInference = std::dynamic_pointer_cast<LightstereoInference>(infer);
  }

  void Lightstereo::setPostprocess(
      std::shared_ptr<::sophon_stream::element::PostProcess> post) {
    mPostProcess = std::dynamic_pointer_cast<LightstereoPostProcess>(post);
  }

  REGISTER_WORKER("lightstereo", Lightstereo)
}  // namespace lightstereo
}  // namespace element
}  // namespace sophon_stream
