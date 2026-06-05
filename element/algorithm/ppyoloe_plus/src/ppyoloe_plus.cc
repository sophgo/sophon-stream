//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#include "ppyoloe_plus.h"

using namespace std::chrono_literals;

namespace sophon_stream {
namespace element {
namespace ppyoloe_plus {

  Ppyoloe_plus::Ppyoloe_plus() {}

  Ppyoloe_plus::~Ppyoloe_plus() {}

  const std::string Ppyoloe_plus::elementName = "ppyoloe_plus";

  common::ErrorCode Ppyoloe_plus::initContext(const std::string& json) {
    common::ErrorCode errorCode = common::ErrorCode::SUCCESS;
    do {
      auto configure = nlohmann::json::parse(json, nullptr, false);
      if (!configure.is_object()) {
        errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
        break;
      }
      
      // class_name
      auto class_names_file = configure.find(CONFIG_INTERNAL_CLASS_NAMES_FILE_FILED)->get<std::string>();
      std::ifstream istream;
      istream.open(class_names_file);
      assert(istream.is_open());
      std::string line;
      while (std::getline(istream, line)) {
        line = line.substr(0, line.length());
        mContext->class_names.push_back(line);
      }
      istream.close();

      // 置信度阈值
      auto threshConfIt = configure.find(CONFIG_INTERNAL_THRESHOLD_CONF_FIELD);
      if (threshConfIt->is_number_float()) {
        mContext->thresh_conf_min = threshConfIt->get<float>();
      }
      // MNS阈值
      auto threshNmsIt = configure.find(CONFIG_INTERNAL_THRESHOLD_NMS_FIELD);
      mContext->thresh_nms = threshNmsIt->get<float>();
      // bgr转rgb
      mContext->bgr2rgb = true;
      auto bgr2rgbIt = configure.find(CONFIG_INTERNAL_THRESHOLD_BGR2RGB_FIELD);
      mContext->bgr2rgb = bgr2rgbIt->get<bool>();
      // mean
      auto meanIt = configure.find(CONFIG_INTERNAL_THRESHOLD_MEAN_FIELD);
      mContext->mean = meanIt->get<std::vector<float>>();
      assert(mContext->mean.size() == 3);
      // strand
      auto stdIt = configure.find(CONFIG_INTERNAL_THRESHOLD_STD_FIELD);
      mContext->stdd = stdIt->get<std::vector<float>>();
      assert(mContext->stdd.size() == 3);

      // 1. get network
      auto modelPathIt = configure.find(CONFIG_INTERNAL_MODEL_PATH_FIELD);
      BMNNHandlePtr handle = std::make_shared<BMNNHandle>(mContext->deviceId);
      mContext->bmContext = std::make_shared<BMNNContext>(handle, modelPathIt->get<std::string>().c_str());
      mContext->bmNetwork = mContext->bmContext->network(0);
      mContext->handle = handle->handle();

      // 2. get input（参考sophon-demo中ppyoloe C++的实现）
      mContext->max_batch = mContext->bmNetwork->maxBatch();
      auto input_img = mContext->bmNetwork->inputTensor(0);  // 图片数据
      std::shared_ptr<BMNNTensor> input_ratio = mContext->bmNetwork->inputTensor(1);  // 缩放比例
      mContext->input_num = mContext->bmNetwork->m_netinfo->input_num;  // 2
      mContext->m_net_channel = input_img->get_shape()->dims[1];
      mContext->net_h = input_img->get_shape()->dims[2];
      mContext->net_w = input_img->get_shape()->dims[3];

      // 3. get output
      mContext->output_num = mContext->bmNetwork->outputTensorNum();
      mContext->min_dim = mContext->bmNetwork->outputTensor(0)->get_shape()->num_dims;

      // 4.converto
      float input_scale = input_ratio->get_scale();
      // 归一化 [0-255] -> [0-1]  归一化：y = (1/255) * x + 0  标准化：z = (1/std) * y + (-mean/std)
      // z = (1/std) * [(1/255) * x + 0] + (-mean/std) = (1/(255 * std)) * x + (-mean/std)
      mContext->converto_attr.alpha_0 = input_scale/(255.f*mContext->stdd[0]);
      mContext->converto_attr.beta_0 = -(mContext->mean[0]) / (mContext->stdd[0]);
      mContext->converto_attr.alpha_1 = input_scale/(255.f*mContext->stdd[1]);
      mContext->converto_attr.beta_1 = -(mContext->mean[1]) / (mContext->stdd[1]);
      mContext->converto_attr.alpha_2 = input_scale/(255.f*mContext->stdd[2]);
      mContext->converto_attr.beta_2 = -(mContext->mean[2]) / (mContext->stdd[2]);

    } while (false);
    return common::ErrorCode::SUCCESS;
  }

