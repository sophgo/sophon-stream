//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#include "yolo8_test.h"

using namespace std::chrono_literals;

namespace sophon_stream {
namespace element {
namespace yolo8_test {

Yolo8Test::Yolo8Test() {}

Yolo8Test::~Yolo8Test() {}

const std::string Yolo8Test::elementName = "yolo8-test";

// 配置中的 task_type 字符串会映射到这里的任务类型，
// 后处理入口据此选择检测、分类、姿态、分割或旋转框分支。
std::unordered_map<std::string, TaskType> taskMap{{"Detect", TaskType::Detect},
                                                  {"Pose", TaskType::Pose},
                                                  {"Cls", TaskType::Cls},
                                                  {"Seg", TaskType::Seg},
                                                  {"Obb", TaskType::Obb},
                                                  {"SegFuse", TaskType::SegFuse}};

common::ErrorCode Yolo8Test::initContext(const std::string& json) {
  common::ErrorCode errorCode = common::ErrorCode::SUCCESS;
  do {
    // 解析算法私有配置：模型路径、阈值、预处理参数、ROI 等。
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
                   "Invalid Task Type in Yolo8Test Config File!");
      mContext->taskType = taskMap[taskName];
    }

    // threshold_conf 支持两种形式：
    // 浮点数表示全类别共用阈值；map 表示按类别名分别设置阈值。
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

    // 记录所有类别阈值中的最小值，便于后处理先做一次快速候选过滤。
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

    // 1. 加载 bmodel 并取得网络句柄，后续三阶段都共享这个上下文。
    BMNNHandlePtr handle = std::make_shared<BMNNHandle>(mContext->deviceId);
    mContext->bmContext = std::make_shared<BMNNContext>(
        handle, modelPathIt->get<std::string>().c_str());
    mContext->bmNetwork = mContext->bmContext->network(0);
    mContext->handle = handle->handle();

    // 2. 读取输入 tensor 信息，确定 batch、输入尺寸和 NCHW/NHWC 布局。
    mContext->max_batch = mContext->bmNetwork->maxBatch();
    auto inputTensor = mContext->bmNetwork->inputTensor(0);
    mContext->input_num = mContext->bmNetwork->m_netinfo->input_num;
    // Detect BGR_PACKED format: shape [1, H, W, 3] instead of [1, 3, H, W]
    {
      auto* shape = inputTensor->get_shape();
      if (shape->dims[3] == 3) {
        mContext->bgr_packed_input = true;
        mContext->m_net_channel = shape->dims[3];
        mContext->net_h = shape->dims[1];
        mContext->net_w = shape->dims[2];
      } else {
        mContext->m_net_channel = shape->dims[1];
        mContext->net_h = shape->dims[2];
        mContext->net_w = shape->dims[3];
      }
    }

    // 3. 读取输出 tensor 信息，根据 shape 推导类别数和后处理格式。
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
        // Auto-detect Pose model from output shape when task_type is not
        // explicitly set. COCO pose model output shape: [1, 56, N] where
        // 56 = 4(bbox) + 1(person class) + 17*3(keypoints)
        int smaller_dim = ndim1 > ndim2 ? ndim2 : ndim1;
        if (smaller_dim - 4 == 1 + 17 * 3) {
          IVS_INFO(
              "Auto-detected Pose model from output shape (channels={0:d})",
              smaller_dim);
          mContext->taskType = TaskType::Pose;
          mContext->class_num = 1;
        } else if (ndim1 > ndim2) {
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
      else if (mContext->taskType == TaskType::Obb){
        int ndim1 = mContext->bmNetwork->outputTensor(0)->get_shape()->dims[1];
        int ndim2 = mContext->bmNetwork->outputTensor(0)->get_shape()->dims[2];
        if (ndim1 < ndim2){
          IVS_CRITICAL(
            "We only support bmodel's output_shape like [N, box_num, nout], usually box_num > nout. "
            "But your bmodel's shape is [{0:d}, {1:d}, {2:d}].", mContext->max_batch, ndim1, ndim2);
          abort();
        }
        mContext->class_num = mContext->bmNetwork->outputTensor(0)->get_shape()->dims[2] - 5;
      } else if (mContext->taskType == TaskType::SegFuse) {
        mContext->class_num =
            mContext->class_names.empty() ? 80 : mContext->class_names.size();
      }
    }

    // 如果启用类别阈值，类别名、阈值表和模型输出类别数必须完全对齐。
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

    // 4. 计算 BMCV convert_to 的归一化参数：
    // output = input * alpha + beta，结合 bmodel input scale 完成量化适配。
    float input_scale = inputTensor->get_scale();
    mContext->converto_attr.alpha_0 = input_scale / (mContext->stdd[0]);
    mContext->converto_attr.beta_0 =
        -(mContext->mean[0]) / (mContext->stdd[0]) * input_scale;
    mContext->converto_attr.alpha_1 = input_scale / (mContext->stdd[1]);
    mContext->converto_attr.beta_1 =
        -(mContext->mean[1]) / (mContext->stdd[1]) * input_scale;
    mContext->converto_attr.alpha_2 = input_scale / (mContext->stdd[2]);
    mContext->converto_attr.beta_2 =
        -(mContext->mean[2]) / (mContext->stdd[2]) * input_scale;

