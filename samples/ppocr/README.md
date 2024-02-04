# PP-OCR Demo
[English](README_EN.md) | 简体中文
## 目录
- [PP-OCR Demo](#PP-OCR-demo)
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

PP-OCRv3，是百度飞桨团队开源的超轻量OCR系列模型，包含文本检测、文本分类、文本识别模型，是PaddleOCR工具库的重要组成之一。支持中英文数字组合识别、竖排文本识别、长文本识别，其性能及精度较之前的PP-OCR版本均有明显提升。本例程对[PaddleOCR-release-2.6](https://github.com/PaddlePaddle/PaddleOCR/tree/release/2.6)的`ch_PP-OCRv3_xx`系列模型和算法进行移植，使之能在SOPHON BM1684/BM1684X/BM1688上进行推理测试。


## 2. 特性
* 支持BM1684、BM1684X(x86 PCIe、SoC)，BM1688(SoC)
* 支持FP32、FP16模型编译和推理
* 支持图片数据集测试

## 3. 准备模型与数据

​在`scripts`目录下提供了相关模型和数据的下载脚本 [download.sh](./scripts/download.sh)。

```bash
# 安装unzip，若已安装请跳过，非ubuntu系统视情况使用yum或其他方式安装
sudo apt install unzip
chmod -R +x scripts/
./scripts/download.sh
```

脚本执行完毕后，会在当前目录下生成`data`目录，其中包含`models`、`datasets`

下载的模型包括：

```bash
├── BM1684
│   ├── ch_PP-OCRv3_det_fp32_1b.bmodel
│   ├── ch_PP-OCRv3_rec_fp32_1b_320.bmodel
│   └── ch_PP-OCRv3_rec_fp32_1b_640.bmodel
├── BM1684X
│   ├── ch_PP-OCRv3_det_fp16_1b.bmodel
│   ├── ch_PP-OCRv3_det_fp32_1b.bmodel
│   ├── ch_PP-OCRv3_rec_fp16_1b_320.bmodel
│   ├── ch_PP-OCRv3_rec_fp16_1b_640.bmodel
│   ├── ch_PP-OCRv3_rec_fp32_1b_320.bmodel
│   └── ch_PP-OCRv3_rec_fp32_1b_640.bmodel
└── BM1688
    ├── ch_PP-OCRv3_det_fp16_1b.bmodel
    ├── ch_PP-OCRv3_det_fp32_1b.bmodel
    ├── ch_PP-OCRv3_rec_fp16_1b_320.bmodel
    ├── ch_PP-OCRv3_rec_fp16_1b_640.bmodel
    ├── ch_PP-OCRv3_rec_fp32_1b_320.bmodel
    └── ch_PP-OCRv3_rec_fp32_1b_640.bmodel
```

下载的数据包括：
```bash
data
├── class.names
├── datasets
│   ├── ppocr_keys_v1.txt
│   ├── train_full_images_0
│   └── train_full_images_0.json
├── models
└── wqy-microhei.ttc

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

ppocr demo中各部分参数位于 [config](./config/) 目录，结构如下所示：

```bash
./config
├── converger.json                # 汇聚插件配置
├── decode.json                   # 解码插件配置
├── distributor_frame_class.json  # 分发插件配置
├── engine_group.json             # sophon-stream graph配置
├── ppocr_demo.json               # ppocr demo配置
├── ppocr_det_group.json          # 检测插件配置
└── ppocr_rec_group.json          # 识别插件配置
```

其中，[ppocr_demo.json](./config/ppocr_demo.json)是例程的整体配置文件，管理输入码流等信息。在一张图上可以支持多路数据的输入，channels参数配置输入的路数，channel中包含码流url等信息。

配置文件中不指定`channel_id`属性的情况，会在demo中对每一路数据的`channel_id`从0开始默认赋值。

```json
{
  "channels": [
    {
      "channel_id": 2,
      "url": "../ppocr/data/train_full_images_0",
      "source_type": "IMG_DIR",
      "sample_interval": 1,
      "loop_num": 1,
      "fps": -1
    },
    {
      "channel_id": 3,
      "url": "../ppocr/data/train_full_images_0",
      "source_type": "IMG_DIR",
      "sample_interval": 1,
      "loop_num": 1,
      "fps": -1
    },
    {
      "channel_id": 20,
      "url": "../ppocr/data/train_full_images_0",
      "source_type": "IMG_DIR",
      "sample_interval": 1,
      "loop_num": 1,
      "fps": -1
    },
    {
      "channel_id": 30,
      "url": "../ppocr/data/train_full_images_0",
      "source_type": "IMG_DIR",
      "sample_interval": 1,
      "loop_num": 1,
      "fps": -1
    }
  ],
  "engine_config_path": "../ppocr/config/engine_group.json",
  "draw_func_name": "draw_ppocr_results",
  "download_image": true
}
```

[engine_group.json](./config/engine_group.json) 包含对graph的配置信息，这部分配置确定之后基本不会发生更改。

这里摘取配置文件的一部分作为示例：在该文件内，需要初始化每个element的信息和element之间的连接方式。element_id是唯一的，起到标识身份的作用。element_config指向该element的详细配置文件地址，port_id是该element的输入输出端口编号，多输入或多输出的情况下，输入/输出编号也不可以重复。is_src标志当前端口是否是整张图的输入端口，is_sink标识当前端口是否是整张图的输出端口。
connection是所有element之间的连接方式，通过element_id和port_id确定。

```json
[
    {
        "graph_id": 0,
        "device_id": 1,
        "graph_name": "ppocr",
        "elements": [
            {
                "element_id": 5000,
                "element_config": "../ppocr/config/decode.json",
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
                "element_config": "../ppocr/config/ppocr_det_group.json",
                "inner_elements_id": [10001, 10002, 10003]
            },
            {
                "element_id": 5004,
                "element_config": "../ppocr/config/distributor_frame_class.json"
            },
            {
                "element_id": 6001,
                "element_config": "../ppocr/config/ppocr_rec_group.json",
                "inner_elements_id": [20001, 20002, 20003]
            },
            {
                "element_id": 5005,
                "element_config": "../ppocr/config/converger.json",
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
                "src_element_id": 5004,
                "src_port": 1,
                "dst_element_id": 6001,
                "dst_port": 0
            },
            {
                "src_element_id": 6001,
                "src_port": 0,
                "dst_element_id": 5005,
                "dst_port": 1
            }


        ]
    }
]
```

### 6.2 运行

对于PCIe平台，可以直接在PCIe平台上运行测试；对于SoC平台，需将交叉编译生成的动态链接库、可执行文件、所需的模型和测试数据拷贝到SoC平台中测试。

SoC平台上，动态库、可执行文件、配置文件、模型、视频数据的目录结构关系应与原始sophon-stream仓库中的关系保持一致。

测试的参数及运行方式是一致的，下面主要以PCIe模式进行介绍。

1. 运行可执行文件
```bash
./main --demo_config_path=../ppocr/config/ppocr_demo.json
```

2路视频流运行结果如下
```bash
 total time cost 376103640 us.
frame count is 10560 | fps is 28.0774 fps.
```

## 7. 性能测试
目前，ppocr例程支持在BM1684X和BM1684的PCIE、SOC模式下进行推理，支持BM1688 SOC模式下推理。

在不同的设备上可能需要修改json配置，例如模型路径、输入路数等。json的配置方法参考6.1节，程序运行方法参考上文6.2节。

由于PCIE设备cpu能力差距较大，性能数据没有参考意义，这里只给出SOC模式的测试结果。

测试图片集train_full_images_0，编译选项为Release模式，结果如下:

|设备|模型|路数|算法线程数|CPU利用率(%)|系统内存(M)|TPU利用率(%)|设备内存(M)|平均FPS|
|----|----|----|-----|-----|-----|-----|-----|-----|
|SE5-16|fp32|1|1-1-1|227.8|410.1|90|1066|23.14|
|SE7|fp32|1|1-1-1|313.6|438.0|88|1085|38.67|
|SE7|fp16|2|2-2-2|519.6|445.1 |52|1046|66.96|



> **测试说明**：
1. 性能测试结果具有一定的波动性，建议多次测试取平均值；
2. BM1684/1684X SoC的主控CPU均为8核 ARM A53 42320 DMIPS @2.3GHz；
3. 上表中，输入路数和算法线程数的设置请参考[json配置说明](#61-json配置说明)，CPU利用率和系统内存使用top命令可查，TPU利用率和设备内存使用bm-smi命令可查，fps可以从运行程序打印的log中获得;
4. BM1688设备暂不支持性能测试。
5. 这个测试数据中，SE5 sdk版本为0.4.8，SE7 sdk版本为0.4.9，不同版本测试结果可能有差异。
