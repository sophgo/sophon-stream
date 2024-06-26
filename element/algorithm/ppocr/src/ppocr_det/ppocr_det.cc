//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#include "ppocr_det.h"

using namespace std::chrono_literals;

namespace sophon_stream {
namespace element {
namespace ppocr_det {

  Ppocr_det::Ppocr_det() {}

  Ppocr_det::~Ppocr_det() {}

  const std::string Ppocr_det::elementName = "ppocr_det";

  common::ErrorCode Ppocr_det::initContext(const std::string& json) {
    common::ErrorCode errorCode = common::ErrorCode::SUCCESS;
    do {
      auto configure = nlohmann::json::parse(json, nullptr, false);
      if (!configure.is_object()) {
        errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
        break;
      }

      auto modelPathIt = configure.find(CONFIG_INTERNAL_MODEL_PATH_FIELD);

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

    // 4.converto
    float input_scale = inputTensor->get_scale();
    // input_scale = input_scale * 1.0 / 255.0;
    mContext->converto_attr.alpha_0 = input_scale / (mContext->stdd[0]);
    mContext->converto_attr.beta_0 = -(mContext->mean[0]) / (mContext->stdd[0]);
    mContext->converto_attr.alpha_1 = input_scale / (mContext->stdd[1]);
    mContext->converto_attr.beta_1 = -(mContext->mean[1]) / (mContext->stdd[1]);
    mContext->converto_attr.alpha_2 = input_scale / (mContext->stdd[2]);
    mContext->converto_attr.beta_2 = -(mContext->mean[2]) / (mContext->stdd[2]);

    } while (false);
    return common::ErrorCode::SUCCESS;
  }

  common::ErrorCode Ppocr_det::initInternal(const std::string& json) {
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
          mFpsProfilerName = "fps_ppocr_det_pre";
        }
        if (std::find(stages.begin(), stages.end(), "infer") != stages.end()) {
          use_infer = true;
          mFpsProfilerName = "fps_ppocr_det_infer";
        }
        if (std::find(stages.begin(), stages.end(), "post") != stages.end()) {
          use_post = true;
          mFpsProfilerName = "fps_ppocr_det_post";
        }

        mFpsProfiler.config(mFpsProfilerName, 100);
      }
      // 新建context,预处理,推理和后处理对象
      mContext = std::make_shared<Ppocr_detContext>();
      mPreProcess = std::make_shared<Ppocr_detPreProcess>();
      mInference = std::make_shared<Ppocr_detInference>();
      mPostProcess = std::make_shared<Ppocr_detPostProcess>();

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

  void Ppocr_det::process(common::ObjectMetadatas & objectMetadatas) {
    common::ErrorCode errorCode = common::ErrorCode::SUCCESS;
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

  common::ErrorCode Ppocr_det::doWork(int dataPipeId) {
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
      errorCode =
          pushOutputData(outputPort, outDataPipeId,
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

  void Ppocr_det::setStage(bool pre, bool infer, bool post) {
    use_pre = pre;
    use_infer = infer;
    use_post = post;
  }

  void Ppocr_det::initProfiler(std::string name, int interval) {
    mFpsProfiler.config(mFpsProfilerName, 100);
  }

  void Ppocr_det::setContext(
      std::shared_ptr<::sophon_stream::element::Context> context) {
    // check
    mContext = std::dynamic_pointer_cast<Ppocr_detContext>(context);
  }

  void Ppocr_det::setPreprocess(
      std::shared_ptr<::sophon_stream::element::PreProcess> pre) {
    mPreProcess = std::dynamic_pointer_cast<Ppocr_detPreProcess>(pre);
  }

  void Ppocr_det::setInference(
      std::shared_ptr<::sophon_stream::element::Inference> infer) {
    mInference = std::dynamic_pointer_cast<Ppocr_detInference>(infer);
  }

  void Ppocr_det::setPostprocess(
      std::shared_ptr<::sophon_stream::element::PostProcess> post) {
    mPostProcess = std::dynamic_pointer_cast<Ppocr_detPostProcess>(post);
  }

  REGISTER_WORKER("ppocr_det", Ppocr_det)
  REGISTER_GROUP_WORKER("ppocr_det_group",
                           sophon_stream::framework::Group<Ppocr_det>, Ppocr_det)
}  // namespace ppocr_det
}  // namespace element
}  // namespace sophon_stream
