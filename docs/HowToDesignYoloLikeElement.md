# 类 YOLO 算法插件设计方案

本文以 `samples/yolov8/` 和 `element/algorithm/yolov8/` 为参考，说明如何在 sophon-stream 中设计并新增一个类似 YOLO 的算法插件。这里的“类 YOLO 插件”指输入为图像帧、输出为检测框或类似结构化结果的深度学习算法插件，例如 YOLOv3/YOLOv10/自研检测模型等。

## 1. 设计目标

新增插件应满足以下目标：

1. 能作为 sophon-stream 的 `Element` 被 `Graph` 创建和连接。
2. 能被注册为普通插件名，例如 `myyolo`，也能被注册为 group 插件名，例如 `myyolo_group`。
3. 支持 `pre`、`infer`、`post` 三阶段拆分，复用框架的 `Group<T>` 能力。
4. 支持多路视频流、多线程、batch 推理。
5. 使用 `ObjectMetadata` 承载输入帧、模型输入输出 tensor 和后处理结果。
6. 能接入现有 sample 主程序，通过 JSON 配置完成运行。

推荐设计的数据流如下：

```text
decode
  -> myyolo_group
       -> myyolo_pre
       -> myyolo_infer
       -> myyolo_post
  -> sink handler 或下游插件
```

如果需要绘制、编码或上报，也可以继续扩展为：

```text
decode -> myyolo_group -> osd -> encode
decode -> myyolo_group -> filter -> http_push
```

## 2. yolov8 示例的关键结构

`samples/yolov8/` 负责提供 demo 级配置：

```text
samples/yolov8/
├── README.md
├── config/
│   ├── decode.json
│   ├── engine_group.json
│   ├── yolov8_demo.json
│   └── yolov8_group.json
└── scripts/
    └── download.sh
```

`element/algorithm/yolov8/` 是真正的插件实现：

```text
element/algorithm/yolov8/
├── CMakeLists.txt
├── include/
│   ├── yolov8.h
│   ├── yolov8_context.h
│   ├── yolov8_pre_process.h
│   ├── yolov8_inference.h
│   └── yolov8_post_process.h
└── src/
    ├── yolov8.cc
    ├── yolov8_pre_process.cc
    ├── yolov8_inference.cc
    └── yolov8_post_process.cc
```

其中 `yolov8.cc` 是插件主类，负责：

1. 注册插件名 `yolov8` 和 `yolov8_group`。
2. 解析配置并初始化 `Context`。
3. 创建预处理、推理、后处理对象。
4. 在 `doWork()` 中从输入队列取数据、凑 batch、执行当前 stage、推送输出。

`yolov8_pre_process.cc` 负责：

1. 从 `ObjectMetadata::mFrame->mSpData` 取得 `bm_image`。
2. 做颜色格式转换、宽度对齐、letterbox resize、ROI crop。
3. 做归一化和数据类型转换。
4. 将模型输入显存挂到 `ObjectMetadata::mInputBMtensors`。

`yolov8_inference.cc` 负责：

1. 使用 `BMNNNetwork` 执行 `forward()`。
2. batch 大于 1 时合并多帧输入显存。
3. 分配输出 tensor 显存。
4. 将输出拆回每个 `ObjectMetadata::mOutputBMtensors`。

`yolov8_post_process.cc` 负责：

1. 解析模型输出 tensor。
2. 执行置信度过滤和 NMS。
3. 坐标从网络输入空间映射回原图空间。
4. 将结果写入 `mDetectedObjectMetadatas`、`mPosedObjectMetadatas`、`mSegmentedObjectMetadatas`、`mObbObjectMetadatas` 等字段。

## 3. 新插件目录设计

假设新插件命名为 `myyolo`，建议目录如下：

```text
element/algorithm/myyolo/
├── CMakeLists.txt
├── README.md
├── include/
│   ├── myyolo.h
│   ├── myyolo_context.h
│   ├── myyolo_pre_process.h
│   ├── myyolo_inference.h
│   └── myyolo_post_process.h
└── src/
    ├── myyolo.cc
    ├── myyolo_pre_process.cc
    ├── myyolo_inference.cc
    └── myyolo_post_process.cc
```

可以用模板脚本生成基础目录：

```bash
cd element/algorithm
./algorithm_maker.sh myyolo
```

