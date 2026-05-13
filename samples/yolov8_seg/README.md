# YOLOv8 Seg Demo

[English](README_EN.md) | 简体中文

## 目录
- [YOLOv8 Seg Demo](#yolov8-seg-demo)
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

本例程用于说明如何使用sophon-stream快速构建视频实例分割应用。

**源代码** (https://github.com/ultralytics/ultralytics)

本例程中，yolov8-seg算法的前处理、推理、后处理分别在三个element上进行运算，element内部可以开启多个线程，保证了一定的检测效率。

后处理参考 [sophon-demo YOLOv8_plus_seg](https://github.com/sophon-ai/sophon-demo) 的cpp示例实现，使用CPU完成mask的生成和渲染。可视化结果通过osd element输出，同时绘制检测框和分割掩码（mask）。

## 2. 特性

* 支持BM1684X(x86 PCIe、SoC)
* 支持多路视频流
* 支持多线程
* 支持检测框+分割掩码（mask）的可视化渲染
* 后处理与sophon-demo YOLOv8_plus_seg对齐

## 3. 准备模型与数据

模型来自sophon-demo的YOLOv8_plus_seg示例，可从sophon-demo仓库获取。

```bash
# 下载sophon-demo中的模型
# 从 open@sophgo.com:sophon-demo/YOLOv8_plus_seg/BM1684X.tar.gz 下载并解压
tar -xzf BM1684X.tar.gz
cp -r BM1684X/* ../yolov8_seg/data/models/BM1684X/
```

下载的模型包括：

```bash
./models/BM1684X/
├── yolov8s_fp32_1b.bmodel   # 使用TPU-MLIR编译，用于BM1684X的FP32 yolov8-seg BModel，batch_size=1
├── yolov8s_fp16_1b.bmodel   # 使用TPU-MLIR编译，用于BM1684X的FP16 yolov8-seg BModel，batch_size=1
├── yolov8s_int8_1b.bmodel   # 使用TPU-MLIR编译，用于BM1684X的INT8 yolov8-seg BModel，batch_size=1
├── yolov8s_int8_4b.bmodel   # 使用TPU-MLIR编译，用于BM1684X的INT8 yolov8-seg BModel，batch_size=4
├── yolov9s_fp32_1b.bmodel   # 使用TPU-MLIR编译，用于BM1684X的FP32 yolov9-seg BModel，batch_size=1
├── yolov9s_fp16_1b.bmodel   # 使用TPU-MLIR编译，用于BM1684X的FP16 yolov9-seg BModel，batch_size=1
├── yolov9c_fp32_1b.bmodel   # 使用TPU-MLIR编译，用于BM1684X的FP32 yolov9-seg BModel，batch_size=1
├── yolov9c_fp16_1b.bmodel   # 使用TPU-MLIR编译，用于BM1684X的FP16 yolov9-seg BModel，batch_size=1
├── yolov9c_int8_1b.bmodel   # 使用TPU-MLIR编译，用于BM1684X的INT8 yolov9-seg BModel，batch_size=1
└── yolov9c_int8_4b.bmodel   # 使用TPU-MLIR编译，用于BM1684X的INT8 yolov9-seg BModel，batch_size=4
```

模型说明：

以上模型移植于[yolov8官方](https://github.com/ultralytics/ultralytics)，插件配置`mean=[0,0,0]`，`std=[255,255,255]`。

模型输出包含2个tensor：
- 检测输出（transposed格式 `[1, 8400, 116]`）：包含bbox坐标（4）、类别分数（80）、mask系数（32）
- 分割输出（`[1, 32, 160, 160]`）：mask prototypes

> **注意**：与yolov8 detect模型不同，分割模型的检测输出采用transposed格式（anchor-major），后处理代码已自动适配。

测试数据与yolov8 sample共用，通过软链接指向 `../yolov8/data/` 目录。

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

yolov8_seg demo中各部分参数位于 [config](./config/) 目录，结构如下所示：

```bash
./config/
├── decode.json                 # 解码配置
├── engine_group.json           # sophon-stream graph配置
├── yolov8_seg_demo.json        # demo输入配置文件
├── yolov8_group.json           # 简化的yolov8配置文件，将yolov8的前处理、推理、后处理合到一个配置文件中
└── osd.json                    # osd可视化配置
```

其中，[yolov8_seg_demo.json](./config/yolov8_seg_demo.json)是例程的整体配置文件，管理输入码流等信息。

Graph拓扑为 `decode → yolov8_group → osd → encode`，输出结果保存在 `./results` 目录下。

[yolov8_group.json](./config/yolov8_group.json)中配置task_type为"Seg"，模型使用FP32 BModel（推荐，FP32模型可输出更准确的置信度）：

```json
{
    "configure": {
        "model_path": "../yolov8_seg/data/models/BM1684X/yolov8s_fp32_1b.bmodel",
        "task_type": "Seg",
        "threshold_conf": 0.5,
        "threshold_nms": 0.5,
        "bgr2rgb": true,
        "mean": [0, 0, 0],
        "std": [255, 255, 255]
    },
    "shared_object": "../../build/lib/libyolov8.so",
    "name": "yolov8_group",
    "side": "sophgo",
    "thread_number": 4
}
```

[osd.json](./config/osd.json)中配置draw_utils为"OPENCV"以支持mask渲染，osd_type为"DET"以绘制检测框和分割掩码：

```json
{
    "configure": {
        "osd_type": "DET",
        "class_names_file": "../yolov8_seg/data/coco.names",
        "draw_utils": "OPENCV",
        "draw_interval": false,
        "put_text": true
    },
    "shared_object": "../../build/lib/libosd.so",
    "name": "osd",
    "side": "sophgo",
    "thread_number": 1
}
```

> **注意**：
> 1. 分割mask的渲染需要使用OPENCV draw_utils（BMCV模式不支持mask渲染）。
> 2. 当mSegmentedObjectMetadatas不为空时，osd会自动在检测框上叠加绘制分割掩码。
> 3. INT8模型在当前配置下可能产生较低的置信度，推荐使用FP32模型获得更好的分割效果。

### 6.2 运行

对于PCIe平台，可以直接在PCIe平台上运行测试；对于SoC平台，需将交叉编译生成的动态链接库、可执行文件、所需的模型和测试数据拷贝到SoC平台中测试。

1. 运行可执行文件
```bash
./main --demo_config_path=../yolov8_seg/config/yolov8_seg_demo.json
```

4路视频流运行结果如下
```bash
 total time cost 114256300 us.
frame count is 2848 | fps is 24.93 fps.
```

## 7. 性能测试

目前，yolov8_seg例程支持在BM1684X的PCIe、SoC模式下进行推理。

测试视频`test_car_person_1080P.avi`，编译选项为Release模式，测试yolov8 seg模型性能，结果如下:

|设备|路数|算法线程数|CPU利用率(%)|TPU利用率(%)|设备内存峰值(M)|平均FPS|
|----|----|-----|-----|-----|-----|---|
|SE7|4|4-4-4|-|-|-|24.93|

> **测试说明**：
> 1. 性能测试结果具有一定的波动性，建议多次测试取平均值；
> 2. 以上性能测试基于yolov8s_fp32_1b分割模型给出；
> 3. 分割后处理（CPU get_mask）及mask渲染（OpenCV）会占用额外的CPU时间，相比纯检测任务fps有所下降；
> 4. 上表中，输入路数和算法线程数的设置请参考[json配置说明](#61-json配置说明)，CPU利用率和系统内存使用top命令可查，TPU利用率和设备内存使用bm-smi命令可查，fps可以从运行程序打印的log中获得;
