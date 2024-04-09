# retinaface Demo

English | [简体中文](README.md)

## Catalogs
- [Retinaface Demo](#retinaface-demo)
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

This example demonstrates how to use sophon-stream to quickly build a video object detection application.

**source code** (https://github.com/biubug6/Pytorch_Retinaface)

In this example, the pre-processing, inference, and post-processing of the YOLOv5 algorithm are computed on three separate elements, allowing multiple threads to be utilized within each element, ensuring a certain level of detection efficiency.

## 2. Features

* Supports BM1684X, BM1684(x86 PCIe、SoC), BM1688(SoC), CV186X(SoC).
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

After the script execution, `data` directory will be generated in the current directory, containing two subdirectories: `models` and `videos`

The downloaded models include:

```bash
./models/
│   ├── BM1684
│   │   ├── retinaface_mobilenet0.25_fp32_1b.bmodel # FP32 BModel for BM1684，batch_size=1，post-process on CPU
│   │   ├── retinaface_mobilenet0.25_int8_1b.bmodel # INT8 BModel for BM1684，batch_size=1，post-process on CPU
│   │   └── retinaface_mobilenet0.25_int8_4b.bmodel # INT8 BModel for BM1684，batch_size=4，post-process on CPU
│   ├── BM1684X
│   │   ├── retinaface_mobilenet0.25_fp16_1b.bmodel # FP16 BModel for BM1684X，batch_size=1，post-process on CPU
│   │   ├── retinaface_mobilenet0.25_fp32_1b.bmodel # FP32 BModel for BM1684X，batch_size=1，post-process on CPU
│   │   ├── retinaface_mobilenet0.25_int8_1b.bmodel # INT8 BModel for BM1684X，batch_size=1，post-process on CPU
│   │   └── retinaface_mobilenet0.25_int8_4b.bmodel # INT8 BModel for BM1684X，batch_size=4，post-process on CPU
│   ├── BM1688
│   │   ├── retinaface_mobilenet0.25_fp16_1b_2core.bmodel # FP16 BModel for BM1688, batch_size=1, post-process on CPU
│   │   ├── retinaface_mobilenet0.25_fp16_1b.bmodel       # FP16 BModel for BM1688 and CV186X, batch_size=1, post-process on CPU
│   │   ├── retinaface_mobilenet0.25_fp32_1b_2core.bmodel # FP32 BModel for BM1688, batch_size=1, post-process on CPU
│   │   ├── retinaface_mobilenet0.25_fp32_1b.bmodel       # FP32 BModel for BM1688 and CV186X, batch_size=1, post-process on CPU
│   │   ├── retinaface_mobilenet0.25_int8_1b_2core.bmodel # INT8 BModel for BM1688, batch_size=1, post-process on CPU
│   │   ├── retinaface_mobilenet0.25_int8_1b.bmodel       # INT8 BModel for BM1688 and CV186X, batch_size=1, post-process on CPU
│   │   ├── retinaface_mobilenet0.25_int8_4b_2core.bmodel # FP16 BModel for BM1688, batch_size=4, post-process on CPU
│   │   └── retinaface_mobilenet0.25_int8_4b.bmodel       # INT8 BModel for BM1688 and CV186X, batch_size=4, post-process on CPU
│   └── onnx
│       └── retinaface_mobilenet0.25.onnx # origin model
```

The downloaded data include:

```bash
./images/
├── face            # images and video for test
│   └── test
│       ├── face
│       └── videos
├── WIDERVAL
└── wind
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

In the retinaface demo, various parameters for each section are located in [config](./config/) directory, structured as follows:

```bash
config/
├── decode.json                 # decoding configuration
├── engine_group.json           # sophon-stream Simplified graph configuration
├── engine.json                 # sophon-stream graph configuration requires separate configuration for pre-processing, inference, and post-processing files.
├── retinaface_demo.json        # input configuration file for the demo
├── retinaface_group.json       # A simplified retinaface configuration file that combines pre-processing, inference, and post-processing into one configuration file.
├── retinaface_infer.json       # retinaface inference configuration file
├── retinaface_post.json        # retinaface post-process configuration file
└── retinaface_pre.json         # retinaface pre-process configuration file
```

Indeed, [retinaface_demo.json](./config/retinaface_demo.json) is the overall configuration file for the example, managing input streams and other information. Multiple data inputs can be supported on a single graph, where the `channels` parameter configures the number of input channels, `sample_interval` sets the frame skipping rate, and `loop_num` sets the number of looped plays. The `channel` section contains video stream information such as the URL. The `download_image` parameter controls whether to save the inference results. If set to `false`, results will not be saved. If set to `true`, they will be saved in the `/build/results` directory.

In the configuration file, when the `channel_id` attribute is not specified, the demo will assign default `channel_id` values starting from 0 for each data channel.

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

[engine_group.json](./config/engine_group.json) contains configuration information for graphs, which, once set, typically remain unchanged.

Here's an excerpt from the configuration file as an example: Within this file, it's necessary to initialize information for each element and specify the connections between elements. The `element_id` serves as a unique identifier. `element_config` points to the detailed configuration file for that element. `port_id` denotes the input/output port number for the element. And in cases of multiple inputs or outputs, these numbers should not be duplicated. `is_src` denotes whether the current port is the input port for the entire graph, while `is_sink` identifies whether the port is the output for the whole graph. `connection` determines how elements are connected, using `element_id` and `port_id` for identification.

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

[retinaface_group.json](./config/retinaface_group.json) and similar configuration files detail the configuration specifics for a particular element, setting parameters such as model settings, dynamic library paths, thresholds, and more. These configuration files don't require specifying the `id` or `device_id` fields, as the demo will pass in the `element_id` and `device_id` specified in the engine_group.json.

Among these configurations, `thread_number` specifies the number of working threads within the `element`. Each thread corresponds to a data queue. In scenarios with multiple inputs, it's essential to set the number of data queues reasonably to ensure an even and adequate workload distribution across threads.

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

### 6.2 Execute

For PCIe platforms, you can directly run tests on the PCIe platform. For SoC platforms, you'll need to copy the dynamically linked libraries, executable files, required models, and test data generated from cross-compilation to the SoC platform for testing.

On the SoC platform, maintain a directory structure for dynamic libraries, executable files, configuration files, models, and video data consistent with the original sophon-stream repository's structure.

The parameters for testing and the method of execution remain consistent. Therefore, it'll primarily explain in terms of PCIe mode.

Run the executable file
```bash
./main --demo_config_path=../retinaface/config/retinaface_demo.json
```

The running results of eight video streams are as follows
```bash
total time cost 4798582 us.
frame count is 920 | fps is 191.723 fps.
```

## 7. Performance Testing

Currently, the retinaface example supports inference on BM1684X and BM1684 in PCIe and SOC modes, BM1688 and CV186X in SoC.

The tested data is `/data/images/wind`. The compilation was done in Release mode. Using the int8 model, the results are as follows:

|Device|Number of Channels|Algorithm Thread Count|CPU Utilization(%)|Average FPS|
|----|----|-----|-----|-----|
|SE7    |4  |4-4-4  |381  |428.797|
|SE9-16 |4  |4-4-4  |400  |263.333|

> **Test Description**:
1. Performance test results exhibit certain fluctuations; it's advisable to conduct multiple tests and calculate the average.
2. Both SE5 and SE7 devices utilize an 8-core ARM A53 processor, offering 42320 DMIPS @ 2.3GHz. SE9-16 device utilizes an 8-core ARM A53 processor @ 1.6GHz, and SE9-8 device utilizes an 6-core ARM A53 processor @ 1.6GHz