模板生成后，再参考 `yolov8` 替换和完善实现。

## 4. 类职责设计

### 4.1 MyyoloContext

`Context` 保存插件运行期共享的只读或低频修改参数。它会被 `pre`、`infer`、`post` 三阶段共享。

建议字段：

```cpp
class MyyoloContext : public ::sophon_stream::element::Context {
 public:
  int deviceId;

  std::shared_ptr<BMNNContext> bmContext;
  std::shared_ptr<BMNNNetwork> bmNetwork;
  bm_handle_t handle;

  std::vector<float> mean;
  std::vector<float> stdd;
  bool bgr2rgb = true;

  float thresh_conf_min = 0.5;
  float thresh_nms = 0.5;
  std::unordered_map<std::string, float> thresh_conf;
  std::vector<std::string> class_names;
  bool class_thresh_valid = false;

  int class_num = 80;
  int net_h = 0;
  int net_w = 0;
  int m_net_channel = 3;
  int max_batch = 1;
  int input_num = 0;
  int output_num = 0;

  bool use_post_opt = false;
  bool bgr_packed_input = false;

  bmcv_convert_to_attr converto_attr;

  bmcv_rect_t roi;
  bool roi_predefined = false;
  int thread_number = 1;
};
```

如果模型只做检测，可以先不要设计 `TaskType`。如果希望同时支持检测、分类、分割、姿态，则可以参考 `Yolov8Context` 增加：

```cpp
enum class TaskType { Detect = 0, Pose, Cls, Seg, Obb };
TaskType taskType = TaskType::Detect;
```

### 4.2 Myyolo 主 Element

`myyolo.h` 中的主类继承 `framework::Element`，并包含四个核心对象：

```cpp
class Myyolo : public ::sophon_stream::framework::Element {
 public:
  Myyolo();
  ~Myyolo() override;

  const static std::string elementName;

  common::ErrorCode initInternal(const std::string& json) override;
  common::ErrorCode doWork(int dataPipeId) override;

  void setContext(std::shared_ptr<::sophon_stream::element::Context> context);
  void setPreprocess(std::shared_ptr<::sophon_stream::element::PreProcess> pre);
  void setInference(std::shared_ptr<::sophon_stream::element::Inference> infer);
  void setPostprocess(
      std::shared_ptr<::sophon_stream::element::PostProcess> post);
  void setStage(bool pre, bool infer, bool post);
  void initProfiler(std::string name, int interval);

 private:
  std::shared_ptr<MyyoloContext> mContext;
  std::shared_ptr<MyyoloPreProcess> mPreProcess;
  std::shared_ptr<MyyoloInference> mInference;
  std::shared_ptr<MyyoloPostProcess> mPostProcess;

  bool use_pre = false;
  bool use_infer = false;
  bool use_post = false;

  std::string mFpsProfilerName;
  ::sophon_stream::common::FpsProfiler mFpsProfiler;

  common::ErrorCode initContext(const std::string& json);
  void process(common::ObjectMetadatas& objectMetadatas, int dataPipeId);
};
```

主类中最重要的函数是：

1. `initInternal()`：解析通用 stage 配置，创建 `Context/PreProcess/Inference/PostProcess`。
2. `initContext()`：解析算法参数、加载 bmodel、读取输入输出 shape、初始化归一化参数。
3. `process()`：按 `use_pre/use_infer/use_post` 依次执行三阶段。
4. `doWork()`：从 `DataPipe` 取数据、凑 batch、调用 `process()`、再推给下游。

### 4.3 MyyoloPreProcess

预处理建议与 `Yolov8PreProcess` 保持一致，除非模型输入格式不同。

设计职责：

1. 调用 `initTensors(context, objectMetadatas)` 初始化输入 tensor 容器。
2. 从 `ObjectMetadata` 取得原始 `bm_image`。
3. 根据模型输入要求转换格式：
   - `FORMAT_RGB_PLANAR`
   - `FORMAT_BGR_PLANAR`
   - `FORMAT_BGR_PACKED`
4. 如果宽度不满足硬件对齐要求，则创建 aligned image。
5. 做 letterbox resize，padding 值建议与训练保持一致，YOLO 常用 `114`。
6. 如果配置了 ROI，则只 crop ROI 区域。
7. 按 `mean/std/input_scale` 做 `bmcv_image_convert_to()`。
8. 将最终显存挂到 `mInputBMtensors->tensors[0]->device_mem`。

