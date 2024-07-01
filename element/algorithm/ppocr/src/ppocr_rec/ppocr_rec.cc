//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#include "ppocr_rec.h"

using namespace std::chrono_literals;

namespace sophon_stream {
namespace element {
namespace ppocr_rec {

PpocrRec::PpocrRec() {}

PpocrRec::~PpocrRec() {}

const std::string PpocrRec::elementName = "ppocr_rec";

common::ErrorCode PpocrRec::initContext(const std::string& json) {
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
    mContext->m_net_channel = inputTensor->get_shape()->dims[1];
    mContext->net_h = inputTensor->get_shape()->dims[2];
    mContext->net_w = inputTensor->get_shape()->dims[3];
    mContext->input_num = mContext->bmNetwork->m_netinfo->input_num;
    auto classNamesFileIt =
        configure.find(CONFIG_INTERNAL_CLASS_NAMES_FILE_FIELD);
    std::string class_names_file = classNamesFileIt->get<std::string>();
    std::ifstream istream;
    istream.open(class_names_file);
    assert(istream.is_open());
    std::string line;
    while (std::getline(istream, line)) {
      line = line.substr(0, line.length());
      mContext->label_list_.push_back(line);
    }
    istream.close();
    mContext->label_list_.insert(mContext->label_list_.begin(),
                                 "#");  // blank char for ctc
    mContext->label_list_.push_back(" ");
    auto beamSearchIt = configure.find(CONFIG_INTERNAL_BEAM_SEARCH_FIELD);
    mContext->beam_search = beamSearchIt->get<bool>();
    auto beamWidthIt = configure.find(CONFIG_INTERNAL_BEAM_WIDTH_FIELD);
    mContext->beam_width = beamWidthIt->get<int>();
    STREAM_CHECK(mContext->beam_width >= 1 && mContext->beam_width <= 40,
                 "beam_size out of range, should be integer in range(1, 41)");
    int pre_net_h = -1;
    for (int i = 0; i < mContext->bmNetwork->m_netinfo->stage_num; i++) {
      auto tensor = mContext->bmNetwork->inputTensor(0);
      int net_h_ = tensor->get_shape()->dims[2];
      if (pre_net_h == -1) {
        pre_net_h = net_h_;
      } else {
        STREAM_CHECK(
            pre_net_h == net_h_,
            "Invalid model size! All Stage's height must be identical.");
      }

      int net_w_ = tensor->get_shape()->dims[3];
      bool skip_flag = false;
      for (auto& tmp_size : mContext->img_size) {
        if (tmp_size.w == net_w_) {
          skip_flag = true;
          break;
        }
      }
      if (skip_flag == true) {
        continue;
      }
      mContext->img_size.push_back({net_w_, net_h_});
    }
    std::sort(
        mContext->img_size.begin(), mContext->img_size.end(),
        [](const RecModelSize& a, const RecModelSize& b) { return a.w < b.w; });
    for (auto& s : mContext->img_size) {
      mContext->img_ratio.push_back((float)s.w / (float)s.h);
    }

    // 3. get output
    mContext->output_num = mContext->bmNetwork->outputTensorNum();

    // 4.converto
    float input_scale = inputTensor->get_scale();
    input_scale = input_scale * 0.0078125;
    mContext->converto_attr.alpha_0 = input_scale;
    mContext->converto_attr.beta_0 = -127.5 * 0.0078125;
    mContext->converto_attr.alpha_1 = input_scale;
    mContext->converto_attr.beta_1 = -127.5 * 0.0078125;
    mContext->converto_attr.alpha_2 = input_scale;
    mContext->converto_attr.beta_2 = -127.5 * 0.0078125;

  } while (false);
  return errorCode;
}

common::ErrorCode PpocrRec::initInternal(const std::string& json) {
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
        mFpsProfilerName = "fps_ppocr_rec_pre";
      }
      if (std::find(stages.begin(), stages.end(), "infer") != stages.end()) {
        use_infer = true;
        mFpsProfilerName = "fps_ppocr_rec_infer";
      }
      if (std::find(stages.begin(), stages.end(), "post") != stages.end()) {
        use_post = true;
        mFpsProfilerName = "fps_ppocr_rec_post";
      }

      mFpsProfiler.config(mFpsProfilerName, 100);
    }
    // 新建context,预处理,推理和后处理对象
    mContext = std::make_shared<PpocrRecContext>();
    mPreProcess = std::make_shared<PpocrRecPreProcess>();
    mInference = std::make_shared<PpocrRecInference>();
    mPostProcess = std::make_shared<PpocrRecPostProcess>();

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

void PpocrRec::process(common::ObjectMetadatas& objectMetadatas) {
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

common::ErrorCode PpocrRec::doWork(int dataPipeId) {
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

void PpocrRec::setStage(bool pre, bool infer, bool post) {
  use_pre = pre;
  use_infer = infer;
  use_post = post;
}

void PpocrRec::initProfiler(std::string name, int interval) {
  mFpsProfiler.config(mFpsProfilerName, 100);
}

void PpocrRec::setContext(
    std::shared_ptr<::sophon_stream::element::Context> context) {
  // check
  mContext = std::dynamic_pointer_cast<PpocrRecContext>(context);
}

void PpocrRec::setPreprocess(
    std::shared_ptr<::sophon_stream::element::PreProcess> pre) {
  mPreProcess = std::dynamic_pointer_cast<PpocrRecPreProcess>(pre);
}

void PpocrRec::setInference(
    std::shared_ptr<::sophon_stream::element::Inference> infer) {
  mInference = std::dynamic_pointer_cast<PpocrRecInference>(infer);
}

void PpocrRec::setPostprocess(
    std::shared_ptr<::sophon_stream::element::PostProcess> post) {
  mPostProcess = std::dynamic_pointer_cast<PpocrRecPostProcess>(post);
}

REGISTER_WORKER("ppocr_rec", PpocrRec)
REGISTER_GROUP_WORKER("ppocr_rec_group",
                         sophon_stream::framework::Group<PpocrRec>, PpocrRec)
}  // namespace ppocr_rec
}  // namespace element
}  // namespace sophon_stream
