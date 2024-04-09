# retinaface Demo

[English](README_EN.md) | 简体中文

## 目录
- [retinaface Demo](#retinaface-demo)
  - [目录](#目录)
  - [1. 简介](#1-简介)
  - [2. 特性](#2-特性)
  - [3. 准备模型与数据](#3-准备模型与数据)
  - [4. 环境准备](#4-环境准备)
    - [4.1 x86/arm PCIe平台](#41-x86arm-pcie平台)
    - [4.2 SoC平台](#42-soc平台)
  - [5. 程序编译](#5-程序编译)
    - [5.1 x86/arm PCIe平台](#51-x86arm-pcie平台)
    - [5.2 SoC平台](#52-soc平台)
  - [6. 程序运行](#6-程序运行)
    - [6.1 Json配置说明](#61-json配置说明)
    - [6.2 运行](#62-运行)
  - [7. 性能测试](#7-性能测试)

## 1. 简介

本例程用于说明如何使用sophon-stream快速构建视频目标检测应用。

**源代码** (https://github.com/biubug6/Pytorch_Retinaface)

本例程中，retinaface算法的前处理、推理、后处理分别在三个element上进行运算，element内部可以开启多个线程，保证了一定的检测效率

## 2. 特性

* 支持BM1684X、BM1684(x86 PCIe、SoC)、BM1688(SoC)、CV186X(SoC)
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
./models/
│   ├── BM1684
│   │   ├── retinaface_mobilenet0.25_fp32_1b.bmodel # 用于BM1684的FP32 BModel，batch_size=1，后处理在CPU上进行
│   │   ├── retinaface_mobilenet0.25_int8_1b.bmodel # 用于BM1684的INT8 BModel，batch_size=1，后处理在CPU上进行
│   │   └── retinaface_mobilenet0.25_int8_4b.bmodel # 用于BM1684的INT8 BModel，batch_size=4，后处理在CPU上进行
│   ├── BM1684X
│   │   ├── retinaface_mobilenet0.25_fp16_1b.bmodel # 用于BM1684X的FP16 BModel，batch_size=1，后处理在CPU上进行
│   │   ├── retinaface_mobilenet0.25_fp32_1b.bmodel # 用于BM1684X的FP32 BModel，batch_size=1，后处理在CPU上进行
│   │   ├── retinaface_mobilenet0.25_int8_1b.bmodel # 用于BM1684X的INT8 BModel，batch_size=1，后处理在CPU上进行
│   │   └── retinaface_mobilenet0.25_int8_4b.bmodel # 用于BM1684X的INT8 BModel，batch_size=4，后处理在CPU上进行
│   ├── BM1688
│   │   ├── retinaface_mobilenet0.25_fp16_1b_2core.bmodel # 用于BM1688的FP16 BModel，batch_size=1，后处理在CPU上进行
│   │   ├── retinaface_mobilenet0.25_fp16_1b.bmodel       # 用于BM1688和CV186X的FP16 BModel，batch_size=1，后处理在CPU上进行
│   │   ├── retinaface_mobilenet0.25_fp32_1b_2core.bmodel # 用于BM1688的FP32 BModel，batch_size=1，后处理在CPU上进行
│   │   ├── retinaface_mobilenet0.25_fp32_1b.bmodel       # 用于BM1688和CV186X的FP32 BModel，batch_size=1，后处理在CPU上进行
│   │   ├── retinaface_mobilenet0.25_int8_1b_2core.bmodel # 用于BM1688的INT8 BModel，batch_size=1，后处理在CPU上进行
│   │   ├── retinaface_mobilenet0.25_int8_1b.bmodel       # 用于BM1688和CV186X的INT8 BModel，batch_size=1，后处理在CPU上进行
│   │   ├── retinaface_mobilenet0.25_int8_4b_2core.bmodel # 用于BM1688的FP16 BModel，batch_size=4，后处理在CPU上进行
│   │   └── retinaface_mobilenet0.25_int8_4b.bmodel       # 用于BM1688和CV186X的INT8 BModel，batch_size=4，后处理在CPU上进行
│   └── onnx
│       └── retinaface_mobilenet0.25.onnx # 原模型
```

模型说明:

以上模型移植于[Retinaface官方](https://github.com/biubug6/Pytorch_Retinaface)，插件配置`mean=[104,117,123]`，`std=[1,1,1]`。

下载的数据包括：

```bash
./images/
├── face            # 测试图片和视频
│   └── test
│       ├── face
│       └── videos
├── WIDERVAL
└── wind
```

## 4. 环境准备

### 4.1 x86/arm PCIe平台

如果您在x86/arm平台安装了PCIe加速卡（如SC系列加速卡），可以直接使用它作为开发环境和运行环境。您需要安装libsophon、sophon-opencv和sophon-ffmpeg，具体步骤可参考[x86-pcie平台的开发和运行环境搭建](../../docs/EnvironmentInstallGuide.md#3-x86-pcie平台的开发和运行环境搭建)或[arm-pcie平台的开发和运行环境搭建](../../docs/EnvironmentInstallGuide.md#5-arm-pcie平台的开发和运行环境搭建)。

### 4.2 SoC平台

如果您使用SoC平台（如SE、SM系列边缘设备），刷机后在`/opt/sophon/`下已经预装了相应的libsophon、sophon-opencv和sophon-ffmpeg运行库包，可直接使用它作为运行环境。通常还需要一台x86主机作为开发环境，用于交叉编译C++程序。

## 5. 程序编译

### 5.1 x86/arm PCIe平台
可以直接在PCIe平台上编译程序，具体请参考[sophon-stream编译](../../docs/HowToMake.md)

### 5.2 SoC平台
通常在x86主机上交叉编译程序，您需要在x86主机上使用SOPHON SDK搭建交叉编译环境，将程序所依赖的头文件和库文件打包至sophon_sdk_soc目录中，具体请参考[sophon-stream编译](../../docs/HowToMake.md)。本例程主要依赖libsophon、sophon-opencv和sophon-ffmpeg运行库包。

## 6. 程序运行

### 6.1 Json配置说明

retinaface demo中各部分参数位于 [config](./config/) 目录，结构如下所示：

```bash
config/
├── decode.json                 # 解码配置
├── engine_group.json           # sophon-stream 简化的graph配置
├── engine.json                 # sophon-stream graph配置，需要分别配置前处理、推理和后处理文件
├── retinaface_demo.json        # demo输入配置文件
├── retinaface_group.json       # 简化的retinaface配置文件，将retinaface的前处理、推理、后处理合到一个配置文件中
├── retinaface_infer.json       # retinaface 推理配置文件
├── retinaface_post.json        # retinaface 后处理配置文件
└── retinaface_pre.json         # retinaface 前处理配置文件
```

其中，[retinaface_demo.json](./config/retinaface_demo.json)是例程的整体配置文件，管理输入码流等信息。在一张图上可以支持多路数据的输入，channels参数配置输入的路数，sample_interval设置跳帧数，loop_num设置循环播放次数，channel中包含码流url等信息。

配置文件中不指定`channel_id`属性的情况，会在demo中对每一路数据的`channel_id`从0开始默认赋值。

```json
{
  "channels": [
    {
      "channel_id": 2,
      "url": "../retinaface/data/images/wind",
      "source_type": "IMG_DIR",
      "loop_num": 1,
      "sample_interval": 1,
      "fps": -1
    },
    {
      "channel_id": 3,
      "url": "../retinaface/data/images/wind",
      "source_type": "IMG_DIR",
      "loop_num": 1,
      "sample_interval": 1,
      "fps": -1
    },
    {
      "channel_id": 20,
      "url": "../retinaface/data/images/wind",
      "source_type": "IMG_DIR",
      "loop_num": 1,
      "sample_interval": 1,
      "fps": -1
    },
    {
      "channel_id": 30,
      "url": "../retinaface/data/images/wind",
      "source_type": "IMG_DIR",
      "loop_num": 1,
      "sample_interval": 1,
      "fps": -1
    }
  ],
  "download_image": true,
  "draw_func_name": "draw_retinaface_results",
  "engine_config_path": "../retinaface/config/engine_group.json"
}
```

[engine_group.json](./config/engine_group.json)包含对graph的配置信息，这部分配置确定之后基本不会发生更改。

这里摘取配置文件的一部分作为示例：在该文件内，需要初始化每个element的信息和element之间的连接方式。element_id是唯一的，起到标识身份的作用。element_config指向该element的详细配置文件地址，port_id是该element的输入输出端口编号，多输入或多输出的情况下，输入/输出编号也不可以重复。is_src标志当前端口是否是整张图的输入端口，is_sink标识当前端口是否是整张图的输出端口。
connection是所有element之间的连接方式，通过element_id和port_id确定。

```json
[
    {
        "graph_id": 0,
        "device_id": 0,
        "graph_name": "retinaface",
        "elements": [
            {
                "element_id": 5000,
                "element_config": "../retinaface/config/decode.json",
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
                "element_config": "../retinaface/config/retinaface_group.json",
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

[retinaface_group.json](./config/retinaface_group.json)等配置文件是对具体某个element的配置细节，设置了模型参数、动态库路径、阈值等信息。该配置文件不需要指定`id`字段和`device_id`字段，例程会将`engine.json`中指定的`element_id`和`device_id`传入。其中，`thread_number`是`element`内部的工作线程数量，一个线程会对应一个数据队列，多路输入情况下，需要合理设置数据队列数目，来保证线程工作压力均匀且合理。

```json
{
    "configure": {
        "model_path": "../retinaface/data/models/BM1684X/retinaface_mobilenet0.25_fp32_1b.bmodel",
        "max_face_count":50,
        "score_threshold":0.5,
        "bgr2rgb": false,
        "mean": [
            104,
            117,
            123
        ],
        "std": [
            1,
            1,
            1
        ]
    },
    "shared_object": "../../build/lib/libretinaface.so",
    "name": "retinaface",
    "side": "sophgo",
    "thread_number": 4
}
```


### 6.2 运行

对于PCIe平台，可以直接在PCIe平台上运行测试；对于SoC平台，需将交叉编译生成的动态链接库、可执行文件、所需的模型和测试数据拷贝到SoC平台中测试。测试的参数及运行方式是一致的，下面主要以PCIe模式进行介绍。

运行可执行文件
```bash
./main --demo_config_path=../retinaface/config/retinaface_demo.json
```

8路视频流运行结果如下:
```bash
total time cost 4798582 us.
frame count is 920 | fps is 191.723 fps.
```

## 7. 性能测试

目前，retinaface例程支持在BM1684X和BM1684的PCIE、SOC模式下进行推理，支持在BM1688和CV186X的SOC模式下进行推理。

测试数据`/data/images/wind`，编译选项为Release模式，使用int8模型，结果如下:

|设备|路数|算法线程数|CPU利用率(%)|平均FPS|
|----|----|-----|-----|-----|
|SE7    |4  |4-4-4  |381  |428.797|
|SE9-16 |4  |4-4-4  |400  |263.333|

> **测试说明**：
1. 性能测试结果具有一定的波动性，建议多次测试取平均值；
2. SE5/SE7主控CPU均为8核 ARM A53 42320 DMIPS @2.3GHz，SE9-16为8核CA53@1.6GHz，SE9-8为6核CA53@1.6GH；