# license_plate_recognition Demo

[English](README_EN.md) | 简体中文

## 目录

- [license_plate_recognition Demo](#license_plate_recognition-demo)
  - [目录](#目录)
  - [1. 简介](#1-简介)
  - [2. 特性](#2-特性)
  - [3. 准备模型与数据](#3-准备模型与数据)
  - [4. 环境准备](#4-环境准备)
    - [4.1 x86/arm PCIe 平台](#41-x86arm-pcie平台)
    - [4.2 SoC 平台](#42-soc平台)
  - [5. 程序编译](#5-程序编译)
    - [5.1 x86/arm PCIe 平台](#51-x86arm-pcie平台)
    - [5.2 SoC 平台](#52-soc平台)
  - [6. 程序运行](#6-程序运行)
    - [6.1 Json 配置说明](#61-json配置说明)
    - [6.2 运行](#62-运行)
  - [7. 性能测试](#7-性能测试)

## 1. 简介

本例程用于说明如何使用 sophon-stream 快速构建基于 yolov5 的车牌检测和基于 lprnet 的车牌识别。

**LPRNET 车牌检测源代码**(https://github.com/sirius-ai/LPRNet_Pytorch)

本例程中，yolov5、lprnet 算法的前处理、推理、后处理均分别在三个 element 上进行运算，element 内部可以开启多个线程，保证了一定的检测效率。

## 2. 特性

- 支持 BM1684X、BM1684(x86 PCIe、SoC)、BM1688(SoC)
- 支持多路视频流
- 支持多线程

## 3. 准备模型与数据

​ 在`scripts`目录下提供了相关模型和数据的下载脚本 [download.sh](./scripts/download.sh)。

```bash
# 安装7z、unzip，若已安装请跳过，非ubuntu系统视情况使用yum或其他方式安装
sudo apt install p7zip
sudo apt install p7zip-full
sudo apt install unzip
chmod -R +x scripts/
./scripts/download.sh
```

脚本执行完毕后，会在当前目录下生成`data`、`models`目录，其中，`data`存放车辆数据集，`models`存放 yolov5 和 lprnet 的模型文件。

下载的模型包括：

```bash
./models/
├── lprnet
│   ├── BM1684
│   │   ├── lprnet_fp32_1b.bmodel
│   │   ├── lprnet_int8_1b.bmodel
│   │   └── lprnet_int8_4b.bmodel
│   ├── BM1684X
│   │   ├── lprnet_fp16_1b.bmodel
│   │   ├── lprnet_fp32_1b.bmodel
│   │   ├── lprnet_int8_1b.bmodel
│   │   └── lprnet_int8_4b.bmodel
│   ├── BM1688
│   │   ├── lprnet_fp32_1b.bmodel
│   │   ├── lprnet_fp32_4b.bmodel
│   │   ├── lprnet_int8_1b.bmodel
│   │   └── lprnet_int8_4b.bmodel
│   ├── onnx
│   │   ├── lprnet_1b.onnx
│   │   └── lprnet_4b.onnx
│   └── torch
│       ├── Final_LPRNet_model.pth
│       └── LPRNet_model.torchscript
└── yolov5s-licensePLate
    ├── BM1684
    │   ├── yolov5s_v6.1_license_3output_fp32_1b.bmodel
    │   └── yolov5s_v6.1_license_3output_int8_1b.bmodel
    ├── BM1684X
    │   ├── yolov5s_v6.1_license_3output_fp32_1b.bmodel
    │   └── yolov5s_v6.1_license_3output_int8_1b.bmodel
    └── BM1688
        ├── yolov5s_v6.1_license_3output_fp32_1b.bmodel
        ├── yolov5s_v6.1_license_3output_fp32_4b.bmodel
        ├── yolov5s_v6.1_license_3output_int8_1b.bmodel
        └── yolov5s_v6.1_license_3output_int8_4b.bmodel
```

模型说明:

以上 lprnet 模型移植于[LNRNet_Pytorch](https://github.com/sirius-ai/LPRNet_Pytorch) , yolov5s-licensePLate 模型基于绿色车牌数据集训练。

下载的数据包括：

```bash
./datasets
├── coco.names
└── test // 用于测试的车辆数据集
```

## 4. 环境准备

### 4.1 x86/arm PCIe 平台

如果您在 x86/arm 平台安装了 PCIe 加速卡（如 SC 系列加速卡），可以直接使用它作为开发环境和运行环境。您需要安装 libsophon、sophon-opencv 和 sophon-ffmpeg，具体步骤可参考[x86-pcie 平台的开发和运行环境搭建](../../docs/EnvironmentInstallGuide.md#3-x86-pcie平台的开发和运行环境搭建)或[arm-pcie 平台的开发和运行环境搭建](../../docs/EnvironmentInstallGuide.md#5-arm-pcie平台的开发和运行环境搭建)。

### 4.2 SoC 平台

如果您使用 SoC 平台（如 SE、SM 系列边缘设备），刷机后在`/opt/sophon/`下已经预装了相应的 libsophon、sophon-opencv 和 sophon-ffmpeg 运行库包，可直接使用它作为运行环境。通常还需要一台 x86 主机作为开发环境，用于交叉编译 C++程序。

## 5. 程序编译

### 5.1 x86/arm PCIe 平台

可以直接在 PCIe 平台上编译程序，具体请参考[sophon-stream 编译](../../docs/HowToMake.md)。

### 5.2 SoC 平台

通常在 x86 主机上交叉编译程序，您需要在 x86 主机上使用 SOPHON SDK 搭建交叉编译环境，将程序所依赖的头文件和库文件打包至 sophon_sdk_soc 目录中，具体请参考[sophon-stream 编译](../../docs/HowToMake.md)。本例程主要依赖 libsophon、sophon-opencv 和 sophon-ffmpeg 运行库包。

## 6. 程序运行

### 6.1 Json 配置说明

license_plate_recognition demo 中各部分参数位于 [config](./config/) 目录，结构如下所示：

```bash
./config
├── converger.json
├── decode.json
├── distributor_time_class.json
├── engine_group.json
├── engine.json
├── license_plate_recognition_demo.json
├── lprnet_group.json
├── lprnet_infer.json
├── lprnet_post.json
├── lprnet_pre.json
├── yolov5_group.json
├── yolov5_infer.json
├── yolov5_post.json
└── yolov5_pre.json
```

其中，[license_plate_recognition.json](./config/license_plate_recognition.json)是例程的整体配置文件，管理输入码流等信息。在一张图上可以支持多路数据的输入，channels 参数配置输入的路数，sample_interval 设置跳帧数，loop_num 设置循环播放次数，channel 中包含码流 url 等信息。

配置文件中不指定`channel_id`属性的情况，会在 demo 中对每一路数据的`channel_id`从 0 开始默认赋值。

```json
{
    "channels": [
      {
        "channel_id": 0,
        "url": "../license_plate_recognition/data/test",
        "source_type": "IMG_DIR",
        "loop_num": 1,
        "fps": -1
      },
      {
        "channel_id": 1,
        "url": "../license_plate_recognition/data/test",
        "source_type": "IMG_DIR",
        "loop_num": 1,
        "fps": -1
      },
      {
        "channel_id": 2,
        "url": "../license_plate_recognition/data/test",
        "source_type": "IMG_DIR",
        "loop_num": 1,
        "fps": -1
      },
      {
        "channel_id": 3,
        "url": "../license_plate_recognition/data/test",
        "source_type": "IMG_DIR",
        "loop_num": 1,
        "fps": -1
      }
    ],
    "class_names": "../license_plate_recognition/data/coco.names",
    "download_image": true,
    "draw_func_name": "draw_license_plate_recognition_results",
    "engine_config_path": "../license_plate_recognition/config/engine_group.json"
}
```

[engine.json](./config/engine.json)包含对 graph 的配置信息，这部分配置确定之后基本不会发生更改。

在该文件内，需要初始化每个 element 的信息和 element 之间的连接方式。element_id 是唯一的，起到标识身份的作用。element_config 指向该 element 的详细配置文件地址，port_id 是该 element 的输入输出端口编号，多输入或多输出的情况下，输入/输出编号也不可以重复。is_src 标志当前端口是否是整张图的输入端口，is_sink 标识当前端口是否是整张图的输出端口。
connection 是所有 element 之间的连接方式，通过 element_id 和 port_id 确定。

[lprnet_pre.json](./config/lprnet_pre.json)等配置文件是对具体某个 element 的配置细节，设置了模型参数、动态库路径、阈值等信息。该配置文件不需要指定`id`字段和`device_id`字段，例程会将`engine.json`中指定的`element_id`和`device_id`传入。其中，`thread_number`是`element`内部的工作线程数量，一个线程会对应一个数据队列，多路输入情况下，需要合理设置数据队列数目，来保证线程工作压力均匀且合理。

```json
{
    "configure": {
        "model_path": "../license_plate_recognition/models/lprnet/BM1684X/lprnet_fp32_1b.bmodel",
        "stage": [
            "pre"
        ]
    },
    "shared_object": "../../build/lib/liblprnet.so",
    "name": "lprnet",
    "side": "sophgo",
    "thread_number": 1
}
```

### 6.2 运行

对于 PCIe 平台，可以直接在 PCIe 平台上运行测试；对于 SoC 平台，需将交叉编译生成的动态链接库、可执行文件、所需的模型和测试数据拷贝到 SoC 平台中测试。

SoC 平台上，动态库、可执行文件、配置文件、模型、视频数据的目录结构关系应与原始 sophon-stream 仓库中的关系保持一致。

测试的参数及运行方式是一致的，下面主要以 PCIe 模式进行介绍。

1. 运行可执行文件
```bash
./main --demo_config_path=../license_plate_recognition/config/license_plate_recognition_demo.json
```

推理结果保存在/build/results路径下。

关闭图片保存时，1684X PCIe上推理 1 路图片运行结果如下，PCIe上的性能由于CPU的不同可能存在较大差异：

```bash
 total time cost 24724501 us.
frame count is 5007 | fps is 202.512 fps.
```

## 7. 性能测试

本例程只供流程参考，暂无最佳性能数据。
