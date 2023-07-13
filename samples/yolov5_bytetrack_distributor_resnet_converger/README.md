# 目标检测-跟踪-分发-属性识别 Demo

## 目录
- [目标检测-跟踪-分发-属性识别 Demo](#目标检测-跟踪-分发-属性识别-demo)
  - [目录](#目录)
  - [1. 简介](#1-简介)
  - [2. 特性](#2-特性)
  - [3. 准备模型与数据](#3-准备模型与数据)
  - [4. 环境准备](#4-环境准备)
    - [4.2 SoC平台](#42-soc平台)
  - [5. 程序编译](#5-程序编译)
    - [5.1 x86/arm PCIe平台](#51-x86arm-pcie平台)
    - [5.2 SoC平台](#52-soc平台)
  - [6. 程序运行](#6-程序运行)
    - [6.1 Json配置说明](#61-json配置说明)
    - [6.2 运行](#62-运行)
  - [7. 性能测试](#7-性能测试)

## 1. 简介

本例程用于说明如何使用sophon-stream快速构建包含了多算法和按类别发往不同分支的复杂应用。

本例程插件的连接方式如下图所示：

![distributor.png](pics/distributor.png)

## 2. 特性
* 检测模型使用yolov5；
* 跟踪模型使用bytetrack；
* 分类模型使用resnet18；
* 支持BM1684X(x86 PCIe、SoC)和BM1684(x86 PCIe、SoC、arm PCIe)
* 支持多路视频流
* 支持多线程

## 3. 准备模型与数据

​在`scripts`目录下提供了相关模型和数据的下载脚本 [download.sh](./scripts/download.sh)。

```bash
# 安装unzip，若已安装请跳过，非ubuntu系统视情况使用yum或其他方式安装
sudo apt install unzip
chmod -R +x scripts/
./scripts/download.sh
```

下载的模型包括：

```bash
./models
├── BM1684
│   ├── resnet50_fp32_1b.bmodel                     # 用于BM1684的RESNET50 FP32 Bmodel，batch_size=1，imagenet
│   ├── resnet50_fp32_4b.bmodel                     # 用于BM1684的RESNET50 FP32 Bmodel，batch_size=4，imagenet
│   ├── resnet50_int8_1b.bmodel                     # 用于BM1684的RESNET50 INT8 Bmodel，batch_size=1，imagenet
│   ├── resnet50_int8_4b.bmodel                     # 用于BM1684的RESNET50 INT8 Bmodel，batch_size=4，imagenet
│   ├── resnet_pedestrian_gender_fp32_1b.bmodel     # 用于BM1684的RESNET18 FP32 Bmodel，batch_size=1，行人性别分类
│   ├── resnet_pedestrian_gender_fp32_4b.bmodel     # 用于BM1684的RESNET18 FP32 Bmodel，batch_size=4，行人性别分类
│   ├── resnet_pedestrian_gender_int8_1b.bmodel     # 用于BM1684的RESNET18 INT8 Bmodel，batch_size=1，行人性别分类
│   ├── resnet_pedestrian_gender_int8_4b.bmodel     # 用于BM1684的RESNET18 INT8 Bmodel，batch_size=4，行人性别分类
│   ├── resnet_vehicle_color_fp32_1b.bmodel         # 用于BM1684的RESNET18 FP32 Bmodel，batch_size=1，车辆颜色分类
│   ├── resnet_vehicle_color_fp32_4b.bmodel         # 用于BM1684的RESNET18 FP32 Bmodel，batch_size=4，车辆颜色分类
│   ├── resnet_vehicle_color_int8_1b.bmodel         # 用于BM1684的RESNET18 INT8 Bmodel，batch_size=1，车辆颜色分类
│   ├── resnet_vehicle_color_int8_4b.bmodel         # 用于BM1684的RESNET18 INT8 Bmodel，batch_size=4，车辆颜色分类
│   ├── yolov5s_v6.1_3output_fp32_1b.bmodel         # 用于BM1684的YOLOV5 FP32 BModel，batch_size=1，后处理在CPU上进行
│   ├── yolov5s_v6.1_3output_int8_1b.bmodel         # 用于BM1684的YOLOV5 INT8 BModel，batch_size=1，后处理在CPU上进行
│   └── yolov5s_v6.1_3output_int8_4b.bmodel         # 用于BM1684的YOLOV5 INT8 BModel，batch_size=4，后处理在CPU上进行
├── BM1684X
│   ├── resnet50_fp16_1b.bmodel                     # 用于BM1684X的RESNET50 FP16 Bmodel，batch_size=1，imagenet
│   ├── resnet50_fp32_1b.bmodel                     # 用于BM1684X的RESNET50 FP32 Bmodel，batch_size=1，imagenet
│   ├── resnet50_fp32_4b.bmodel                     # 用于BM1684X的RESNET50 FP32 Bmodel，batch_size=4，imagenet
│   ├── resnet50_int8_1b.bmodel                     # 用于BM1684X的RESNET50 INT8 Bmodel，batch_size=1，imagenet
│   ├── resnet50_int8_4b.bmodel                     # 用于BM1684X的RESNET50 INT8 Bmodel，batch_size=4，imagenet
│   ├── resnet_pedestrian_gender_fp16_1b.bmodel     # 用于BM1684X的RESNET18 FP16 Bmodel，batch_size=1，行人性别分类
│   ├── resnet_pedestrian_gender_fp32_1b.bmodel     # 用于BM1684X的RESNET18 FP32 Bmodel，batch_size=1，行人性别分类
│   ├── resnet_pedestrian_gender_fp32_4b.bmodel     # 用于BM1684X的RESNET18 FP32 Bmodel，batch_size=4，行人性别分类
│   ├── resnet_pedestrian_gender_int8_1b.bmodel     # 用于BM1684X的RESNET18 INT8 Bmodel，batch_size=1，行人性别分类
│   ├── resnet_pedestrian_gender_int8_4b.bmodel     # 用于BM1684X的RESNET18 INT8 Bmodel，batch_size=4，行人性别分类
│   ├── resnet_vehicle_color_fp16_1b.bmodel         # 用于BM1684X的RESNET18 FP16 Bmodel，batch_size=1，车辆颜色分类
│   ├── resnet_vehicle_color_fp32_1b.bmodel         # 用于BM1684X的RESNET18 FP32 Bmodel，batch_size=1，车辆颜色分类
│   ├── resnet_vehicle_color_fp32_4b.bmodel         # 用于BM1684X的RESNET18 FP32 Bmodel，batch_size=4，车辆颜色分类
│   ├── resnet_vehicle_color_int8_1b.bmodel         # 用于BM1684X的RESNET18 INT8 Bmodel，batch_size=1，车辆颜色分类
│   ├── resnet_vehicle_color_int8_4b.bmodel         # 用于BM1684X的RESNET18 INT8 Bmodel，batch_size=4，车辆颜色分类
│   ├── yolov5s_v6.1_3output_fp16_1b.bmodel         # 用于BM1684X的YOLOV5 FP16 BModel，batch_size=1，后处理在CPU上进行
│   ├── yolov5s_v6.1_3output_fp32_1b.bmodel         # 用于BM1684X的YOLOV5 FP32 BModel，batch_size=1，后处理在CPU上进行
│   ├── yolov5s_v6.1_3output_int8_1b.bmodel         # 用于BM1684X的YOLOV5 INT8 BModel，batch_size=1，后处理在CPU上进行
│   └── yolov5s_v6.1_3output_int8_4b.bmodel         # 用于BM1684X的YOLOV5 INT8 BModel，batch_size=4，后处理在CPU上进行
└── BM1684X_tpukernel
    ├── yolov5s_tpukernel_fp16_1b.bmodel            # 用于BM1684X的YOLOV5 FP16 BModel，batch_size=1，后处理采用tpu_kernel
    ├── yolov5s_tpukernel_fp32_1b.bmodel            # 用于BM1684X的YOLOV5 FP32 BModel，batch_size=1，后处理采用tpu_kernel
    ├── yolov5s_tpukernel_int8_1b.bmodel            # 用于BM1684X的YOLOV5 INT8 BModel，batch_size=1，后处理采用tpu_kernel
    └── yolov5s_tpukernel_int8_4b.bmodel            # 用于BM1684X的YOLOV5 INT8 BModel，batch_size=4，后处理采用tpu_kernel
```

下载的数据包括：

```bash
./videos/                                           # 测试视频
├── carvana_video.mp4
├── mot17_01_frcnn.mp4
├── mot17_03_frcnn.mp4
├── mot17_06_frcnn.mp4
├── mot17_07_frcnn.mp4
├── mot17_08_frcnn.mp4
├── mot17_12_frcnn.mp4
├── mot17_14_frcnn.mp4
├── test_car_person_1080P.avi
└── traffic.mp4
```

## 4. 环境准备

如果您在x86/arm平台安装了PCIe加速卡（如SC系列加速卡），可以直接使用它作为开发环境和运行环境。您需要安装libsophon、sophon-opencv和sophon-ffmpeg，具体步骤可参考[x86-pcie平台的开发和运行环境搭建](../../docs/EnvironmentInstallGuide.md#3-x86-pcie平台的开发和运行环境搭建)或[arm-pcie平台的开发和运行环境搭建](../../docs/EnvironmentInstallGuide.md#5-arm-pcie平台的开发和运行环境搭建)。

本例程依赖Eigen，您需要在编译程序的机器上运行如下命令安装：
```bash
sudo apt install libeigen3-dev
```

### 4.2 SoC平台

如果您使用SoC平台（如SE、SM系列边缘设备），刷机后在`/opt/sophon/`下已经预装了相应的libsophon、sophon-opencv和sophon-ffmpeg运行库包，可直接使用它作为运行环境。通常还需要一台x86主机作为开发环境，用于交叉编译C++程序。

本例程依赖Eigen，您需要在编译程序的机器上运行如下命令安装：
```bash
sudo apt install libeigen3-dev
```

## 5. 程序编译
程序运行前需要编译可执行文件。
### 5.1 x86/arm PCIe平台
可以直接在PCIe平台上编译程序，具体请参考[sophon-stream编译](../../docs/HowToMake.md)

### 5.2 SoC平台
通常在x86主机上交叉编译程序，您需要在x86主机上使用SOPHON SDK搭建交叉编译环境，将程序所依赖的头文件和库文件打包至sophon_sdk_soc目录中，具体请参考[sophon-stream编译](../../docs/HowToMake.md)。本例程主要依赖libsophon、sophon-opencv和sophon-ffmpeg运行库包。

## 6. 程序运行

### 6.1 Json配置说明

配置文件位于 [./config](./config/)目录，结构如下所示：

```bash
./config
├── bytetrack.json                                                    # bytetrack跟踪算法配置
├── converger.json                                                    # 汇聚element配置
├── decode.json                                                       # 解码配置
├── distributor_class.json                                            # 每帧按类别分发
├── distributor_frame_class.json                                      # 跳帧按类别分发
├── distributor_frame.json                                            # 跳帧分发full frame
├── distributor_time_class.json                                       # 间隔时间按类别分发（默认）
├── distributor_time.json                                             # 间隔时间分发full frame
├── engine.json                                                       # graph配置
├── resnet_car.json                                                   # resnet 车辆颜色分类
├── resnet_person.json                                                # resnet 行人性别分类
├── yolov5_bytetrack_distributor_resnet_converger_demo.json           # demo配置
├── yolov5_infer.json                                                 # yolov5 推理配置
├── yolov5_post.json                                                  # yolov5 后处理配置
└── yolov5_pre.json                                                   # yolov5 前处理配置
```

其中，[yolov5_bytetrack_distributor_resnet_converger_demo.json](./config/yolov5_bytetrack_distributor_resnet_converger_demo.json)是例程的整体配置文件，管理输入码流等信息。在一张图上可以支持多路数据的输入，num_channels_per_graph参数配置输入的路数，channel中包含码流url等信息。

配置文件中不指定`channel_id`属性的情况，会在demo中对每一路数据的`channel_id`从0开始默认赋值。

```json
{
  "num_channels_per_graph": 1,
  "channel": {
    "url": "../data/videos/traffic.mp4",
    "source_type": "VIDEO",
    "loop_num": 1,
    "fps": 25,
    "channel_id": 0
  },
  "class_names": "../data/coco.names",
  "car_attributes": "../data/car.attributes",
  "person_attributes": "../data/person.attributes",
  "download_image": true,
  "engine_config_path": "../config/engine.json"
}
```

[engine.json](./config/engine.json)包含对graph的配置信息，这部分配置确定之后基本不会发生更改。

在该文件内，需要初始化每个element的信息和element之间的连接方式。element_id是唯一的，起到标识身份的作用。element_config指向该element的详细配置文件地址，port_id是该element的输入输出端口编号，多输入或多输出的情况下，输入/输出编号也不可以重复。is_src标志当前端口是否是整张图的输入端口，is_sink标识当前端口是否是整张图的输出端口。
connection是所有element之间的连接方式，通过element_id和port_id确定。

[yolov5_pre.json](./config/yolov5_pre.json)等配置文件是对具体某个element的配置细节，设置了模型参数、动态库路径、阈值等信息。该配置文件不需要指定`id`字段和`device_id`字段，例程会将`engine.json`中指定的`element_id`和`device_id`传入。其中，`thread_number`是`element`内部的工作线程数量，一个线程会对应一个数据队列，多路输入情况下，需要合理设置数据队列数目，来保证线程工作压力均匀且合理。

### 6.2 运行

对于PCIe平台，可以直接在PCIe平台上运行测试；对于SoC平台，需将交叉编译生成的动态链接库、可执行文件、所需的模型和测试数据拷贝到SoC平台中测试。测试的参数及运行方式是一致的，下面主要以PCIe模式进行介绍。

运行可执行文件
```bash
./yolov5_bytetrack_distributor_resnet_converger_demo
```

运行结果存放在`./build/results`目录下。本例程默认配置方式为每秒按类别发送到resnet分支，会在结果目录中每秒保存一帧绘制了目标box、track_id和具体属性的图像。

![result.bmp](./pics/result.bmp)

>**注意：**

soc环境运行时如果报错
```bash
./yolov5_bytetrack_distributor_resnet_converger_demo: error while loading shared libraries: libframework.so: cannot open shared object file: No such file or directory
```

需要设置环境变量
```bash
export LD_LIBRARY_PATH=path-to/sophon-stream/build/lib/:$LD_LIBRARY_PATH
```

## 7. 性能测试
由于全流程依赖输入视频fps且画图速度慢，本例程暂不提供性能测试结果，如需各模型推理性能，请到对应模型例程查看。