  common::ErrorCode Ppyoloe_plus::initInternal(const std::string& json) {
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
          mFpsProfilerName = "fps_ppyoloe_plus_pre";
        }
        if (std::find(stages.begin(), stages.end(), "infer") != stages.end()) {
          use_infer = true;
          mFpsProfilerName = "fps_ppyoloe_plus_infer";
        }
        if (std::find(stages.begin(), stages.end(), "post") != stages.end()) {
          use_post = true;
          mFpsProfilerName = "fps_ppyoloe_plus_post";
        }

        mFpsProfiler.config(mFpsProfilerName, 100);
      }
      // 新建context,预处理,推理和后处理对象
      mContext = std::make_shared<Ppyoloe_plusContext>();
      mPreProcess = std::make_shared<Ppyoloe_plusPreProcess>();
      mInference = std::make_shared<Ppyoloe_plusInference>();
      mPostProcess = std::make_shared<Ppyoloe_plusPostProcess>();

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

  void Ppyoloe_plus::process(common::ObjectMetadatas & objectMetadatas) {
    common::ErrorCode errorCode = common::ErrorCode::SUCCESS;
    if (use_pre) {
      IVS_INFO("preprocess");
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
      IVS_INFO("infer");
      errorCode = mInference->predict(mContext, objectMetadatas);
      if (common::ErrorCode::SUCCESS != errorCode) {
        for (unsigned i = 0; i < objectMetadatas.size(); i++) {
          objectMetadatas[i]->mErrorCode = errorCode;
        }
        return;
      }
    }
    // 后处理
    if (use_post) {
      IVS_INFO("postProcess");
      mPostProcess->postProcess(mContext, objectMetadatas);
    } 
    
  }

  common::ErrorCode Ppyoloe_plus::doWork(int dataPipeId) {
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

  void Ppyoloe_plus::setStage(bool pre, bool infer, bool post) {
    use_pre = pre;
    use_infer = infer;
    use_post = post;
  }

  void Ppyoloe_plus::initProfiler(std::string name, int interval) {
    mFpsProfiler.config(name, 100);
  }

  void Ppyoloe_plus::setContext(
      std::shared_ptr<::sophon_stream::element::Context> context) {
    // check
    mContext = std::dynamic_pointer_cast<Ppyoloe_plusContext>(context);
  }

  void Ppyoloe_plus::setPreprocess(
      std::shared_ptr<::sophon_stream::element::PreProcess> pre) {
    mPreProcess = std::dynamic_pointer_cast<Ppyoloe_plusPreProcess>(pre);
  }

  void Ppyoloe_plus::setInference(
      std::shared_ptr<::sophon_stream::element::Inference> infer) {
    mInference = std::dynamic_pointer_cast<Ppyoloe_plusInference>(infer);
  }

  void Ppyoloe_plus::setPostprocess(
      std::shared_ptr<::sophon_stream::element::PostProcess> post) {
    mPostProcess = std::dynamic_pointer_cast<Ppyoloe_plusPostProcess>(post);
  }

  REGISTER_WORKER("ppyoloe_plus", Ppyoloe_plus)
  REGISTER_GROUP_WORKER("ppyoloe_plus_group",
                        sophon_stream::framework::Group<Ppyoloe_plus>, Ppyoloe_plus)
}  // namespace ppyoloe_plus
}  // namespace element
}  // namespace sophon_stream
