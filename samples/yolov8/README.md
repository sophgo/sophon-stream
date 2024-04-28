# YOLOv8 Demo

[English](README_EN.md) | 简体中文

## 目录
- [YOLOv8 Demo](#yolov8-demo)
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

**源代码** (https://github.com/ultralytics/ultralytics)

本例程中，yolov8算法的前处理、推理、后处理分别在三个element上进行运算，element内部可以开启多个线程，保证了一定的检测效率

## 2. 特性

* 支持BM1684X、BM1684(x86 PCIe、SoC)、BM1688(SoC)
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

脚本执行完毕后，会在当前目录下生成`data`目录，其中包含`models`和`videos`两个子目录。

下载的模型包括：

```bash
./models/
├── BM1684
|   ├── yolov8n_cls_fp32_1b.bmodel  # 使用TPU-MLIR编译，用于BM1684的FP32 yolov8-cls BModel，batch_size=1
│   ├── yolov8n_pose_fp32_1b.bmodel # 使用TPU-MLIR编译，用于BM1684的FP32 yolov8-pose BModel，batch_size=1
│   ├── yolov8n_pose_int8_1b.bmodel # 使用TPU-MLIR编译，用于BM1684的INT8 yolov8-pose BModel，batch_size=1
│   ├── yolov8s_fp32_1b.bmodel   # 使用TPU-MLIR编译，用于BM1684的FP32 yolov8-detect BModel，batch_size=1
│   ├── yolov8s_int8_1b.bmodel   # 使用TPU-MLIR编译，用于BM1684的INT8 yolov8-detect BModel，batch_size=1
│   └── yolov8s_int8_4b.bmodel   # 使用TPU-MLIR编译，用于BM1684的INT8 yolov8-detect BModel，batch_size=4
├── BM1684X
|   ├── yolov8n_cls_fp32_1b.bmodel  # 使用TPU-MLIR编译，用于BM1684X的FP32 yolov8-cls BModel，batch_size=1
│   ├── yolov8n_pose_fp32_1b.bmodel # 使用TPU-MLIR编译，用于BM1684X的FP32 yolov8-pose BModel，batch_size=1
│   ├── yolov8n_pose_int8_1b.bmodel # 使用TPU-MLIR编译，用于BM1684X的INT8 yolov8-pose BModel，batch_size=1
│   ├── yolov8s_fp32_1b.bmodel   # 使用TPU-MLIR编译，用于BM1684X的FP32 yolov8-detect BModel，batch_size=1
│   ├── yolov8s_fp16_1b.bmodel   # 使用TPU-MLIR编译，用于BM1684X的FP16 yolov8-detect BModel，batch_size=1
│   ├── yolov8s_int8_1b.bmodel   # 使用TPU-MLIR编译，用于BM1684X的INT8 yolov8-detect BModel，batch_size=1
│   └── yolov8s_int8_4b.bmodel   # 使用TPU-MLIR编译，用于BM1684X的INT8 yolov8-detect BModel，batch_size=4
└── BM1688
    ├── yolov8n_cls_fp32_1b.bmodel    # 使用TPU-MLIR编译，用于BM1688的FP32 yolov8-cls BModel，batch_size=1
    ├── yolov8n_cls_fp32_1b_2core.bmodel    # 使用TPU-MLIR编译，用于BM1688的FP32 双核 yolov8-cls BModel，batch_size=1
    ├── yolov8n_pose_fp32_1b_1core.bmodel   # 使用TPU-MLIR编译，用于BM1688的FP32 单核 yolov8-pose BModel，batch_size=1
    ├── yolov8n_pose_fp32_1b_2core.bmodel   # 使用TPU-MLIR编译，用于BM1688的FP32 双核 yolov8-pose BModel，batch_size=1
    ├── yolov8n_pose_int8_1b_1core.bmodel   # 使用TPU-MLIR编译，用于BM1688的INT8 单核 yolov8-pose BModel，batch_size=1
    ├── yolov8n_pose_int8_1b_2core.bmodel   # 使用TPU-MLIR编译，用于BM1688的INT8 双核 yolov8-pose BModel，batch_size=1
    ├── yolov8s_fp16_1b_2core.bmodel  # 使用TPU-MLIR编译，用于BM1688的FP16 双核 yolov8-detect BModel，batch_size=1
    ├── yolov8s_fp16_1b.bmodel        # 使用TPU-MLIR编译，用于BM1688的FP16 单核 yolov8-detect BModel，batch_size=1
    ├── yolov8s_fp16_4b_2core.bmodel  # 使用TPU-MLIR编译，用于BM1688的FP16 双核 yolov8-detect BModel，batch_size=4
    ├── yolov8s_fp16_4b.bmodel        # 使用TPU-MLIR编译，用于BM1688的FP16 单核 yolov8-detect BModel，batch_size=4
    ├── yolov8s_fp32_1b_2core.bmodel  # 使用TPU-MLIR编译，用于BM1688的FP32 双核 yolov8-detect BModel，batch_size=1
    ├── yolov8s_fp32_1b.bmodel        # 使用TPU-MLIR编译，用于BM1688的FP32 单核 yolov8-detect BModel，batch_size=1
    ├── yolov8s_fp32_4b_2core.bmodel  # 使用TPU-MLIR编译，用于BM1688的FP32 双核 yolov8-detect BModel，batch_size=4
    ├── yolov8s_fp32_4b.bmodel        # 使用TPU-MLIR编译，用于BM1688的FP32 单核 yolov8-detect BModel，batch_size=4
    ├── yolov8s_int8_1b_2core.bmodel  # 使用TPU-MLIR编译，用于BM1688的INT8 双核 yolov8-detect BModel，batch_size=1
    ├── yolov8s_int8_1b.bmodel        # 使用TPU-MLIR编译，用于BM1688的INT8 单核 yolov8-detect BModel，batch_size=1
    ├── yolov8s_int8_4b_2core.bmodel  # 使用TPU-MLIR编译，用于BM1688的INT8 双核 yolov8-detect BModel，batch_size=4
    └── yolov8s_int8_4b.bmodel        # 使用TPU-MLIR编译，用于BM1688的INT8 单核 yolov8-detect BModel，batch_size=4
```

模型说明:

以上模型移植于[yolov8官方](https://github.com/ultralytics/ultralytics)，插件配置`mean=[0,0,0]`，`std=[255,255,255]`。

其中，名为`yolov8n_cls`的模型支持基于`ImageNet`的1000类分类任务，名为`yolov8n_pose`的模型支持人体姿态检测任务，其它模型支持COCO数据集的80分类检测任务。

任务配置时，需要参考[yolov8_element](../../element/algorithm/yolov8/README.md)的说明来修改配置文件。

目前，默认的配置方式实现的是目标检测功能。如果希望运行姿态检测算法，则除了需要将模型和任务修改外，还需要将[yolov8_demo.json](./config/yolov8_demo.json)中可视化算法名称修改为`draw_yolov8_det_pose`。对于分类算法，因为分类任务没有可视化的结果，因此不需要配置可视化算法名称，也不需要保存图片，观察程序运行中的日志即可。

下载的数据包括：

```bash
videos/
├── demo_skeleton.mp4     # 姿态检测测试视频1
├── yaotou.mp4            # 姿态检测测试视频2
├── carvana_video.mp4     # 目标检测测试视频
├── elevator-1080p-25fps-4000kbps.h264
├── mot17_01_frcnn.mp4
├── mot17_03_frcnn.mp4
├── mot17_06_frcnn.mp4
├── mot17_07_frcnn.mp4
├── mot17_08_frcnn.mp4
├── mot17_12_frcnn.mp4
├── mot17_14_frcnn.mp4
├── sample_1080p_h265.mp4
└── test_car_person_1080P.avi

./pics/                     # 分类任务测试数据
├── bus.jpg
├── n01440764_10043.jpg
├── n01440764_10470.jpg
├── n01440764_10744.jpg
├── n01440764_10845.jpg
├── n01440764_11170.jpg
├── n01440764_12021.jpg
├── n01440764_12063.jpg
├── n01440764_12090.jpg
├── n01440764_12329.jpg
└── n01440764_12435.jpg
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

yolov8 demo中各部分参数位于 [config](./config/) 目录，结构如下所示：

```bash
./config/
├── decode.json                 # 解码配置
├── engine_group.json           # sophon-stream graph配置
├── yolov8_classthresh_roi_example.json  # yolov8按照类别设置阈值的参考配置文件，需要注意，按类别设置阈值仅支持非tpu_kernel的后处理模式
├── yolov8_demo.json            # demo输入配置文件
├── yolov8_group.json           # 简化的yolov8配置文件，将yolov8的前处理、推理、后处理合到一个配置文件中
```

其中，[yolov8_demo.json](./config/yolov8_demo.json)是例程的整体配置文件，管理输入码流等信息。在一张图上可以支持多路数据的输入，channels参数配置输入的路数，sample_interval设置跳帧数，loop_num设置循环播放次数，channel中包含码流url等信息。download_image控制是否保存推理结果，若为false则不保存，若为true，则会保存在/build/results目录下。

配置文件中不指定`channel_id`属性的情况，会在demo中对每一路数据的`channel_id`从0开始默认赋值。

```json
{
  "channels": [
    {
      "channel_id": 2,
      "url": "../yolov8/data/videos/test_car_person_1080P.avi",
      "source_type": "VIDEO",
      "sample_interval": 1,
      "loop_num": 1,
      "fps": -1
    },
    {
      "channel_id": 3,
      "url": "../yolov8/data/videos/test_car_person_1080P.avi",
      "source_type": "VIDEO",
      "sample_interval": 1,
      "loop_num": 1,
      "fps": -1
    },
    {
      "channel_id": 20,
      "url": "../yolov8/data/videos/test_car_person_1080P.avi",
      "source_type": "VIDEO",
      "sample_interval": 1,
      "loop_num": 1,
      "fps": -1
    },
    {
      "channel_id": 30,
      "url": "../yolov8/data/videos/test_car_person_1080P.avi",
      "source_type": "VIDEO",
      "sample_interval": 1,
      "loop_num": 1,
      "fps": -1
    }
  ],
  "class_names": "../yolov8/data/coco.names",
  "download_image": true,
  "draw_func_name": "draw_yolov8_results",
  "engine_config_path": "../yolov8/config/engine_group.json"
}
```

[engine_group.json](./config/engine_group.json)包含对graph的配置信息，这部分配置确定之后基本不会发生更改。

需要注意，部署环境下的NPU等设备内存大小会显著影响例程运行的路数。如果默认的输入路数运行中出现了申请内存失败等错误，可以考虑把输入路数减少，再进行测试。

这里摘取配置文件的一部分作为示例：在该文件内，需要初始化每个element的信息和element之间的连接方式。element_id是唯一的，起到标识身份的作用。element_config指向该element的详细配置文件地址，port_id是该element的输入输出端口编号，多输入或多输出的情况下，输入/输出编号也不可以重复。is_src标志当前端口是否是整张图的输入端口，is_sink标识当前端口是否是整张图的输出端口。
connection是所有element之间的连接方式，通过element_id和port_id确定。

```json
[
    {
        "graph_id": 0,
        "device_id": 0,
        "graph_name": "yolov8",
        "elements": [
            {
                "element_id": 5000,
                "element_config": "../yolov8/config/decode.json",
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
                "element_config": "../yolov8/config/yolov8_group.json",
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

[yolov8_group.json](./config/yolov8_group.json)等配置文件是对具体某个element的配置细节，设置了模型参数、动态库路径、阈值等信息。该配置文件不需要指定`id`字段和`device_id`字段，例程会将`engine_group.json`中指定的`element_id`和`device_id`传入。其中，`thread_number`是`element`内部的工作线程数量，一个线程会对应一个数据队列，多路输入情况下，需要合理设置数据队列数目，来保证线程工作压力均匀且合理。

```json
{
    "configure": {
        "model_path": "../yolov8/data/models/BM1684X/yolov8s_int8_1b.bmodel",
        "threshold_conf": 0.5,
        "threshold_nms": 0.5,
        "bgr2rgb": true,
        "mean": [
            0,
            0,
            0
        ],
        "std": [
            255,
            255,
            255
        ]
    },
    "shared_object": "../../build/lib/libyolov8.so",
    "name": "yolov8_group",
    "side": "sophgo",
    "thread_number": 4
}
```

### 6.2 运行

对于PCIe平台，可以直接在PCIe平台上运行测试；对于SoC平台，需将交叉编译生成的动态链接库、可执行文件、所需的模型和测试数据拷贝到SoC平台中测试。

SoC平台上，动态库、可执行文件、配置文件、模型、视频数据的目录结构关系应与原始sophon-stream仓库中的关系保持一致。

测试的参数及运行方式是一致的，下面主要以PCIe模式进行介绍。

1. 运行可执行文件
```bash
./main --demo_config_path=../yolov8/config/yolov8_demo.json
```

2路视频流运行结果如下
```bash
 total time cost 13714673 us.
frame count is 1424 | fps is 103.83 fps.
```

## 7. 性能测试


目前，yolov8例程支持在BM1684X和BM1684的PCIe、SoC模式，BM1688的SoC模式下进行推理。

在不同的设备上可能需要修改json配置，例如模型路径、输入路数等。json的配置方法参考6.1节，程序运行方法参考上文6.2节。

由于PCIe设备cpu能力差距较大，性能数据没有参考意义，这里只给出SoC模式的测试结果。

测试视频`test_car_person_1080P.avi`，编译选项为Release模式，测试yolov8 检测模型性能，结果如下:

|设备|路数|算法线程数|CPU利用率(%)|系统内存(M)|TPU利用率(%)|设备内存峰值(M)|平均FPS|
|----|----|-----|-----|-----|-----|-----|---|
|SE7|4|4-4-4|470|96|40|2388|95.24|
|SE5-16|4|4-4-4|453|96|90|2700|89.02|
|SE9-16|4|4-4-4|489|111|70|2700|72|

> **测试说明**：
1. 性能测试结果具有一定的波动性，建议多次测试取平均值；
2. BM1684/1684X SoC的主控CPU均为8核 ARM A53 42320 DMIPS @2.3GHz；
3. 以上性能测试均基于int8模型给出；
4. 在BM1684设备上运行时，batch_size为4的模型可以达到更高的fps；
5. 上表中，输入路数和算法线程数的设置请参考[json配置说明](#61-json配置说明)，CPU利用率和系统内存使用top命令可查，TPU利用率和设备内存使用bm-smi命令可查，fps可以从运行程序打印的log中获得;
