# ResNet Demo

English | [简体中文](README.md)

## Catalogs
- [ResNet Demo](#resnet-demo)
  - [Catalogs](#catalogs)
  - [1. Introduction](#1-introduction)
  - [2. Features](#2-features)
  - [3. Prepare Models and Data](#3-prepare-models-and-data)
  - [4. Prepare Environment](#4-prepare-environment)
    - [4.1 x86/arm PCIe Platform](#41-x86arm-pcie-platform)
    - [4.2 SoC Platform](#42-soc-platform)
  - [5. Program Compilation](#5-program-pompilation)
    - [5.1 x86/arm PCIe Platform](#51-x86arm-pcie-platform)
    - [5.2 SoC Platform](#52-soc-platform)
  - [6. Program Execution](#6-program-execution)
    - [6.1 JSON Configuration](#61-json-configuration)
    - [6.2 Execute](#62-execute)
  - [7. Performance Testing](#7-performance-testing)

## 1. Introduction

This example demonstrates how to use sophon-stream to quickly build a video object classification application.

Deep residual network (ResNet) is a deep neural network architecture due to Kaiming He et al. in 2015, which utilizes residual learning to solve the problem of deep neural network training degradation.

The contributions of Kaiming He, Xiangyu Zhang, Shaoqing Ren, Jian Sun, and others are greatly appreciated.

**Paper** (https://arxiv.org/abs/1512.03385)

## 2. Features

* Supports BM1684X(x86 PCIe、SoC) and BM1684(x86 PCIe、SoC、arm PCIe)
* Supports FP32, FP16(BM1684X) and INT8 models compilation and inference.
* Supports multiple video streams.
* Supports multi-threading.

## 3. Prepare Models and Data

The `scripts` directory contains download scripts for relevant models and data. [download.sh](./scripts/download.sh).

```bash
# Install unzip. Skip this step if already installed. If not Ubuntu systems, use yum or other methods as needed.
sudo apt install unzip
chmod -R +x scripts/
./scripts/download.sh
```

After the script execution, `data` directory will be generated in the current directory, containing three subdirectories: `models`, `videos` and `images`

The downloaded models include:

```bash
models/
├── BM1684
│   ├── resnet_pedestrian_gender_fp32_1b.bmodel    # FP32 BModel for BM1684，batch_size=1
│   ├── resnet_pedestrian_gender_fp32_4b.bmodel    # FP32 BModel for BM1684，batch_size=4
│   ├── resnet_pedestrian_gender_int8_1b.bmodel    # INT8 BModel for BM1684，batch_size=1
│   ├── resnet_pedestrian_gender_int8_4b.bmodel    # INT8 BModel for BM1684，batch_size=4
│   ├── resnet_vehicle_color_fp32_1b.bmodel        # FP32 BModel for BM1684，batch_size=1
│   ├── resnet_vehicle_color_fp32_4b.bmodel        # FP32 BModel for BM1684，batch_size=4
│   ├── resnet_vehicle_color_int8_1b.bmodel        # INT8 BModel for BM1684，batch_size=1
│   ├── resnet_vehicle_color_int8_4b.bmodel        # INT8 BModel for BM1684，batch_size=4
│   ├── resnet50_fp32_1b.bmodel                    # FP32 BModel for BM1684，batch_size=1
│   ├── resnet50_fp32_4b.bmodel                    # FP32 BModel for BM1684，batch_size=4
│   ├── resnet50_int8_1b.bmodel                    # INT8 BModel for BM1684，batch_size=1
│   └── resnet50_int8_4b.bmodel                    # INT8 BModel for BM1684，batch_size=4
└── BM1684X
    ├── resnet_pedestrian_gender_fp32_1b.bmodel    # FP32 BModel for BM1684X，batch_size=1
    ├── resnet_pedestrian_gender_fp32_4b.bmodel    # FP32 BModel for BM1684X，batch_size=4
    ├── resnet_pedestrian_gender_fp16_1b.bmodel    # FP16 BModel for BM1684X，batch_size=1
    ├── resnet_pedestrian_gender_int8_1b.bmodel    # INT8 BModel for BM1684X，batch_size=1
    ├── resnet_pedestrian_gender_int8_4b.bmodel    # INT8 BModel for BM1684X，batch_size=4
    ├── resnet_vehicle_color_fp32_1b.bmodel        # FP32 BModel for BM1684X，batch_size=1
    ├── resnet_vehicle_color_fp32_4b.bmodel        # FP32 BModel for BM1684X，batch_size=4
    ├── resnet_vehicle_color_fp16_1b.bmodel        # FP16 BModel for BM1684X，batch_size=1
    ├── resnet_vehicle_color_int8_1b.bmodel        # INT8 BModel for BM1684X，batch_size=1
    └── resnet_vehicle_color_int8_4b.bmodel        # INT8 BModel for BM1684X，batch_size=4
    ├── resnet50_fp32_1b.bmodel                    # FP32 BModel for BM1684X，batch_size=1
    ├── resnet50_fp32_4b.bmodel                    # FP32 BModel for BM1684X，batch_size=4
    ├── resnet50_fp16_1b.bmodel                    # FP16 BModel for BM1684X，batch_size=1
    ├── resnet50_int8_1b.bmodel                    # INT8 BModel for BM1684X，batch_size=1
    └── resnet50_int8_4b.bmodel                    # INT8 BModel for BM1684X，batch_size=4
```

The downloaded data include:

```bash
data/
├── images
│   ├── imagenet_val_1k                    # imagenet test images, total 1000 images
│   ├── pedestrian_gender                  # pedestrian sex classification test pictures
│   └── vehicle_color                      # vehicle color classification test pictures
└── video
    ├── test_imagenet.mp4                  # imagenet test video
    └── test_vehicle_color.mp4             # vehicle color classification test video
```

## 4. Prepare Environment

### 4.1 x86/arm PCIe Platform

If you have installed a PCIe accelerator card (such as the SC series card) on an x86/arm platform, you can directly use it as the development or runtime environment. You need to install libsophon, sophon-opencv, and sophon-ffmpeg. For specific steps, please refer to [the setup guide for x86-pcie platform](../../docs/EnvironmentInstallGuide_EN.md#3-x86-pcie-platform-development-and-runtime-environment-construction) or [setup guide for arm-pcie platform](../../docs/EnvironmentInstallGuide_EN.md#5-arm-pcie-platform-development-and-runtime-environment-construction).

### 4.2 SoC Platform

If you are using the SoC platform (such as SE or SM series edge devices), after flashing(Upgrade the operating system by SD card.), the corresponding libsophon, sophon-opencv, and sophon-ffmpeg runtime library packages are pre-installed under `/opt/sophon/`, which can be directly used as the runtime environment. Typically, you would also need an x86 machine as the development environment for cross-compiling C++ programs.

## 5. Program Compilation

### 5.1 x86/arm PCIe Platform
You can directly compile programs on the PCIe platform. For specifics, please refer to [sophon-stream compilation](../../docs/HowToMake_EN.md).

### 5.2 SoC Platform
Typically, programs are cross-compiled on an x86 computer. You need to set up a cross-compilation environment using SOPHON SDK on the x86 computer. Package the necessary include files and library files for the program into the `sophon_sdk_soc` directory. For specifics, please refer to [sophon-stream compilation](../../docs/HowToMake_EN.md). This example mainly dependes on the libsophon, sophon-opencv, and sophon-ffmpeg runtime library packages.

## 6. Program Execution

### 6.1 JSON Configuration

In the ResNet demo, various parameters for each section are located in [config](./config/) directory, structured as follows:

```bash
./config
├── decode.json             # decoding configuration
├── engine.json             # sophon-stream graph configuration
├── resnet_demo.json        # resnet demo configuration
├── resnet_roi.json         # resnet roi configuration
└── resnet.json             # resnet plugin configuration
```

Indeed, [resnet_demo.json](./config/resnet_demo.json) is the overall configuration file for the example, managing input streams and other information. Multiple data inputs can be supported on a single graph, where the `channels` parameter configures the number of input channels, `sample_interval` sets the frame skipping rate, and `loop_num` sets the number of looped plays. The `channel` section contains video stream information such as the URL. 

In the configuration file, when the `channel_id` attribute is not specified, the demo will assign default `channel_id` values starting from 0 for each data channel.

```json
{
  "channels": [
    {
      "channel_id": 2,
      "url": "../resnet/data/images/imagenet_val_1k",
      "source_type": "IMG_DIR",
      "loop_num": 1,
      "sample_interval": 1,
      "fps": -1
    },
    {
      "channel_id": 3,
      "url": "../resnet/data/images/imagenet_val_1k",
      "source_type": "IMG_DIR",
      "loop_num": 1,
      "sample_interval": 1,
      "fps": -1
    },
    {
      "channel_id": 20,
      "url": "../resnet/data/images/imagenet_val_1k",
      "source_type": "IMG_DIR",
      "loop_num": 1,
      "sample_interval": 1,
      "fps": -1
    },
    {
      "channel_id": 30,
      "url": "../resnet/data/images/imagenet_val_1k",
      "source_type": "IMG_DIR",
      "loop_num": 1,
      "sample_interval": 1,
      "fps": -1
    }
  ],
  "engine_config_path": "../resnet/config/engine.json"
}
```

[engine.json](./config/engine.json) contains configuration information for graphs, which, once set, typically remain unchanged.

Here's an excerpt from the configuration file as an example: Within this file, it's necessary to initialize information for each element and specify the connections between elements. The `element_id` serves as a unique identifier. `element_config` points to the detailed configuration file for that element. `port_id` denotes the input/output port number for the element. And in cases of multiple inputs or outputs, these numbers should not be duplicated. `is_src` denotes whether the current port is the input port for the entire graph, while `is_sink` identifies whether the port is the output for the whole graph. `connection` determines how elements are connected, using `element_id` and `port_id` for identification.

```json
[
    {
        "graph_id": 0,
        "device_id": 0,
        "graph_name": "resnet",
        "elements": [
            {
                "element_id": 5000,
                "element_config": "../resnet/config/decode.json",
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
                "element_config": "../resnet/config/resnet.json",
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
            }
        ]
    }
]
```

### 6.2 Execute

For PCIe platforms, you can directly run tests on the PCIe platform. For SoC platforms, you'll need to copy the dynamically linked libraries, executable files, required models, and test data generated from cross-compilation to the SoC platform for testing.

On the SoC platform, maintain a directory structure for dynamic libraries, executable files, configuration files, models, and video data consistent with the original sophon-stream repository's structure.

The parameters for testing and the method of execution remain consistent. Therefore, it'll primarily explain in terms of PCIe mode.

Run the executable file
```bash
./main --demo_config_path=../resnet/config/resnet_demo.json
```

The running results of two video streams are as follows
```bash
 total time cost 1986302 us.
frame count is 1000 | fps is 803.448 fps.
```

## 7. Performance Testing

Currently, the YOLOv5 example supports inference on BM1684X and BM1684 in PCIe and SOC modes.

The tested video is `videos/test_imagenet.mp4`. The compilation was done in Release mode. The results are as follows:

|Device|Number of Channels|Algorithm Thread Count|CPU Utilization(%)|System Memory(M)|Peak System Memory(M)|TPU Utilization(%)|Device Memory(M)|Peak Device Memory(M)|AverageFPS|Peak FPS|
|----|----|-----|-----|-----|-----|-----|-----|-----|-----|-----|
|SE7|8|8|357.18|24.93|31.98|95.95|83.41|95.00|1990.34|2222.02|
|SE5-16|8| 8 |128.98|19.52|20.68|99.30|73.93|90.00|713.51|739.36|
|SE5-8|8| 4 |81.38|19.56|20.43|94.38|52.12|61.00|448.69|462.06|

> **Test Description**:
1. Performance test results exhibit certain fluctuations; it's advisable to conduct multiple tests and calculate the average.
2. Both BM1684 and BM1684X SoC devices utilize an 8-core ARM A53 processor, offering 42320 DMIPS @ 2.3GHz.