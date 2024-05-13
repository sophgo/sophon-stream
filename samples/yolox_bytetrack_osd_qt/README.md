# 目标跟踪算法结果显示Demo

[English](README_EN.md) | 简体中文

## 目录
- [目标跟踪算法结果显示Demo](#目标跟踪算法结果显示demo)
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

本例程用于说明如何使用sophon-stream快速构建视频目标跟踪应用，并将算法结果显示输出；

本例程插件的连接方式如下图所示:

![elements.jpg](pics/dec_det_track_osd_qt.jpg)

## 2. 特性
* 检测模型使用yolox；
* 跟踪模型使用bytetrack；
* 支持BM1684X(x86 PCIe、SoC)，BM1684(x86 PCIe、SoC、arm PCIe)，BM1688(SoC)
* 支持多路视频流
* 支持多线程
* 支持qt显示

## 3. 准备模型与数据

​在`scripts`目录下提供了相关模型和数据的下载脚本[download.sh](./scripts/download.sh)。

脚本执行完毕后，会在当前目录下生成`data`目录，其中包含`models`和`videos`两个子目录。

```bash
# 安装unzip，若已安装请跳过，非ubuntu系统视情况使用yum或其他方式安装
sudo apt install unzip
chmod -R +x scripts/
./scripts/download.sh
```

下载的模型包括：
```bash
./models/
├── BM1684
│   ├── yolox_bytetrack_s_fp32_1b.bmodel    # 用于BM1684的FP32 BModel，batch_size=1
│   ├── yolox_bytetrack_s_fp32_4b.bmodel    # 用于BM1684的FP32 BModel，batch_size=4
│   ├── yolox_bytetrack_s_int8_1b.bmodel    # 用于BM1684的INT8 BModel，batch_size=1
│   ├── yolox_bytetrack_s_int8_4b.bmodel    # 用于BM1684的INT8 BModel，batch_size=4
│   ├── yolox_s_fp32_1b.bmodel              # 用于BM1684的FP32 BModel，batch_size=1
│   ├── yolox_s_fp32_4b.bmodel              # 用于BM1684的FP32 BModel，batch_size=4
│   ├── yolox_s_int8_1b.bmodel              # 用于BM1684的INT8 BModel，batch_size=1
│   └── yolox_s_int8_4b.bmodel              # 用于BM1684的INT8 BModel，batch_size=4
├── BM1684X
│   ├── yolox_bytetrack_s_fp32_4b.bmodel    # 用于BM1684X的FP32 BModel，batch_size=4
│   ├── yolox_bytetrack_s_fp32_1b.bmodel    # 用于BM1684X的FP32 BModel，batch_size=1
│   ├── yolox_bytetrack_s_int8_1b.bmodel    # 用于BM1684X的INT8 BModel，batch_size=1
│   ├── yolox_bytetrack_s_int8_4b.bmodel    # 用于BM1684X的INT8 BModel，batch_size=4
│   ├── yolox_s_fp32_1b.bmodel              # 用于BM1684X的FP32 BModel，batch_size=1
│   ├── yolox_s_fp32_4b.bmodel              # 用于BM1684X的FP32 BModel，batch_size=4
│   ├── yolox_s_int8_1b.bmodel              # 用于BM1684X的INT8 BModel，batch_size=1
│   └── yolox_s_int8_4b.bmodel              # 用于BM1684X的INT8 BModel，batch_size=4
└── BM1688_2cores
    ├── yolox_s_int8_1b.bmodel              # 用于BM1688的INT8 BModel，batch_size=1
    └── yolox_s_int8_4b.bmodel              # 用于BM1688的INT8 BModel，batch_size=4
```
模型说明:

1.`yolox_bytetrack_s`系列模型移植于[bytetrack官方](https://github.com/ifzhang/ByteTrack)，插件配置`mean=[0,0,0]`，`std=[255,255,255]`，支持person类别的检测任务。

2.`yolox_s`系列模型移植于[yolox官方](https://github.com/Megvii-BaseDetection/YOLOX)，插件配置`mean=[0,0,0]`，`std=[1,1,1]`，支持COCO数据集的80分类检测任务。

下载的数据包括：
```bash
./data/videos                             # 测试视频
├── mot17_01_frcnn.mp4
├── mot17_03_frcnn.mp4
├── mot17_06_frcnn.mp4
├── mot17_07_frcnn.mp4
├── mot17_08_frcnn.mp4
├── mot17_12_frcnn.mp4
├── mot17_14_frcnn.mp4
└── sample_1080p_h265.mp4
```

## 4. 环境准备

### 4.1 x86/arm PCIe平台

如果您在x86/arm平台安装了PCIe加速卡（如SC系列加速卡），可以直接使用它作为开发环境和运行环境。您需要安装libsophon、sophon-opencv和sophon-ffmpeg，具体步骤可参考[x86-pcie平台的开发和运行环境搭建](../../docs/EnvironmentInstallGuide.md#3-x86-pcie平台的开发和运行环境搭建)或[arm-pcie平台的开发和运行环境搭建](../../docs/EnvironmentInstallGuide.md#5-arm-pcie平台的开发和运行环境搭建)。

除此之外还需要安装公版QT：

```bash
sudo apt install qtbase5-dev
```

### 4.2 SoC平台

如果您使用SoC平台（如SE、SM系列边缘设备），刷机后在`/opt/sophon/`下已经预装了相应的libsophon、sophon-opencv和sophon-ffmpeg运行库包，可直接使用它作为运行环境。通常还需要一台x86主机作为开发环境，用于交叉编译C++程序。

#### 4.2.1 BM1684/BM1684X
如果您使用的是BM1684/BM1684X设备，您需要在您进行交叉编译的设备上，通过以下命令下载并解压sophon-qt以进行后续的交叉编译：

```bash
python3 -m dfss --url=open@sophgo.com:sophon-demo/MultiYolov5/qt-5.14-amd64-aarch64-fl2000fb_v1.1.0.tar.xz
tar -zxvf lib_1011.tar.gz
```
#### 4.2.2 BM1688
如果您使用的是BM1688设备，您需要在您进行交叉编译的设备上，通过以下命令下载并解压arm的公版qt以进行后续的交叉编译：
```bash
python3 -m dfss --url=open@sophgo.com:sophon-pipeline/a2_bringup/qtbase.zip
unzip qtbase.zip
```

## 5. 程序编译
程序运行前需要编译可执行文件。
### 5.1 x86/arm PCIe平台
可以直接在PCIe平台上编译程序，具体请参考[sophon-stream编译](../../docs/HowToMake.md)

### 5.2 SoC平台
通常在x86主机上交叉编译程序，您需要在x86主机上使用SOPHON SDK搭建交叉编译环境，将程序所依赖的头文件和库文件打包至sophon_sdk_soc目录中，具体请参考[sophon-stream编译](../../docs/HowToMake.md)。本例程主要依赖libsophon、sophon-opencv和sophon-ffmpeg运行库包。

## 6. 程序运行

### 6.1 Json配置说明

配置文件位于 [./config](../yolox_bytetrack_osd_qt/config)

其中，[yolox_bytetrack_osd_qt_demo.json](../yolox_bytetrack_osd_qt/config/yolox_bytetrack_osd_qt_demo.json)是例程的整体配置文件，管理输入码流等信息。在一张图上可以支持多路数据的输入，channels中包含每一路码流url等信息。

```json
{
  "channels": [
    {
      "channel_id": 2,
      "url": "../yolox_bytetrack_osd_qt/data/videos/mot17_01_frcnn.mp4",
      "source_type": "VIDEO",
      "loop_num": 1
    },
    {
      "channel_id": 3,
      "url": "../yolox_bytetrack_osd_qt/data/videos/mot17_03_frcnn.mp4",
      "source_type": "VIDEO",
      "loop_num": 1
    },
    {
      "channel_id": 20,
      "url": "../yolox_bytetrack_osd_qt/data/videos/mot17_06_frcnn.mp4",
      "source_type": "VIDEO",
      "loop_num": 1
    },
    {
      "channel_id": 30,
      "url": "../yolox_bytetrack_osd_qt/data/videos/mot17_08_frcnn.mp4",
      "source_type": "VIDEO",
      "loop_num": 1
    }
  ],
  "engine_config_path": "../yolox_bytetrack_osd_qt/config/engine_group.json"
}
```

[engine.json](../yolox_bytetrack_osd_qt/config/engine.json) 包含对graph的配置信息，这部分配置确定之后基本不会发生更改。

需要注意，部署环境下的NPU等设备内存大小会显著影响例程运行的路数。如果默认的输入路数运行中出现了申请内存失败等错误，可以考虑把输入路数减少，即删去`channels`里的部分元素，再进行测试。

这里摘取配置文件的一部分作为示例：在该文件内，需要初始化每个element的信息和element之间的连接方式。element_id是唯一的，起到标识身份的作用。element_config指向该element的详细配置文件地址，port_id是该element的输入输出端口编号，多输入或多输出的情况下，输入/输出编号也不可以重复。is_src标志当前端口是否是整张图的输入端口，is_sink标识当前端口是否是整张图的输出端口。
connection是所有element之间的连接方式，通过element_id和port_id确定。

```json
[
    {
        "graph_id": 0,
        "device_id": 0,
        "graph_name": "yolox_osd_qt_display",
        "elements": [
            {
                "element_id": 5000,
                "element_config": "../yolox_bytetrack_osd_qt/config/decode.json",
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
                "element_config": "../yolox_bytetrack_osd_qt/config/yolox_group.json",
                "inner_elements_id": [10001, 10002, 10003]
            },
            {
                "element_id": 5004,
                "element_config": "../yolox_bytetrack_osd_qt/config/bytetrack.json"
            },
            {
                "element_id": 5005,
                "element_config": "../yolox_bytetrack_osd_qt/config/osd.json"
            },
            {
                "element_id": 5006,
                "element_config": "../yolox_bytetrack_osd_qt/config/qt_display.json",
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
            },
            {
                "src_element_id": 5001,
                "src_port": 0,
                "dst_element_id": 5004,
                "dst_port": 0
            },
            {
                "src_element_id": 5004,
                "src_port": 0,
                "dst_element_id": 5005,
                "dst_port": 0
            },
            {
                "src_element_id": 5005,
                "src_port": 0,
                "dst_element_id": 5006,
                "dst_port": 0
            }
        ]
    }
]
```

### 6.2 运行
对于PCIe平台，可以直接在PCIe平台上运行测试；对于SoC平台，需将交叉编译生成的动态链接库、可执行文件、所需的模型和测试数据拷贝到SoC平台中测试。

SoC平台上，动态库、可执行文件、配置文件、模型、视频数据的目录结构关系应与原始sophon-stream仓库中的关系保持一致。

PCIE模式下运行可执行文件
```bash
sudo ./main --demo_config_path=../yolox_bytetrack_osd_qt/config/yolox_bytetrack_osd_qt_demo.json
```

SoC模式下，如果桌面程序正在运行，需要先停止服务 
```bash
sudo systemctl stop SophonHDMI.service
```

然后在scripts目录下运行run_hdmi_show.sh脚本
```bash
cd scripts
sudo ./run_hdmi_show.sh
```

运行结果如下
```bash
total time cost 60697616 us.
frame count is 3773 | fps is 62.1606 fps.
```

## 7. 性能测试
由于Osd插件画图速度慢，本例程暂不提供性能测试结果，如需各模型推理性能，请到对应模型例程查看。