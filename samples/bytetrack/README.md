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
  - [5. Json配置说明](#5-json配置说明)
  - [6. 程序编译](#6-程序编译)
    - [6.1 x86/arm PCIe平台](#61-x86arm-pcie平台)
    - [6.2 SoC平台](#62-soc平台)
  - [7. 运行测试](#7-运行测试)
  - [8. 性能测试](#8-性能测试)


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

​在`scripts`目录下提供了相关模型和数据的下载脚本[download.sh](./scripts/download.sh)。

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
```bash
./videos
└──  test_car_person_1080P.avi                 # 测试视频
```

## 4. 环境准备

### 4.1 x86/arm PCIe平台

如果您在x86/arm平台安装了PCIe加速卡（如SC系列加速卡），可以直接使用它作为开发环境和运行环境。您需要安装libsophon、sophon-opencv和sophon-ffmpeg，具体步骤可参考[x86-pcie平台的开发和运行环境搭建](../../docs/Environment_Install_Guide.md#3-x86-pcie平台的开发和运行环境搭建)或[arm-pcie平台的开发和运行环境搭建](../../docs/Environment_Install_Guide.md#5-arm-pcie平台的开发和运行环境搭建)。

### 4.2 SoC平台

如果您使用SoC平台（如SE、SM系列边缘设备），刷机后在`/opt/sophon/`下已经预装了相应的libsophon、sophon-opencv和sophon-ffmpeg运行库包，可直接使用它作为运行环境。通常还需要一台x86主机作为开发环境，用于交叉编译C++程序。

## 5. Json配置说明
bytetrack usecase 中各部分参数位于[config](../bytetrack/config/)目录，结构如下所示

```bash
./config
   ├── usecase_bytetrack.json    # bytetrack usecase 配置
   ├── bytetrack.json            # bytetrack目标跟踪器参数配置
   ├── decoder.json              # 解码配置
   ├── engine.json               # sophon-stream graph配置
   ├── infer.json                # 目标检测器推理配置
   ├── post.json                 # 目标检测器后处理配置
   └── pre.json                  # 目标检测器前处理配置
```

其中, [usecase_bytetrack.json](../bytetrack/config/usecase_bytetrack.json)是例程的整体配置文件, 管理输入码流等信息。在一张图上可以支持多路数据的输入, num_channels_per_graph参数配置输入的路数, channel中包含码流url等信息。
```json
{
    "num_channels_per_graph": 1,
    "channel": {
      "url": "../data/videos/test_car_person_1080P.avi",
      "source_type": 0
    },
    "download_image": false,
    "engine_config_path": "../config/engine.json"
  }
```
[engine.json](../bytetrack/config/engine.json)包含对每一张graph的配置信息。这里摘取一部分作为示例：在一张图内, 需要初始化每个element的信息和element之间的连接方式。element_id是唯一的, 起到标识身份的作用。element_config指向该element的详细配置文件地址, port_id是该element的输入输出端口编号, 多输入或多输出的情况下, 输入/输出编号也不可以重复。is_src标志当前端口是否是整张图的输入端口, is_sink标识当前端口是否是整张图的输出端口。
connection是所有element之间的连接方式, 通过element_id和port_id确定。
```json
{
    "graph_id": 0,
    "graph_name": "bytetrack",
    "elements": [
        {
            "element_id": 5000,
            "element_config": "../config/decode.json",
            "ports": {
                "input": [
                    {
                        "port_id": 0,
                        "is_sink": false,
                        "is_src": true
                    }
                ],
                "output": [
                    {
                        "port_id": 0,
                        "is_sink": false,
                        "is_src": false
                    }
                ]
            }
        },
        {
            "element_id": 5001,
            "element_config": "../config/yolox_pre.json",
            "ports": {
                "input": [
                    {
                        "port_id": 0,
                        "is_sink": false,
                        "is_src": false
                    }
                ],
                "output": [
                    {
                        "port_id": 0,
                        "is_sink": false,
                        "is_src": false
                    }
                ]
            }
        },
        {
            "element_id": 5002,
            "element_config": "../config/yolox_infer.json",
            "ports": {
                "input": [
                    {
                        "port_id": 0,
                        "is_sink": false,
                        "is_src": false
                    }
                ],
                "output": [
                    {
                        "port_id": 0,
                        "is_sink": false,
                        "is_src": false
                    }
                ]
            }
        },
        {
            "element_id": 5003,
            "element_config": "../config/yolox_post.json",
            "ports": {
                "input": [
                    {
                        "port_id": 0,
                        "is_sink": false,
                        "is_src": false
                    }
                ],
                "output": [
                    {
                        "port_id": 0,
                        "is_sink": false,
                        "is_src": false
                    }
                ]
            }
        },
        {
            "element_id": 5004,
            "element_config": "../config/bytetrack.json",
            "ports": {
                "input": [
                    {
                        "port_id": 0,
                        "is_sink": false,
                        "is_src": false
                    }
                ],
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
            "dst_element_id": 5002,
            "dst_port": 0
        },
        {
            "src_element_id": 5002,
            "src_port": 0,
            "dst_element_id": 5003,
            "dst_port": 0
        },
        {
            "src_element_id": 5003,
            "src_port": 0,
            "dst_element_id": 5004,
            "dst_port": 0
        }
    ]
}
```
[bytetrack.json](../bytetrack/config/bytetrack.json)等配置文件是对具体某个element的配置细节, 设置了模型参数、动态库路径、阈值等信息。
其中, thread_number是element内部的工作线程数量, 一个线程会对应一个数据队列, 多路输入情况下, 需要合理设置数据队列数目, 来保证线程工作压力均匀且合理。
```json
{
    "configure": {
        "track_thresh": 0.6,
        "high_thresh": 0.7,
        "match_thresh": 0.8,
        "frame_rate": 30,
        "track_buffer": 30
    },
    "shared_object": "../../../element/algorithm/bytetrack/build/libbytetrack.so",
    "device_id": 0,
    "id": 0,
    "name": "bytetrack",
    "side": "sophgo",
    "thread_number": 2
}
```

## 6. 程序编译
程序运行前需要编译可执行文件。
### 6.1 x86/arm PCIe平台
可以直接在PCIe平台上编译程序,具体请参考[sophon-stream编译](../docs/how_to_make.md)

### 6.2 SoC平台
通常在x86主机上交叉编译程序，您需要在x86主机上使用SOPHON SDK搭建交叉编译环境，将程序所依赖的头文件和库文件打包至soc-sdk目录中，具体请参考[sophon-stream编译](../docs/how_to_make.md)。本例程主要依赖libsophon、sophon-opencv和sophon-ffmpeg运行库包。

## 7. 运行测试
对于PCIe平台，可以直接在PCIe平台上运行测试；对于SoC平台，需将交叉编译生成的可执行文件及所需的模型、测试数据拷贝到SoC平台中测试。测试的参数及运行方式是一致的，下面主要以PCIe模式进行介绍。

运行可执行文件
```bash
./usecase_bytetrack
```

2路视频流运行结果如下
```bash
 total time cost 5316744 us.
frame count is 1422 | fps is 267.457 fps.
[       OK ] TestMultiAlgorithmGraph.MultiAlgorithmGraph (5317 ms)
[----------] 1 test from TestMultiAlgorithmGraph (5317 ms total)

[----------] Global test environment tear-down
[==========] 1 test from 1 test case ran. (5317 ms total)
[  PASSED  ] 1 test.
```

>**注意：**

soc环境运行时如果报错
```bash
./usecase_bytetrack: error while loading shared libraries: libframework.so: cannot open shared object file: No such file or directory
```

需要设置环境变量
```bash
export LD_LIBRARY_PATH=path-to/framework/build/:$LD_LIBRARY_PATH
```

##  8. 性能测试

在不同的测试平台上，使用不同数量的graph、视频通道、element线程，测试视频`data/videos/test_car_person_1080P.avi`，性能测试结果如下：

|    测试平台    | num_graph | num_channels_per_graph | num_channels | num_threads_pre | num_threads_infer | num_threads_post |num_threads_tracker | frame_count | fps | tpu | mem |
| ------------ | --------- | ---------------------- | ------------ | --------------- | ----------------- | ----------------- | ------------------ | ----------- | --- | --- | ---|
| BM1684X PCIE | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 711/711 | 237.315 | 80+ | 430-450MB |
| BM1684X PCIE | 1 | 3 | 3 | 1 | 1 | 1 | 3 | 2133/2133|	161.98 | 40+	|580-700MB|
| BM1684X PCIE | 1 | 3 | 3 | 2 | 2 | 2 | 3 | 2131-2133/2133	|252.241	| 90-100 |700-900MB |
| BM1684X PCIE | 1 | 9 | 9 | 3 | 3 | 3 | 9 | 6366-6384/6399	|298.531	| 100	 | 1200-1500MB |
| BM1684X PCIE | 1 | 12 | 12 | 4 | 4 | 4 | 12 | 8460/8532 |	299.585 |	100	 | 1900-2150MB |
| BM1684X SOC  | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 711/711 | 110.036	| 60-85	| 430-500MB |
| BM1684X SOC  | 1 | 3 | 3 | 1 | 1 | 1 | 3 | 2131-2133/2133 |	122.29	| 40-55	| 550-700MB |
| BM1684X SOC  | 1 | 3 | 3 | 2 | 2 | 2 | 3 | 2131-2133/2133	| 194.788	| 80-95	| 700-1000MB |
| BM1684X SOC  | 1 | 9 | 9 | 3 | 3 | 3 | 9 | 6382-6384/6399	| 232.303	| 80-100| 1300-1650MB |
| BM1684X SOC  | 1 | 12 | 12 | 4 | 4 | 4 | 12 | 8465/8532 |	244.878	| 90-100	| 1900-2200MB |
| BM1684 PCIE  | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 7711/711	| 100.858 |	60-80	| 500-620MB |
| BM1684 PCIE  | 1 | 3 | 3 | 1 | 1 | 1 | 3 | 2133/2133 |	98.758 |	60-80	| 600-750MB |
| BM1684 PCIE  | 1 | 3 | 3 | 2 | 2 | 2 | 3 | 2133/2133 |115.264 |	80-90	| 700-900MB  |
| BM1684 PCIE  | 1 | 6 | 6 | 2 | 2 | 2 | 6 | 4264-4265/4266	| 123.323 |	86-100	| 1000-1100MB |
| BM1684 PCIE  | 1 | 6 | 6 | 3 | 3 | 3 | 6 | 4264-4265/4266	| 129.433	| 90-100	| 1100-1250MB |
| BM1684 SOC   | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 710-711/711	| 90.375	| 60-80	| 300-500MB |
| BM1684 SOC   | 1 | 3 | 3 | 1 | 1 | 1 | 3 | 2133/2133	| 95.464	| 60-90	| 600-700MB |
| BM1684 SOC   | 1 | 3 | 3 | 2 | 2 | 2 | 3 | 2133/2133	| 110.264	| 70-90	| 700-950MB |
| BM1684 SOC   | 1 | 6 | 6 | 2 | 2 | 2 | 6 | 4260-4263/4266	| 118.125	| 80-90	| 1000-1100MB |
| BM1684 SOC   | 1 | 6 | 6 | 3 | 3 | 3 | 6 | 4261-4266/4266	| 126.039	| 80-90	| 1100-1300MB |

> **测试说明**：
1. tpu占用率单位为(%)；
2. 性能测试结果具有一定的波动性，建议多次测试取平均值；
3. BM1684/1684X SoC的主控CPU均为8核 ARM A53 42320 DMIPS @2.3GHz，PCIe上的性能由于CPU的不同可能存在较大差异；