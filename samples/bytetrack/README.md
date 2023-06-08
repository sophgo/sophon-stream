# ByteTrack

## 目录
- [ByteTrack](#bytetrack)
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
  - [6. json配置说明](#6-json配置说明)
  - [7. 推理测试](#7-推理测试)


## 1. 简介
ByteTrack是一个简单、快速、强大的多目标跟踪器，且不依赖特征提取模型。

**论文** (https://arxiv.org/abs/2110.06864)

**源代码** (https://github.com/ifzhang/ByteTrack)

## 2. 特性
* 支持BM1684X(x86 PCIe、SoC)和BM1684(x86 PCIe、SoC、arm PCIe)
* 支持检测模块和跟踪模块解藕，可适配各种检测器，本例程主要以YOLOX作为检测器
* 支持多路视频流
* 支持多线程

## 3. 准备模型与数据

​在`scripts`目录下提供了相关模型和数据的下载脚本`download.sh`。

```bash
# 安装unzip，若已安装请跳过，非ubuntu系统视情况使用yum或其他方式安装
sudo apt install unzip
chmod -R +x scripts/
./scripts/download.sh
```

下载的模型包括：
```
./models
├── BM1684
│   ├── yolox_s_fp32_1b.bmodel    # 用于BM1684的FP32 BModel，batch_size=1
│   ├── yolox_s_fp32_4b.bmodel    # 用于BM1684的FP32 BModel，batch_size=4
│   ├── yolox_s_int8_1b.bmodel    # 用于BM1684的INT8 BModel，batch_size=1
│   └── yolox_s_int8_4b.bmodel    # 用于BM1684的INT8 BModel，batch_size=4
├── BM1684X
│   ├── yolox_s_fp32_1b.bmodel    # 用于BM1684X的FP32 BModel，batch_size=1
│   ├── yolox_s_fp32_4b.bmodel    # 用于BM1684X的FP32 BModel，batch_size=4
│   ├── yolox_s_int8_1b.bmodel    # 用于BM1684X的INT8 BModel，batch_size=1batch_size=1
└── └── yolox_s_int8_4b.bmodel    # 用于BM1684X的INT8 BModel，batch_size=4batch_size=4
```
下载的数据包括：
```
./videos
└──  test_car_person_1080P.avi                 # 测试视频
```

## 4. 环境准备

### 4.1 x86/arm PCIe平台

如果您在x86/arm平台安装了PCIe加速卡（如SC系列加速卡），可以直接使用它作为开发环境和运行环境。您需要安装libsophon、sophon-opencv和sophon-ffmpeg，具体步骤可参考[x86-pcie平台的开发和运行环境搭建](../../docs/Environment_Install_Guide.md#3-x86-pcie平台的开发和运行环境搭建)或[arm-pcie平台的开发和运行环境搭建](../../docs/Environment_Install_Guide.md#5-arm-pcie平台的开发和运行环境搭建)。

### 4.2 SoC平台

如果您使用SoC平台（如SE、SM系列边缘设备），刷机后在`/opt/sophon/`下已经预装了相应的libsophon、sophon-opencv和sophon-ffmpeg运行库包，可直接使用它作为运行环境。通常还需要一台x86主机作为开发环境，用于交叉编译C++程序。

## 5. 程序编译
程序运行前需要编译可执行文件。
### 5.1 x86/arm PCIe平台
可以直接在PCIe平台上编译程序,具体请参考[how_to_make.md](../docs/how_to_make.md)

### 5.2 SoC平台
通常在x86主机上交叉编译程序，您需要在x86主机上使用SOPHON SDK搭建交叉编译环境，将程序所依赖的头文件和库文件打包至soc-sdk目录中，具体请参考[how_to_make.md](../docs/how_to_make.md)。本例程主要依赖libsophon、sophon-opencv和sophon-ffmpeg运行库包。

## 6. json配置说明
bytetrack usecase 中各部分参数由 json 来配置

```
./config
   ├── bytetrack.json    # bytetrack usecase 配置
   ├── bytetracker.json  # bytetrack目标跟踪器参数配置
   ├── decoder.json      # 解码配置
   ├── engine.json       # sophon-stream graph配置
   ├── infer.json        # 目标检测器推理配置
   ├── post.json         # 目标检测器后处理配置
   └── pre.json          # 目标检测器前处理配置
```

## 7. 推理测试
对于PCIe平台，可以直接在PCIe平台上推理测试；对于SoC平台，需将交叉编译生成的可执行文件及所需的模型、测试数据拷贝到SoC平台中测试。测试的参数及运行方式是一致的，下面主要以PCIe模式进行介绍。

```bash
./usecase_bytetrack
```

2路视频流运行结果如下
```
 total time cost 5316744 us.
frame count is 1422 | fps is 267.457 fps.
[       OK ] TestMultiAlgorithmGraph.MultiAlgorithmGraph (5317 ms)
[----------] 1 test from TestMultiAlgorithmGraph (5317 ms total)

[----------] Global test environment tear-down
[==========] 1 test from 1 test case ran. (5317 ms total)
[  PASSED  ] 1 test.
```

>**注意：**

soc环境运行时报错
```
./usecase_bytetrack: error while loading shared libraries: libframework.so: cannot open shared object file: No such file or directory
```

需要设置环境变量
```
export LD_LIBRARY_PATH=path-to/framework/build/:$LD_LIBRARY_PATH
```
