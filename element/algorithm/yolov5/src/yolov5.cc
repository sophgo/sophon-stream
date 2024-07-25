//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#include "yolov5.h"

using namespace std::chrono_literals;

namespace sophon_stream {
namespace element {
namespace yolov5 {

Yolov5::Yolov5() {}

Yolov5::~Yolov5() {}

const std::string Yolov5::elementName = "yolov5";

common::ErrorCode Yolov5::initContext(const std::string& json) {
  common::ErrorCode errorCode = common::ErrorCode::SUCCESS;
  do {
    auto configure = nlohmann::json::parse(json, nullptr, false);
    if (!configure.is_object()) {
      errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
      break;
    }

    auto modelPathIt = configure.find(CONFIG_INTERNAL_MODEL_PATH_FIELD);

    auto threshConfIt = configure.find(CONFIG_INTERNAL_THRESHOLD_CONF_FIELD);
    if (threshConfIt->is_number_float()) {
      mContext->thresh_conf_min = threshConfIt->get<float>();
    } else {
      mContext->thresh_conf =
          threshConfIt->get<std::unordered_map<std::string, float>>();
    }

    if (threshConfIt->is_number_float()) {
      mContext->class_thresh_valid = false;
    } else {
      auto classNamesFileIt =
          configure.find(CONFIG_INTERNAL_CLASS_NAMES_FILE_FIELD);
      if (classNamesFileIt->is_string()) {
        mContext->class_thresh_valid = true;
        std::string class_names_file = classNamesFileIt->get<std::string>();
        std::ifstream istream;
        istream.open(class_names_file);
        assert(istream.is_open());
        std::string line;
        while (std::getline(istream, line)) {
          line = line.substr(0, line.length());
          mContext->class_names.push_back(line);
          // if (mContext->thresh_conf_min != -1) {
          //   mContext->thresh_conf.insert({line, mContext->thresh_conf_min});
          // }
        }
        istream.close();
      }
    }

    for (auto thresh_it = mContext->thresh_conf.begin();
         thresh_it != mContext->thresh_conf.end(); ++thresh_it) {
      mContext->thresh_conf_min = mContext->thresh_conf_min < thresh_it->second
                                      ? mContext->thresh_conf_min
                                      : thresh_it->second;
    }
    mContext->log_conf_threshold = -std::log(1 / mContext->thresh_conf_min - 1);

    auto threshNmsIt = configure.find(CONFIG_INTERNAL_THRESHOLD_NMS_FIELD);
    mContext->thresh_nms = threshNmsIt->get<float>();

    mContext->bgr2rgb = true;
    auto bgr2rgbIt = configure.find(CONFIG_INTERNAL_THRESHOLD_BGR2RGB_FIELD);
    mContext->bgr2rgb = bgr2rgbIt->get<bool>();

    auto meanIt = configure.find(CONFIG_INTERNAL_THRESHOLD_MEAN_FIELD);
    mContext->mean = meanIt->get<std::vector<float>>();
    assert(mContext->mean.size() == 3);

    auto stdIt = configure.find(CONFIG_INTERNAL_THRESHOLD_STD_FIELD);
    mContext->stdd = stdIt->get<std::vector<float>>();
    assert(mContext->stdd.size() == 3);

    auto tpu_kernelIt =
        configure.find(CONFIG_INTERNAL_THRESHOLD_TPU_KERNEL_FIELD);
    if (configure.end() == tpu_kernelIt || !tpu_kernelIt->is_boolean()) {
      errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
      break;
    }
    mContext->use_tpu_kernel = tpu_kernelIt->get<bool>();

    auto max_detIt = configure.find(CONFIG_INTERNAL_MAX_DET_FILED);
    auto min_detIt = configure.find(CONFIG_INTERNAL_MIN_DET_FILED);
    if (configure.end() != max_detIt) {
      mContext->m_max_det = max_detIt->get<unsigned int>();
    }
    if (configure.end() != min_detIt) {
      mContext->m_min_det = min_detIt->get<unsigned int>();
    }

    // 1. get network
    BMNNHandlePtr handle = std::make_shared<BMNNHandle>(mContext->deviceId);

    // use_tpu_kernel could only be enable on 1684x
    // check it before load model
    unsigned int chip_id_;
    bm_get_chipid(handle->handle(), &chip_id_);
    STREAM_CHECK((mContext->use_tpu_kernel && (chip_id_ == 0x1686)) ||
                     (!mContext->use_tpu_kernel),
                 "TPU KERNEL could only be enabled on 1684X, please check your "
                 "Json files");

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
    mContext->min_dim =
        mContext->bmNetwork->outputTensor(0)->get_shape()->num_dims;
    if (mContext->output_num == 3) {
      if (mContext->use_tpu_kernel)
        mContext->class_num =
            mContext->bmNetwork->outputTensor(0)->get_shape()->dims[1] / 3 - 4 -
            1;  // class_nums + box_4 + conf_1
      else
        mContext->class_num =
            mContext->bmNetwork->outputTensor(0)->get_shape()->dims[4] - 4 - 1;
    } else {
      mContext->class_num =
          mContext->bmNetwork->outputTensor(0)->get_shape()->dims[2] - 5;
    }

    if (mContext->class_thresh_valid) {
      if (mContext->class_num != mContext->class_names.size() ||
          mContext->class_num != mContext->thresh_conf.size() ||
          mContext->thresh_conf.size() != mContext->class_names.size()) {
        IVS_CRITICAL(
            "Class Number Does Not Match The Model! Please Check The Json "
            "File.");
        abort();
      }
    }

    // 4.converto
    float input_scale = inputTensor->get_scale();
    // input_scale = input_scale * 1.0 / 255.f;
    mContext->converto_attr.alpha_0 = input_scale / (mContext->stdd[0]);
    mContext->converto_attr.beta_0 = -(mContext->mean[0]) / (mContext->stdd[0]);
    mContext->converto_attr.alpha_1 = input_scale / (mContext->stdd[1]);
    mContext->converto_attr.beta_1 = -(mContext->mean[1]) / (mContext->stdd[1]);
    mContext->converto_attr.alpha_2 = input_scale / (mContext->stdd[2]);
    mContext->converto_attr.beta_2 = -(mContext->mean[2]) / (mContext->stdd[2]);

    // 6. tpu_kernel postprocess
    if (mContext->use_tpu_kernel) {
      tpu_kernel_module_t tpu_module;
      std::string tpu_kernel_module_path =
          "../../3rdparty/tpu_kernel_module/"
          "libbm1684x_kernel_module.so";
      std::ifstream file(tpu_kernel_module_path);
      STREAM_CHECK(file.good(),
                   "kernel_module.so does not exist, please check your path: ",
                   tpu_kernel_module_path);
      file.close();
      tpu_module = tpu_kernel_load_module_file(mContext->bmContext->handle(),
                                               tpu_kernel_module_path.c_str());
      mContext->func_id =
          tpu_kernel_get_function(mContext->bmContext->handle(), tpu_module,
                                  "tpu_kernel_api_yolov5_detect_out");
      std::cout << "Using tpu_kernel yolo postprocession, kernel funtion id: "
                << mContext->func_id << std::endl;
    }

    // 7. roi
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
    mContext->thread_number = getThreadNumber();
  } while (false);
  return common::ErrorCode::SUCCESS;
}

common::ErrorCode Yolov5::initInternal(const std::string& json) {
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
        mFpsProfilerName = "fps_yolov5_pre";
      }
      if (std::find(stages.begin(), stages.end(), "infer") != stages.end()) {
        use_infer = true;
        mFpsProfilerName = "fps_yolov5_infer";
      }
      if (std::find(stages.begin(), stages.end(), "post") != stages.end()) {
        use_post = true;
        mFpsProfilerName = "fps_yolov5_post";
      }

