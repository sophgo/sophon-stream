# yolo8-test 插件 POC 修改说明

本文记录本次为了验证 sophon-stream 新增算法插件流程而新增的 `yolo8-test` POC 插件。该插件以 `element/algorithm/yolov8` 和 `samples/yolov8` 为蓝图，复用 YOLOv8 的前处理、推理、后处理逻辑，并为新插件建立独立的目录、C++ 类型、CMake 目标、动态库和运行时注册名。

## 1. 修改目标

本次修改目标是快速验证新增插件的完整链路：

```text
新增 element/algorithm 插件
  -> 接入顶层 CMake
  -> 编译生成新动态库
  -> 新 sample JSON 通过 shared_object 加载新动态库
  -> ElementFactory 通过新注册名创建新 Element
  -> Graph 按 decode -> yolo8-test_group 运行
```

本次不重新设计模型算法，也不改 SophonSDK 推理逻辑；模型、视频、类别文件继续复用 `samples/yolov8/data`。

## 2. 新增插件目录

新增目录：

```text
element/algorithm/yolo8-test/
```

该目录由 `element/algorithm/yolov8/` 复制并重命名而来。由于 C++ 标识符不能包含 `-`，所以目录和运行时注册名使用 `yolo8-test`，C++ namespace 和文件名使用 `yolo8_test`。

主要文件：

```text
element/algorithm/yolo8-test/
├── CMakeLists.txt
├── README.md
├── README_EN.md
├── include/
│   ├── yolo8_test.h
│   ├── yolo8_test_context.h
│   ├── yolo8_test_inference.h
│   ├── yolo8_test_post_process.h
│   └── yolo8_test_pre_process.h
└── src/
    ├── yolo8_test.cc
    ├── yolo8_test_inference.cc
    ├── yolo8_test_post_process.cc
    └── yolo8_test_pre_process.cc
```

## 3. 插件命名关系

| 项目 | 名称 |
|---|---|
| 插件目录 | `element/algorithm/yolo8-test` |
| C++ namespace | `sophon_stream::element::yolo8_test` |
| C++ 主类 | `Yolo8Test` |
| Context 类 | `Yolo8TestContext` |
| PreProcess 类 | `Yolo8TestPreProcess` |
| Inference 类 | `Yolo8TestInference` |
| PostProcess 类 | `Yolo8TestPostProcess` |
| 普通插件注册名 | `yolo8-test` |
| Group 插件注册名 | `yolo8-test_group` |
| CMake target | `yolo8-test` |
| 编译产物 | `build/lib/libyolo8-test.so` |

## 4. CMake 接入

修改根目录 `CMakeLists.txt`，在原 `yolov8` 插件附近加入：

```cmake
checkAndAddElement(element/algorithm/yolo8-test)
```

新增插件自身的 `CMakeLists.txt` 使用：

```cmake
add_library(yolo8-test SHARED
    src/yolo8_test_pre_process.cc
    src/yolo8_test_post_process.cc
    src/yolo8_test_inference.cc
    src/yolo8_test.cc
)
```

因此编译后会生成：

```text
build/lib/libyolo8-test.so
```

## 5. 运行时注册

在 `element/algorithm/yolo8-test/src/yolo8_test.cc` 中注册：

```cpp
REGISTER_WORKER("yolo8-test", Yolo8Test)
REGISTER_GROUP_WORKER("yolo8-test_group",
                      sophon_stream::framework::Group<Yolo8Test>,
                      Yolo8Test)
```

这使得 JSON 中可以通过：

```json
"name": "yolo8-test_group"
```

创建新的 group 插件。

## 6. 新增 sample

新增目录：

```text
samples/yolo8-test/
```

主要配置文件：

```text
samples/yolo8-test/config/
├── decode.json
├── engine_group.json
├── yolo8-test_demo.json
├── yolo8-test_group.json
└── yolo8-test_classthresh_roi_example.json
```

`engine_group.json` 中的 graph 使用新插件配置：

```json
"graph_name": "yolo8-test"
```

```json
"element_config": "../yolo8-test/config/yolo8-test_group.json"
```

`yolo8-test_group.json` 使用新动态库和新注册名：

```json
"shared_object": "../../build/lib/libyolo8-test.so",
"name": "yolo8-test_group"
```

## 7. 资源复用

为了快速 POC，未复制大模型和视频文件。新 sample 复用原 YOLOv8 资源：

```json
"model_path": "../yolov8/data/models/BM1684X/yolov8s_int8_1b.bmodel"
```

```json
"url": "../yolov8/data/videos/test_car_person_1080P.avi"
```

```json
"class_names": "../yolov8/data/coco.names"
```

这意味着运行 `yolo8-test` demo 前，仍需保证 `samples/yolov8/data` 下存在对应模型、视频和类别文件。

## 8. 运行方式

编译后进入 `samples/build`：

```bash
./main --demo_config_path=../yolo8-test/config/yolo8-test_demo.json
```

如果只验证插件加载和推理链路，建议先将输入路数降为 1 路，并把 `thread_number` 降为 1，降低设备显存压力。

## 9. 本次未做事项

本次按用户要求只做代码和配置修改，未执行编译。

未新增独立模型下载脚本，未复制 yolov8 大模型和测试视频，未修改 sample 主程序、绘图函数或 OSD 插件。普通检测结果继续复用现有 `draw_yolov5_results` 绘图函数。
