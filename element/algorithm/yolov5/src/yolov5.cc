//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#include "yolov5.h"

#include <stdlib.h>

#include <chrono>
#include <nlohmann/json.hpp>

#include "common/logger.h"
#include "element_factory.h"
using namespace std::chrono_literals;

namespace sophon_stream {
namespace element {
namespace yolov5 {

constexpr const char* CONFIG_INTERNAL_STAGE_NAME_FIELD = "stage";
constexpr const char* CONFIG_INTERNAL_MODEL_PATH_FIELD = "model_path";
constexpr const char* CONFIG_INTERNAL_THRESHOLD_CONF_FIELD = "threshold_conf";
constexpr const char* CONFIG_INTERNAL_THRESHOLD_NMS_FIELD = "threshold_nms";
constexpr const char* CONFIG_INTERNAL_THRESHOLD_TPU_KERNEL_FIELD =
    "use_tpu_kernel";

/**
 * 构造函数
 */
Yolov5::Yolov5() {}

/**
 * 析构函数
 */
Yolov5::~Yolov5() {}

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
    mContext->thresh_conf = threshConfIt->get<float>();

    auto threshNmsIt = configure.find(CONFIG_INTERNAL_THRESHOLD_NMS_FIELD);
    mContext->thresh_nms = threshNmsIt->get<float>();

    auto tpu_kernelIt =
        configure.find(CONFIG_INTERNAL_THRESHOLD_TPU_KERNEL_FIELD);
    if (configure.end() == tpu_kernelIt || !tpu_kernelIt->is_boolean()) {
      errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
      break;
    }
    mContext->use_tpu_kernel = tpu_kernelIt->get<bool>();

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
    mContext->class_num =
        mContext->bmNetwork->outputTensor(0)->get_shape()->dims[1] / 3 - 4 -
        1;  // class_nums + box_4 + conf_1

    // 4.converto
    float input_scale = inputTensor->get_scale();
    input_scale = input_scale * 1.0 / 255.f;
    mContext->converto_attr.alpha_0 = input_scale;
    mContext->converto_attr.beta_0 = 0;
    mContext->converto_attr.alpha_1 = input_scale;
    mContext->converto_attr.beta_1 = 0;
    mContext->converto_attr.alpha_2 = input_scale;
    mContext->converto_attr.beta_2 = 0;

    // 6. tpu_kernel postprocess
    if (mContext->use_tpu_kernel) {
      tpu_kernel_module_t tpu_module;
      std::string tpu_kernel_module_path =
          "../../../share/3rdparty/tpu_kernel_module/"
          "libbm1684x_kernel_module.so";
      tpu_module = tpu_kernel_load_module_file(mContext->bmContext->handle(),
                                               tpu_kernel_module_path.c_str());
      mContext->func_id =
          tpu_kernel_get_function(mContext->bmContext->handle(), tpu_module,
                                  "tpu_kernel_api_yolov5_detect_out");
      std::cout << "Using tpu_kernel yolo postprocession, kernel funtion id: "
                << mContext->func_id << std::endl;
    }
  } while (false);
  return common::ErrorCode::SUCCESS;
}

/**
 * 初始化函数
 * @param[in] side:  设备类型
 * @param[in] deviceId:  设备ID
 * @param[in] json:  初始化字符串
 * @return 错误码
 */
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
    if (configure.end() == stageNameIt || !stageNameIt->is_array()) {
      errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
      break;
    }

    std::vector<std::string> stages =
        stageNameIt->get<std::vector<std::string>>();
    if (std::find(stages.begin(), stages.end(), "pre") != stages.end())
      use_pre = true;
    if (std::find(stages.begin(), stages.end(), "infer") != stages.end())
      use_infer = true;
    if (std::find(stages.begin(), stages.end(), "post") != stages.end())
      use_post = true;

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

    mBatch = mContext->max_batch;

  } while (false);
  return errorCode;
}

/**
 * 预测函数
 * @param[in/out] objectMetadatas:  输入数据和预测结果
 */
void Yolov5::process(common::ObjectMetadatas& objectMetadatas) {
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

/**
 * 资源释放函数
 */
void Yolov5::uninitInternal() {}

common::ErrorCode Yolov5::doWork(int dataPipeId) {
  common::ErrorCode errorCode = common::ErrorCode::SUCCESS;

  common::ObjectMetadatas objectMetadatas;
  std::vector<int> inputPorts = getInputPorts();
  int inputPort = inputPorts[0];
  int outputPort = 0;
  if (!getLastElementFlag()) {
    std::vector<int> outputPorts = getOutputPorts();
    int outputPort = outputPorts[0];
  }

  while (objectMetadatas.size() < mBatch &&
         (getThreadStatus() == ThreadStatus::RUN)) {
    // 如果队列为空则等待
    auto data = getInputData(inputPort, dataPipeId);
    if (!data) {
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
      continue;
    }

    auto objectMetadata =
        std::static_pointer_cast<common::ObjectMetadata>(data);

    objectMetadatas.push_back(objectMetadata);

    if (objectMetadata->mFrame->mEndOfStream) {
      break;
    }
  }
  process(objectMetadatas);

  for (auto& objectMetadata : objectMetadatas) {
    int channel_id_internal = objectMetadata->mFrame->mChannelIdInternal;
    int outDataPipeId =
        getLastElementFlag()
            ? 0
            : (channel_id_internal % getOutputConnector(outputPort)->getDataPipeCount());
    errorCode = pushOutputData(outputPort, outDataPipeId,
                               std::static_pointer_cast<void>(objectMetadata));
    if (common::ErrorCode::SUCCESS != errorCode) {
      IVS_WARN(
          "Send data fail, element id: {0:d}, output port: {1:d}, data: "
          "{2:p}",
          getId(), outputPort, static_cast<void*>(objectMetadata.get()));
    }
  }

  return common::ErrorCode::SUCCESS;
}

REGISTER_WORKER("yolov5", Yolov5)

}  // namespace yolov5
}  // namespace element
}  // namespace sophon_stream