需要重点确认：

| 项目 | 说明 |
|---|---|
| 输入 shape | 常见为 `[N, 3, H, W]`，也可能是 `[N, H, W, 3]` |
| 颜色顺序 | 是否需要 `bgr2rgb` |
| 归一化 | `y = (x - mean) / std`，再乘模型 input scale |
| resize 策略 | 是否保持宽高比，是否 letterbox |
| padding 值 | 必须和模型训练/导出时一致 |

### 4.4 MyyoloInference

推理阶段通常可以直接复用 `Yolov8Inference` 的结构：

```cpp
common::ErrorCode MyyoloInference::predict(
    std::shared_ptr<MyyoloContext> context,
    common::ObjectMetadatas& objectMetadatas) {
  if (objectMetadatas.empty()) return common::ErrorCode::SUCCESS;

  if (context->max_batch > 1) {
    auto inputTensors = mergeInputDeviceMem(context, objectMetadatas);
    auto outputTensors = getOutputDeviceMem(context);
    context->bmNetwork->forward(inputTensors->tensors, outputTensors->tensors);
    splitOutputMemIntoObjectMetadatas(context, objectMetadatas, outputTensors);
  } else {
    if (objectMetadatas[0]->mFrame->mEndOfStream)
      return common::ErrorCode::SUCCESS;
    objectMetadatas[0]->mOutputBMtensors = getOutputDeviceMem(context);
    context->bmNetwork->forward(
        objectMetadatas[0]->mInputBMtensors->tensors,
        objectMetadatas[0]->mOutputBMtensors->tensors);
  }

  for (auto& obj : objectMetadatas) {
    obj->mInputBMtensors = nullptr;
  }

  return common::ErrorCode::SUCCESS;
}
```

如果模型有多个输入，需要扩展 `preProcess()` 和 `mergeInputDeviceMem()` 的输入 tensor 组织方式。如果模型输出多头特征图，需要在 `postProcess()` 中按输出数量逐个解析。

### 4.5 MyyoloPostProcess

后处理是类 YOLO 插件差异最大的部分，应单独设计。

基础检测模型建议流程：

```text
同步输出 tensor 到 CPU
  -> 根据输出 shape 解析候选框
  -> decode bbox
  -> 置信度过滤
  -> NMS
  -> 坐标反变换到原图
  -> 写入 DetectedObjectMetadata
```

结果写入方式建议使用：

```cpp
auto detData = std::make_shared<common::DetectedObjectMetadata>();
detData->mBox.mX = x1;
detData->mBox.mY = y1;
detData->mBox.mWidth = x2 - x1;
detData->mBox.mHeight = y2 - y1;
detData->mClassify = class_id;
detData->mScores.push_back(score);
obj->mDetectedObjectMetadatas.push_back(detData);
```

如果输出是分割、姿态或旋转框，应分别写入：

| 任务 | 建议元数据 |
|---|---|
| 检测 | `mDetectedObjectMetadatas` |
| 分类 | `mRecognizedObjectMetadatas` |
| 姿态 | `mDetectedObjectMetadatas` + `mPosedObjectMetadatas` |
| 分割 | `mDetectedObjectMetadatas` + `mSegmentedObjectMetadatas` |
| 旋转框 | `mObbObjectMetadatas` |

坐标反变换必须与预处理保持一致。如果预处理使用 letterbox，需要记录或重新计算：

```text
ratio = min(net_w / src_w, net_h / src_h)
pad_x = (net_w - src_w * ratio) / 2
pad_y = (net_h - src_h * ratio) / 2

x = (x_net - pad_x) / ratio
y = (y_net - pad_y) / ratio
```

如果启用了 ROI，则还需要加回 ROI 的 `start_x/start_y`。

## 5. 配置文件设计

### 5.1 插件配置 myyolo_group.json

建议新增：

```text
samples/myyolo/config/myyolo_group.json
```

示例：

```json
{
  "configure": {
    "model_path": "../myyolo/data/models/BM1684X/myyolo_int8_1b.bmodel",
    "threshold_conf": 0.5,
    "threshold_nms": 0.5,
    "bgr2rgb": true,
    "mean": [0, 0, 0],
    "std": [255, 255, 255],
    "class_names_file": "../myyolo/data/coco.names"
  },
  "shared_object": "../../build/lib/libmyyolo.so",
  "name": "myyolo_group",
  "side": "sophgo",
  "thread_number": 4
}
```

