//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#include "yolov8.h"

using namespace std::chrono_literals;

namespace sophon_stream {
namespace element {
namespace yolov8 {

Yolov8::Yolov8() {}

Yolov8::~Yolov8() {
  if ((mContext->taskType == TaskType::Seg) && (mContext->seg_tpu_opt)) {
    if (mContext->bmrt != nullptr) {
      bmrt_destroy(mContext->bmrt);
      mContext->bmrt = nullptr;
      bm_dev_free(mContext->tpu_mask_handle);
    }
  }
}

const std::string Yolov8::elementName = "yolov8";

std::unordered_map<std::string, TaskType> taskMap{{"Detect", TaskType::Detect},
                                                  {"Pose", TaskType::Pose},
                                                  {"Cls", TaskType::Cls},
                                                  {"Seg", TaskType::Seg}};

common::ErrorCode Yolov8::initContext(const std::string& json) {
  common::ErrorCode errorCode = common::ErrorCode::SUCCESS;
  do {
    auto configure = nlohmann::json::parse(json, nullptr, false);
    if (!configure.is_object()) {
      errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
      break;
    }

    auto modelPathIt = configure.find(CONFIG_INTERNAL_MODEL_PATH_FIELD);

    auto task_it = configure.find(CONFIG_INTERNAL_TASK_TYPE_FILED);
    if (task_it != configure.end()) {
      std::string taskName = task_it->get<std::string>();
      STREAM_CHECK(taskMap.count(taskName) != 0,
                   "Invalid Task Type in Yolov8 Config File!");
      mContext->taskType = taskMap[taskName];
    }

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
          if (mContext->thresh_conf_min != -1) {
            mContext->thresh_conf.insert({line, mContext->thresh_conf_min});
          }
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

    // yolov8_seg_tpu_opt
    auto segTpuOptIt = configure.find(CONFIG_INTERNAL_SEG_TPU_OPT_FILED);
    if (segTpuOptIt != configure.end()) {
      mContext->seg_tpu_opt = segTpuOptIt->get<bool>();

      if (mContext->seg_tpu_opt) {
        auto maskBmodelPath = configure.find(CONFIG_INTERNAL_MASK_BMODEL_PATH);

        if (maskBmodelPath != configure.end()) {
          mContext->mask_bmodel_path =
              maskBmodelPath->get<std::string>().c_str();
          // 1. get handle
          STREAM_CHECK(BM_SUCCESS == bm_dev_request(&mContext->tpu_mask_handle,
                                                    mContext->deviceId),
                       "bm_dev_request error");
          // 2. create bmrt and load bmodel
          mContext->bmrt = bmrt_create(mContext->tpu_mask_handle);
          STREAM_CHECK(
              true == bmrt_load_bmodel(mContext->bmrt,
                                       mContext->mask_bmodel_path.c_str()),
              "bmrt_load_bmodel error, please check the path of "
              "mask_bmodel_path!");
          // 3. get network names from bmodel
          const char** names = nullptr;
          int num = bmrt_get_network_number(mContext->bmrt);
          if (num > 1) {
            IVS_WARN(
                "The tpu mask bmodel have {0:d} networks, and this program "
                "will "
                "only take network 0.",
                num);
          }
          bmrt_get_network_names(mContext->bmrt, &names);
          for (int i = 0; i < num; ++i) {
            mContext->network_names.emplace_back(names[i]);
          }
          free(names);
          // 4. get netinfo by netname
          mContext->netinfo = bmrt_get_network_info(
              mContext->bmrt, mContext->network_names[0].c_str());
          if (mContext->netinfo->stage_num > 1) {
            IVS_WARN(
                "The tpu mask bmodel have {0:d} stages, and this program will "
                "only "
                "take stage 0.",
                mContext->netinfo->stage_num);
          }
          // 5. initialize parameters.
          mContext->m_tpumask_net_h =
              mContext->netinfo->stages[0].input_shapes[1].dims[2];
          mContext->m_tpumask_net_w =
              mContext->netinfo->stages[0].input_shapes[1].dims[3];
          mContext->tpu_mask_num =
              mContext->netinfo->stages[0].input_shapes[0].dims[1];  // 32
          mContext->mask_len =
              mContext->netinfo->stages[0].input_shapes[1].dims[1];  // 32
        } else {
          STREAM_CHECK(false, "The path to mask_bmodel_path is not specified");
        }
      }
    }

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
    mContext->min_dim =
        mContext->bmNetwork->outputTensor(0)->get_shape()->num_dims;
    if (mContext->output_num == 3) {
      // 暂未提供三输出模型，这里暂且留待扩展
      mContext->class_num =
          mContext->bmNetwork->outputTensor(0)->get_shape()->dims[4] - 4 - 1;
    } else {
      if (mContext->taskType == TaskType::Detect) {
        int ndim1 = mContext->bmNetwork->outputTensor(0)->get_shape()->dims[1];
        int ndim2 = mContext->bmNetwork->outputTensor(0)->get_shape()->dims[2];
        if (ndim1 > ndim2) {
          mContext->use_post_opt = true;
          mContext->class_num = ndim2 - 4;
        } else {
          mContext->class_num = ndim1 - 4;
        }
      } else if (mContext->taskType == TaskType::Cls)
        mContext->class_num =
            mContext->bmNetwork->outputTensor(0)->get_shape()->dims[1];
      else if (mContext->taskType == TaskType::Pose)
        mContext->class_num = 1;
      else if (mContext->taskType == TaskType::Seg)
        mContext->class_num =
            mContext->bmNetwork->outputTensor(0)->get_shape()->dims[1] -
            mContext->mask_len - 4;
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
    mContext->converto_attr.alpha_0 = input_scale / (mContext->stdd[0]);
    mContext->converto_attr.beta_0 = -(mContext->mean[0]) / (mContext->stdd[0]);
    mContext->converto_attr.alpha_1 = input_scale / (mContext->stdd[1]);
    mContext->converto_attr.beta_1 = -(mContext->mean[1]) / (mContext->stdd[1]);
    mContext->converto_attr.alpha_2 = input_scale / (mContext->stdd[2]);
    mContext->converto_attr.beta_2 = -(mContext->mean[2]) / (mContext->stdd[2]);

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

common::ErrorCode Yolov8::initInternal(const std::string& json) {
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
        mFpsProfilerName = "fps_yolov8_pre";
      }
      if (std::find(stages.begin(), stages.end(), "infer") != stages.end()) {
        use_infer = true;
        mFpsProfilerName = "fps_yolov8_infer";
      }
      if (std::find(stages.begin(), stages.end(), "post") != stages.end()) {
        use_post = true;
        mFpsProfilerName = "fps_yolov8_post";
      }

      mFpsProfiler.config(mFpsProfilerName, 100);
    }

    // 新建context,预处理,推理和后处理对象
    mContext = std::make_shared<Yolov8Context>();
    mPreProcess = std::make_shared<Yolov8PreProcess>();
    mInference = std::make_shared<Yolov8Inference>();
    mPostProcess = std::make_shared<Yolov8PostProcess>();

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

void Yolov8::process(common::ObjectMetadatas& objectMetadatas, int dataPipeId) {
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

common::ErrorCode Yolov8::doWork(int dataPipeId) {
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

void Yolov8::setStage(bool pre, bool infer, bool post) {
  use_pre = pre;
  use_infer = infer;
  use_post = post;
}

void Yolov8::initProfiler(std::string name, int interval) {
  mFpsProfiler.config(name, 100);
}

void Yolov8::setContext(
    std::shared_ptr<::sophon_stream::element::Context> context) {
  // check
  mContext = std::dynamic_pointer_cast<Yolov8Context>(context);
}

void Yolov8::setPreprocess(
    std::shared_ptr<::sophon_stream::element::PreProcess> pre) {
  mPreProcess = std::dynamic_pointer_cast<Yolov8PreProcess>(pre);
}

void Yolov8::setInference(
    std::shared_ptr<::sophon_stream::element::Inference> infer) {
  mInference = std::dynamic_pointer_cast<Yolov8Inference>(infer);
}

void Yolov8::setPostprocess(
    std::shared_ptr<::sophon_stream::element::PostProcess> post) {
  mPostProcess = std::dynamic_pointer_cast<Yolov8PostProcess>(post);
}

REGISTER_WORKER("yolov8", Yolov8)
REGISTER_GROUP_WORKER("yolov8_group", sophon_stream::framework::Group<Yolov8>,
                      Yolov8)

}  // namespace yolov8
}  // namespace element
}  // namespace sophon_stream