      mFpsProfiler.config(mFpsProfilerName, 100);
    }

    // 新建context,预处理,推理和后处理对象
    mContext = std::make_shared<Yolov5Context>();
    mPreProcess = std::make_shared<Yolov5PreProcess>();
    mInference = std::make_shared<Yolov5Inference>();
    mPostProcess = std::make_shared<Yolov5PostProcess>();

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

void Yolov5::process(common::ObjectMetadatas& objectMetadatas, int dataPipeId) {
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
  if (use_post)
    mPostProcess->postProcess(mContext, objectMetadatas, dataPipeId);
}

common::ErrorCode Yolov5::doWork(int dataPipeId) {
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

  process(objectMetadatas, dataPipeId);

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

void Yolov5::setStage(bool pre, bool infer, bool post) {
  use_pre = pre;
  use_infer = infer;
  use_post = post;
}

void Yolov5::initProfiler(std::string name, int interval) {
  mFpsProfiler.config(name, 100);
}

void Yolov5::setContext(
    std::shared_ptr<::sophon_stream::element::Context> context) {
  // check
  mContext = std::dynamic_pointer_cast<Yolov5Context>(context);
}

void Yolov5::setPreprocess(
    std::shared_ptr<::sophon_stream::element::PreProcess> pre) {
  mPreProcess = std::dynamic_pointer_cast<Yolov5PreProcess>(pre);
}

void Yolov5::setInference(
    std::shared_ptr<::sophon_stream::element::Inference> infer) {
  mInference = std::dynamic_pointer_cast<Yolov5Inference>(infer);
}

void Yolov5::setPostprocess(
    std::shared_ptr<::sophon_stream::element::PostProcess> post) {
  mPostProcess = std::dynamic_pointer_cast<Yolov5PostProcess>(post);
}

void Yolov5::registListenFunc(
    sophon_stream::framework::ListenThread* listener) {
  std::string mIdStr = std::to_string(getId());
  std::string handlerName = postNameSetConfThreshold + "/" + mIdStr;
  listener->setHandler(handlerName.c_str(),
                       sophon_stream::framework::RequestType::POST,
                       std::bind(&Yolov5::listenerSetConfThreshold, this,
                                 std::placeholders::_1, std::placeholders::_2));
}

void Yolov5::listenerSetConfThreshold(const httplib::Request& request,
                                      httplib::Response& response) {
  common::Response resp;
  common::RequestSingleFloat rsi;
  common::str_to_object(request.body, rsi);
  mContext->thresh_conf_min = rsi.value;
  resp.code = 0;
  resp.msg = "success";
  nlohmann::json json_res = resp;
  response.set_content(json_res.dump(), "application/json");
  return;
}

REGISTER_WORKER("yolov5", Yolov5)
REGISTER_GROUP_WORKER("yolov5_group", sophon_stream::framework::Group<Yolov5>,
                      Yolov5)

}  // namespace yolov5
}  // namespace element
}  // namespace sophon_stream