如果需要按类别设置阈值：

```json
"threshold_conf": {
  "person": 0.45,
  "car": 0.5
}
```

此时必须配置 `class_names_file`，并且类别数量要与模型输出类别数一致。

### 5.2 Graph 配置 engine_group.json

最小检测 graph 可以参考 `samples/yolov8/config/engine_group.json`：

```json
[
  {
    "graph_id": 0,
    "device_id": 0,
    "graph_name": "myyolo",
    "elements": [
      {
        "element_id": 5000,
        "element_config": "../myyolo/config/decode.json",
        "ports": {
          "input": [
            {
              "port_id": 0,
              "is_sink": false,
              "is_src": true
            }
          ]
        }
      },
      {
        "element_id": 5001,
        "element_config": "../myyolo/config/myyolo_group.json",
        "inner_elements_id": [10001, 10002, 10003],
        "ports": {
          "output": [
            {
              "port_id": 0,
              "is_sink": true,
              "is_src": false
            }
          ]
        }
      }
    ],
    "connections": [
      {
        "src_element_id": 5000,
        "src_port": 0,
        "dst_element_id": 5001,
        "dst_port": 0
      }
    ]
  }
]
```

`inner_elements_id` 是 group 模式的关键。框架会把 `myyolo_group` 拆成三个内部 element：

```text
10001: pre
10002: infer
10003: post
```

### 5.3 Demo 配置 myyolo_demo.json

建议新增：

```json
{
  "channels": [
    {
      "channel_id": 0,
      "url": "../myyolo/data/videos/test.mp4",
      "source_type": "VIDEO",
      "sample_interval": 1,
      "loop_num": 1,
      "fps": -1
    }
  ],
  "class_names": "../myyolo/data/coco.names",
  "download_image": false,
  "draw_func_name": "draw_yolov5_results",
  "engine_config_path": "../myyolo/config/engine_group.json"
}
```

如果新插件输出的是普通检测框，可以先复用 `draw_yolov5_results` 或 `draw_yolov8_results`。如果输出元数据结构不同，需要在 `samples/include/draw_funcs.h` 和相关实现中新增绘制函数。

## 6. CMake 接入设计

### 6.1 插件 CMakeLists.txt

`element/algorithm/myyolo/CMakeLists.txt` 可以从 `yolov8` 复制，并修改库名和源文件：

```cmake
add_library(myyolo SHARED
    src/myyolo_pre_process.cc
    src/myyolo_post_process.cc
    src/myyolo_inference.cc
    src/myyolo.cc
)

target_link_libraries(myyolo
    ${FFMPEG_LIBS}
    ${OpenCV_LIBS}
    ${BM_LIBS}
    ${JPU_LIBS}
    -lpthread
)
```

PCIe 和 SoC 两个分支都要修改。

### 6.2 顶层 CMakeLists.txt

在根目录 `CMakeLists.txt` 中增加：

```cmake
checkAndAddElement(element/algorithm/myyolo)
```

建议放在其他 `element/algorithm/*` 附近。

编译成功后，应生成：

```text
build/lib/libmyyolo.so
```

## 7. 插件注册设计

在 `myyolo.cc` 文件末尾注册：

```cpp
REGISTER_WORKER("myyolo", Myyolo)
REGISTER_GROUP_WORKER("myyolo_group",
                      sophon_stream::framework::Group<Myyolo>,
                      Myyolo)
```

注册名必须与 JSON 中的 `name` 一致：

```json
"name": "myyolo_group"
```

如果只注册了 `myyolo`，则无法使用 `myyolo_group`。如果要使用 group 三阶段拆分，必须注册 `REGISTER_GROUP_WORKER`。

## 8. initContext 设计要点

`initContext()` 建议按以下顺序实现：

1. 解析 JSON。
2. 读取 `model_path`。
3. 读取 `threshold_conf`、`threshold_nms`。
4. 读取 `bgr2rgb`、`mean`、`std`。
5. 读取可选 `roi`。
6. 创建 `BMNNHandle`。
7. 创建 `BMNNContext` 并加载 bmodel。
8. 获取 `BMNNNetwork`。
9. 获取输入 shape，设置 `net_h/net_w/m_net_channel/max_batch`。
10. 获取输出 tensor 数量和 shape，推导 `class_num`。
11. 根据 input scale 初始化 `bmcv_convert_to_attr`。
12. 保存 `thread_number`。