    // 5. 可选 ROI：配置后只对原图指定区域做检测，后处理会再把坐标加回原图。
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

common::ErrorCode Yolo8Test::initInternal(const std::string& json) {
  common::ErrorCode errorCode = common::ErrorCode::SUCCESS;
  do {
    // 校验 configure JSON。Element 基类已处理 id/name/thread 等通用字段。
    auto configure = nlohmann::json::parse(json, nullptr, false);
    if (!configure.is_object()) {
      errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
      break;
    }

    // stage 决定当前 Yolo8Test 实例负责哪一段。
    // group 模式下框架会创建三个内部实例：pre、infer、post。
    auto stageNameIt = configure.find(CONFIG_INTERNAL_STAGE_NAME_FIELD);
    if (configure.end() != stageNameIt && stageNameIt->is_array()) {
      std::vector<std::string> stages =
          stageNameIt->get<std::vector<std::string>>();
      if (std::find(stages.begin(), stages.end(), "pre") != stages.end()) {
        use_pre = true;
        mFpsProfilerName = "fps_yolo8-test_pre";
      }
      if (std::find(stages.begin(), stages.end(), "infer") != stages.end()) {
        use_infer = true;
        mFpsProfilerName = "fps_yolo8-test_infer";
      }
      if (std::find(stages.begin(), stages.end(), "post") != stages.end()) {
        use_post = true;
        mFpsProfilerName = "fps_yolo8-test_post";
      }

      mFpsProfiler.config(mFpsProfilerName, 100);
    }

    // 新建 context、预处理、推理、后处理对象。
    // Group<Yolo8Test> 会让三个内部实例共享这些对象和模型上下文。
    mContext = std::make_shared<Yolo8TestContext>();
    mPreProcess = std::make_shared<Yolo8TestPreProcess>();
    mInference = std::make_shared<Yolo8TestInference>();
    mPostProcess = std::make_shared<Yolo8TestPostProcess>();

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

void Yolo8Test::process(common::ObjectMetadatas& objectMetadatas, int dataPipeId) {
  common::ErrorCode errorCode = common::ErrorCode::SUCCESS;
  // 根据 use_pre/use_infer/use_post 执行当前实例负责的阶段。
  // 单个 Yolo8Test element 可同时执行三段；yolo8-test_group 通常每个内部 element 只执行一段。
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

common::ErrorCode Yolo8Test::doWork(int dataPipeId) {
  common::ErrorCode errorCode = common::ErrorCode::SUCCESS;

  common::ObjectMetadatas objectMetadatas;
  // Yolo8Test 插件只使用第一个输入端口和第一个输出端口。
  std::vector<int> inputPorts = getInputPorts();
  int inputPort = inputPorts[0];
  int outputPort = 0;
  if (!getSinkElementFlag()) {
    std::vector<int> outputPorts = getOutputPorts();
    outputPort = outputPorts[0];
  }

  common::ObjectMetadatas pendingObjectMetadatas;

  // 从当前线程绑定的 DataPipe 中取数据，最多凑满模型 max_batch。
  // objectMetadatas 是真正参与算法的数据；pendingObjectMetadatas 还保留被过滤数据，
  // 这样下游仍能收到完整帧序列和 EOS。
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

  // 执行预处理、推理、后处理中的当前阶段。
  process(objectMetadatas, dataPipeId);

  // 将处理完成的数据推给下游。若当前 element 是 sink，则 pushOutputData 会触发 sink handler。
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

void Yolo8Test::setStage(bool pre, bool infer, bool post) {
  // Group<Yolo8Test> 调用该函数，为内部三个 element 分配职责。
  use_pre = pre;
  use_infer = infer;
  use_post = post;
}

void Yolo8Test::initProfiler(std::string name, int interval) {
  mFpsProfiler.config(name, 100);
}

void Yolo8Test::setContext(
    std::shared_ptr<::sophon_stream::element::Context> context) {
  // check
  mContext = std::dynamic_pointer_cast<Yolo8TestContext>(context);
}

void Yolo8Test::setPreprocess(
    std::shared_ptr<::sophon_stream::element::PreProcess> pre) {
  mPreProcess = std::dynamic_pointer_cast<Yolo8TestPreProcess>(pre);
}

void Yolo8Test::setInference(
    std::shared_ptr<::sophon_stream::element::Inference> infer) {
  mInference = std::dynamic_pointer_cast<Yolo8TestInference>(infer);
}

void Yolo8Test::setPostprocess(
    std::shared_ptr<::sophon_stream::element::PostProcess> post) {
  mPostProcess = std::dynamic_pointer_cast<Yolo8TestPostProcess>(post);
}

REGISTER_WORKER("yolo8-test", Yolo8Test)
// 注册 group 版本。JSON 配置 name="yolo8-test_group" 时会创建 Group<Yolo8Test>，
// 再自动展开成 pre、infer、post 三个内部 Element。
REGISTER_GROUP_WORKER("yolo8-test_group", sophon_stream::framework::Group<Yolo8Test>,
                      Yolo8Test)

}  // namespace yolo8_test
}  // namespace element
}  // namespace sophon_stream
