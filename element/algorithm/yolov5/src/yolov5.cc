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
    mContext->m_bmContext = std::make_shared<BMNNContext>(
        handle, modelPathIt->get<std::string>().c_str());
    mContext->m_bmNetwork = mContext->m_bmContext->network(0);
    mContext->handle = handle->handle();

    // 2. get input
    mContext->max_batch = mContext->m_bmNetwork->maxBatch();
    auto inputTensor = mContext->m_bmNetwork->inputTensor(0);
    mContext->input_num = mContext->m_bmNetwork->m_netinfo->input_num;
    mContext->m_net_channel = inputTensor->get_shape()->dims[1];
    mContext->m_net_h = inputTensor->get_shape()->dims[2];
    mContext->m_net_w = inputTensor->get_shape()->dims[3];

    // 3. get output
    mContext->output_num = mContext->m_bmNetwork->outputTensorNum();
    mContext->min_dim =
        mContext->m_bmNetwork->outputTensor(0)->get_shape()->num_dims;
    mContext->class_num =
        mContext->m_bmNetwork->outputTensor(0)->get_shape()->dims[1] / 3 - 4 -
        1;  // class_nums + box_4 + conf_1

    // 4. initialize bmimages
    mContext->m_resized_imgs.resize(mContext->max_batch);
    mContext->m_converto_imgs.resize(mContext->max_batch);
    // some API only accept bm_image whose stride is aligned to 64
    int aligned_net_w = FFALIGN(mContext->m_net_w, 64);
    int strides[3] = {aligned_net_w, aligned_net_w, aligned_net_w};
    for (int i = 0; i < mContext->max_batch; i++) {
      auto ret = bm_image_create(mContext->m_bmContext->handle(),
                                 mContext->m_net_h, mContext->m_net_w,
                                 FORMAT_RGB_PLANAR, DATA_TYPE_EXT_1N_BYTE,
                                 &mContext->m_resized_imgs[i], strides);
      assert(BM_SUCCESS == ret);
    }
    bm_image_alloc_contiguous_mem(mContext->max_batch,
                                  mContext->m_resized_imgs.data());

    // 5.converto
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
          "../../share/3rdparty/tpu_kernel_module/libbm1684x_kernel_module.so";
      tpu_module = tpu_kernel_load_module_file(mContext->m_bmContext->handle(),
                                               tpu_kernel_module_path.c_str());
      mContext->func_id =
          tpu_kernel_get_function(mContext->m_bmContext->handle(), tpu_module,
                                  "tpu_kernel_api_yolov5_detect_out");
      std::cout << "Using tpu_kernel yolo postprocession, kernel funtion id: "
                << mContext->func_id << std::endl;

      int out_len_max = 25200 * 7;
      int input_num = mContext->m_bmNetwork->outputTensorNum();
      int batch_num = 1;  // 4b has bug, now only for 1b.

      bm_handle_t handle_ = mContext->m_bmContext->handle();
      bm_device_mem_t in_dev_mem[input_num];
      for (int i = 0; i < input_num; i++)
        in_dev_mem[i] =
            *mContext->m_bmNetwork->outputTensor(i)->get_device_mem();

      for (int i = 0; i < mContext->max_batch; i++) {
        mContext->output_tensor[i] = new float[out_len_max];
        for (int j = 0; j < input_num; j++) {
          mContext->api[i].bottom_addr[j] =
              bm_mem_get_device_addr(in_dev_mem[j]) +
              i * in_dev_mem[j].size / mContext->max_batch;
        }
        auto ret = bm_malloc_device_byte(handle_, &mContext->out_dev_mem[i],
                                         out_len_max * sizeof(float));
        assert(BM_SUCCESS == ret);
        ret = bm_malloc_device_byte(handle_, &mContext->detect_num_mem[i],
                                    batch_num * sizeof(int32_t));
        assert(BM_SUCCESS == ret);
        mContext->api[i].top_addr =
            bm_mem_get_device_addr(mContext->out_dev_mem[i]);
        mContext->api[i].detected_num_addr =
            bm_mem_get_device_addr(mContext->detect_num_mem[i]);

        // config
        mContext->api[i].input_num = input_num;
        mContext->api[i].batch_num = batch_num;
        for (int j = 0; j < input_num; ++j) {
          mContext->api[i].hw_shape[j][0] =
              mContext->m_bmNetwork->outputTensor(j)->get_shape()->dims[2];
          mContext->api[i].hw_shape[j][1] =
              mContext->m_bmNetwork->outputTensor(j)->get_shape()->dims[3];
        }
        mContext->api[i].num_classes = mContext->class_num;
        const std::vector<std::vector<std::vector<int>>> anchors{
            {{10, 13}, {16, 30}, {33, 23}},
            {{30, 61}, {62, 45}, {59, 119}},
            {{116, 90}, {156, 198}, {373, 326}}};
        mContext->api[i].num_boxes = anchors[0].size();
        mContext->api[i].keep_top_k = 200;
        mContext->api[i].nms_threshold =
            0.1 > mContext->thresh_nms ? 0.1 : mContext->thresh_nms;
        mContext->api[i].confidence_threshold =
            0.1 > mContext->thresh_conf ? 0.1 : mContext->thresh_conf;
        auto it = mContext->api[i].bias;
        for (const auto& subvector2 : anchors) {
          for (const auto& subvector1 : subvector2) {
            it = copy(subvector1.begin(), subvector1.end(), it);
          }
        }
        for (int j = 0; j < input_num; j++)
          mContext->api[i].anchor_scale[j] =
              mContext->m_net_h /
              mContext->m_bmNetwork->outputTensor(j)->get_shape()->dims[2];
        mContext->api[i].clip_box = 1;
      }
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

common::ErrorCode Yolov5::doWork() {
  common::ErrorCode errorCode = common::ErrorCode::SUCCESS;

  common::ObjectMetadatas objectMetadatas;
  std::vector<int> inputPorts = getInputPorts();
  std::vector<int> outputPorts = getInputPorts();
  int inputPort = inputPorts[0];
  int outputPort = outputPorts[0];

  while (objectMetadatas.size() < mBatch) {
    // 如果队列为空则等待
    auto data = getInputData(inputPort);
    if (!data) {
      continue;
    }
    popInputData(inputPort);

    auto objectMetadata =
        std::static_pointer_cast<common::ObjectMetadata>(data);

    objectMetadatas.push_back(objectMetadata);

    if (objectMetadata->mFrame->mEndOfStream) {
      break;
    }
  }
  if(use_pre)
    for(auto& obj : objectMetadatas)
    {
      printf("doWork before process channel_id: %d and frame_id: %d\n",
      obj->mFrame->mChannelId, obj->mFrame->mFrameId);
    }

  process(objectMetadatas);

  // if (use_pre) {
  //   int x = rand() % 100+200;
  //   printf("random x:%d\n", x);
  //   std::this_thread::sleep_for(std::chrono::milliseconds(x));
  // }

  for (auto& objectMetadata : objectMetadatas) {
    errorCode =
        pushOutputData(outputPort, std::static_pointer_cast<void>(objectMetadata),
                 std::chrono::milliseconds(200));
    if (common::ErrorCode::SUCCESS != errorCode) {
      IVS_WARN(
          "Send data fail, element id: {0:d}, output port: {1:d}, data: "
          "{2:p}",
          getId(), outputPort, static_cast<void*>(objectMetadata.get()));
    }
  }

  return common::ErrorCode::SUCCESS;
}

REGISTER_WORKER("Yolov5", Yolov5)

}  // namespace yolov5
}  // namespace element
}  // namespace sophon_stream