需要特别校验：

| 校验项 | 失败风险 |
|---|---|
| `model_path` 是否存在 | 初始化失败或加载 bmodel 失败 |
| `mean/std` 长度是否为 3 | 预处理归一化越界 |
| 输入 shape 是否符合预期 | `net_h/net_w` 解析错误 |
| 类别数是否匹配 | 后处理类别越界 |
| ROI 是否越界 | BMCV crop 失败 |

## 9. doWork 设计要点

`doWork()` 可以直接沿用 `Yolov8::doWork()` 的结构：

```text
获取 inputPort/outputPort
  -> 从 input DataPipe pop ObjectMetadata
  -> 跳过 mFilter=true 的对象
  -> 凑满 max_batch 或遇到 EOS
  -> process(objectMetadatas)
  -> 将 pendingObjectMetadatas 推给下游
  -> 更新 FPS profiler
```

设计注意事项：

1. `pendingObjectMetadatas` 要包含被 filter 的对象，否则被过滤帧可能丢失，影响下游时序。
2. `objectMetadatas` 只包含真正需要处理的对象。
3. 遇到 `mEndOfStream` 要及时 break，并让 EOS 继续传递到下游。
4. 非 sink element 时，输出 DataPipe 通常按 `channel_id_internal % capacity` 分配。
5. sink element 时，输出 DataPipe 使用 0。

## 10. 后处理设计细节

### 10.1 输出 shape 适配

不同 YOLO 模型导出的输出 shape 差异很大，例如：

```text
[1, 84, 8400]
[1, 8400, 84]
[1, num_boxes, 4 + class_num]
多输出 head
带 objectness
不带 objectness
```

设计时应先打印或读取模型输出 shape，再明确 decode 规则：

```cpp
auto shape = context->bmNetwork->outputTensor(0)->get_shape();
```

建议在 README 中写清楚插件支持的输出格式。

### 10.2 置信度计算

常见 YOLO 有两种形式：

```text
score = objectness * class_score
score = class_score
```

YOLOv8 检测通常没有单独 objectness，后处理时直接取类别最大分数。自定义模型必须确认导出格式。

### 10.3 NMS

普通检测框使用标准 IoU NMS。旋转框需要 rotated NMS 或类似 `probiou` 的逻辑。分割模型需要先对 bbox 做 NMS，再计算 mask。

### 10.4 元数据写入

为了复用现有 OSD 和 sample 绘制逻辑，普通检测结果优先写入：

```cpp
obj->mDetectedObjectMetadatas
```

字段至少应包括：

```text
mBox
mClassify
mScores
```

如果需要类别名称，通常由 sample 的 `class_names` 文件和绘图函数处理。

## 11. sample 目录设计

建议新增：

```text
samples/myyolo/
├── README.md
├── config/
│   ├── decode.json
│   ├── engine_group.json
│   ├── myyolo_demo.json
│   └── myyolo_group.json
├── scripts/
│   └── download.sh
└── data/
    ├── models/
    ├── videos/
    └── coco.names
```

如果不需要新增 sample，也可以暂时复用 `samples/yolov8`，只修改：

1. `yolov8_group.json` 中的 `shared_object` 和 `name`。
2. `engine_group.json` 中的 element config 路径。
3. `yolov8_demo.json` 中的输入视频和 engine 配置路径。

但长期维护建议单独建 `samples/myyolo`。

## 12. 开发步骤建议

推荐按以下顺序开发：

1. 使用 `algorithm_maker.sh myyolo` 生成基础文件。
2. 复制 `yolov8` 的 `CMakeLists.txt`，改库名和源文件名。
3. 在根目录 `CMakeLists.txt` 添加 `checkAndAddElement(element/algorithm/myyolo)`。
4. 在 `myyolo.cc` 中注册 `myyolo` 和 `myyolo_group`。
5. 实现 `MyyoloContext`，先只支持 Detect。
6. 实现 `initContext()`，完成 bmodel 加载和 shape 解析。
7. 复制并裁剪 `Yolov8PreProcess`，保证输入 tensor 正确。
8. 复用 `Yolov8Inference` 的推理结构。
9. 实现最小检测后处理，只输出 `DetectedObjectMetadata`。
10. 新增 `samples/myyolo/config`。
11. 编译生成 `libmyyolo.so`。
12. 先用 1 路视频、1 线程、batch 1 验证。
13. 再开启多线程、多路视频和 batch 模型验证。

## 13. 验证清单

编译验证：

```bash
mkdir build
cd build
cmake ..
make -j
```

产物检查：

```text
build/lib/libmyyolo.so
samples/build/main
```

运行验证：

```bash
cd samples/build
./main ../myyolo/config/myyolo_demo.json
```

如果项目实际运行命令与当前 sample README 不同，以 sample README 为准。

功能验证：

1. 插件注册日志中能看到 `myyolo` 和 `myyolo_group`。
2. Graph 初始化成功。
3. `decode` 能向 `myyolo_group` 推送帧。
4. `pre` 阶段无 BMCV 内存或格式错误。
5. `infer` 阶段无 bmodel forward 错误。
6. `post` 阶段能输出非空检测结果。
7. sink handler 能收到 `ObjectMetadata`。
8. 如果 `download_image=true`，结果图能正常保存。
9. 多路视频下没有 DataPipe 堵塞或显存耗尽。
10. EOS 能正常传递，程序能结束。

## 14. 常见问题

### 14.1 找不到插件

检查：

1. 是否生成 `build/lib/libmyyolo.so`。
2. JSON 中 `shared_object` 路径是否正确。
3. JSON 中 `name` 是否与 `REGISTER_WORKER` 或 `REGISTER_GROUP_WORKER` 一致。
4. 顶层 CMake 是否添加了 `checkAndAddElement(element/algorithm/myyolo)`。

### 14.2 预处理结果不对

重点检查：

1. RGB/BGR 是否与训练一致。
2. `mean/std` 是否与模型导出一致。
3. 是否需要 letterbox。
4. padding 值是否正确。
5. 输入 shape 是 NCHW 还是 NHWC。

### 14.3 检测框偏移

通常是坐标反变换与预处理不一致：

1. 预处理是否保持宽高比。
2. 是否有 `pad_x/pad_y`。
3. 是否启用了 ROI。
4. 是否把网络输入坐标误当成原图坐标。

### 14.4 batch 模型异常

检查：

1. `context->max_batch` 是否正确。
2. `mergeInputDeviceMem()` 是否支持当前输入数量和 shape。
3. `splitOutputMemIntoObjectMetadatas()` 拆分输出时是否按正确 batch 维处理。

## 15. 最小交付内容

一个可维护的类 YOLO 插件至少应提交：

```text
element/algorithm/myyolo/
├── CMakeLists.txt
├── README.md
├── include/
│   ├── myyolo.h
│   ├── myyolo_context.h
│   ├── myyolo_pre_process.h
│   ├── myyolo_inference.h
│   └── myyolo_post_process.h
└── src/
    ├── myyolo.cc
    ├── myyolo_pre_process.cc
    ├── myyolo_inference.cc
    └── myyolo_post_process.cc

samples/myyolo/
├── README.md
├── config/
│   ├── decode.json
│   ├── engine_group.json
│   ├── myyolo_demo.json
│   └── myyolo_group.json
└── scripts/
    └── download.sh
```

同时修改：

```text
CMakeLists.txt
```

如果新结果类型无法复用现有绘图函数，还需要修改：

```text
samples/include/draw_funcs.h
samples/src/main.cc 或绘图函数所在实现文件
```

## 16. 推荐实现策略

如果目标是快速落地一个检测插件，建议先做最小版本：

1. 只支持 `Detect`。
2. 只支持单输出 bmodel。
3. 只支持 `[N, boxes, attrs]` 或 `[N, attrs, boxes]` 中一种明确格式。
4. 只输出 `DetectedObjectMetadata`。
5. 先不支持类别阈值、ROI、分割、姿态和 OBB。

跑通后再逐步增加：

1. 多输出 head。
2. 类别阈值。
3. ROI。
4. OSD 绘制适配。
5. 多任务输出。
6. batch 优化。

这样可以把风险集中在后处理 decode 这一处，避免同时排查预处理、推理、配置和绘图问题。
