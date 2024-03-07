# YOLOX Demo

English | [简体中文](README.md)

## Catalogs
- [YOLOX Demo](#yolox-demo)
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

The connection method for this example plugin is shown in the following diagram.

![process](./pics/elements.jpg)

yolox is proposed by Megvii and is based on the improvement of the YOLO series.

**Paper** (https://arxiv.org/abs/2107.08430)

**Source Code** (https://github.com/Megvii-BaseDetection/YOLOX)

In this example, the pre-processing, inference, and post-processing of the YOLOX algorithm are computed on three separate elements, allowing multiple threads to be utilized within each element, ensuring a certain level of detection efficiency.

## 2. Feature

* Supports BM1684X, BM1684(x86 PCIe、SoC), supports BM1688(SoC)
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
├── BM1684
│   ├── yolox_bytetrack_s_fp32_1b.bmodel    # FP32 BModel for BM1684，batch_size=1
│   ├── yolox_bytetrack_s_fp32_4b.bmodel    # FP32 BModel for BM1684，batch_size=4
│   ├── yolox_bytetrack_s_int8_1b.bmodel    # INT8 BModel for BM1684，batch_size=1
│   ├── yolox_bytetrack_s_int8_4b.bmodel    # INT8 BModel for BM1684，batch_size=4
│   ├── yolox_s_fp32_1b.bmodel              # FP32 BModel for BM1684，batch_size=1
│   ├── yolox_s_fp32_4b.bmodel              # FP32 BModel for BM1684，batch_size=4
│   ├── yolox_s_int8_1b.bmodel              # INT8 BModel for BM1684，batch_size=1
│   └── yolox_s_int8_4b.bmodel              # INT8 BModel for BM1684，batch_size=4
├── BM1684X
│   ├── yolox_bytetrack_s_fp16_1b.bmodel    # FP16 BModel for BM1684X，batch_size=1
│   ├── yolox_bytetrack_s_fp32_1b.bmodel    # FP32 BModel for BM1684X，batch_size=1
│   ├── yolox_bytetrack_s_int8_1b.bmodel    # INT8 BModel for BM1684X，batch_size=1
│   ├── yolox_bytetrack_s_int8_4b.bmodel    # INT8 BModel for BM1684X，batch_size=4
│   ├── yolox_s_fp32_1b.bmodel              # FP32 BModel for BM1684X，batch_size=1
│   ├── yolox_s_fp32_4b.bmodel              # FP32 BModel for BM1684X，batch_size=4
│   ├── yolox_s_int8_1b.bmodel              # INT8 BModel for BM1684X，batch_size=1
│   └── yolox_s_int8_4b.bmodel              # INT8 BModel for BM1684X，batch_size=4
└── BM1688_2cores
    ├── yolox_s_int8_1b.bmodel              # INT8 BModel for BM1688，batch_size=1
    └── yolox_s_int8_4b.bmodel              # INT8 BModel for BM1688，batch_size=4
```

Model description:

1.`yolox_s_bytetrack_` models are from [bytetrack](https://github.com/ifzhang/ByteTrack), `mean=[0,0,0]`, `std=[255,255,255]`, support for person category detection tasks.

2.`yolox_s` models are from [yolox](https://github.com/Megvii-BaseDetection/YOLOX), `mean=[0,0,0]`, `std=[1,1,1]`, support for 80 classes of COCO dataset.

The downloaded data include:

```bash
videos/
├── carvana_video.mp4   # test video
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

In the YOLOX demo, various parameters for each section are located in [config](./config/) directory, structured as follows:

```bash
./config
├── decode.json             # decoding configuration
├── engine_group.json       # sophon-stream Simplified graph configuration
├── engine.json             # sophon-stream graph configuration requires separate configuration for pre-processing, inference, and post-processing files.
├── yolox_classthresh_roi_example.json # reference configuration file for setting thresholds per category in YOLOX. Please note that setting thresholds per category is only supported in non-tpu_kernel post-processing mode
├── yolox_demo.json         # input configuration file for the demo
├── yolox_group.json        # A simplified YOLOX configuration file that combines pre-processing, inference, and post-processing into one configuration file.
├── yolox_infer.json        # yolox inference configuration file
├── yolox_post.json         # yolox post-processing configuration file
└── yolox_pre.json          # yolox pre-processing configuration file
```

Indeed, [yolox_demo.json](./config/yolox_demo.json) is the overall configuration file for the example, managing input streams and other information. Multiple data inputs can be supported on a single graph, where the `channels` parameter configures the number of input channels, `sample_interval` sets the frame skipping rate, and `loop_num` sets the number of looped plays. The `channel` section contains video stream information such as the URL. The `download_image` parameter controls whether to save the inference results. If set to `false`, results will not be saved. If set to `true`, they will be saved in the `/build/results` directory.

In the configuration file, when the `channel_id` attribute is not specified, the demo will assign default `channel_id` values starting from 0 for each data channel.

```json
{
  "channels": [
    {
      "channel_id": 2,
      "url": "../yolox/data/videos/elevator-1080p-25fps-4000kbps.h264",
      "source_type": "VIDEO",
      "sample_interval": 1,
      "loop_num": 1,
      "fps": -1
    },
    {
      "channel_id": 3,
      "url": "../yolox/data/videos/elevator-1080p-25fps-4000kbps.h264",
      "source_type": "VIDEO",
      "sample_interval": 1,
      "loop_num": 1,
      "fps": -1
    },
    {
      "channel_id": 20,
      "url": "../yolox/data/videos/elevator-1080p-25fps-4000kbps.h264",
      "source_type": "VIDEO",
      "sample_interval": 1,
      "loop_num": 1,
      "fps": -1
    },
    {
      "channel_id": 30,
      "url": "../yolox/data/videos/elevator-1080p-25fps-4000kbps.h264",
      "source_type": "VIDEO",
      "sample_interval": 1,
      "loop_num": 1,
      "fps": -1
    }
  ],
  "class_names": "../yolox/data/coco.names",
  "download_image": true,
  "draw_func_name": "draw_yolox_results",
  "engine_config_path": "../yolox/config/engine_group.json"
}
```

[engine_group.json](./config/engine_group.json) contains configuration information for graphs, which, once set, typically remain unchanged.

Here's an excerpt from the configuration file as an example: Within this file, it's necessary to initialize information for each element and specify the connections between elements. The `element_id` serves as a unique identifier. `element_config` points to the detailed configuration file for that element. `port_id` denotes the input/output port number for the element. And in cases of multiple inputs or outputs, these numbers should not be duplicated. `is_src` denotes whether the current port is the input port for the entire graph, while `is_sink` identifies whether the port is the output for the whole graph. `connection` determines how elements are connected, using `element_id` and `port_id` for identification.

```json
[
    {
        "graph_id": 0,
        "device_id": 0,
        "graph_name": "yolox",
        "elements": [
            {
                "element_id": 5000,
                "element_config": "../yolox/config/decode.json",
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
                "element_config": "../yolox/config/yolox_group.json",
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

[yolox_group.json](./config/yolox_group.json) and similar configuration files detail the configuration specifics for a particular element, setting parameters such as model settings, dynamic library paths, thresholds, and more. These configuration files don't require specifying the `id` or `device_id` fields, as the demo will pass in the `element_id` and `device_id` specified in the engine_group.json.

Among these configurations, `thread_number` specifies the number of working threads within the `element`. Each thread corresponds to a data queue. In scenarios with multiple inputs, it's essential to set the number of data queues reasonably to ensure an even and adequate workload distribution across threads.

```json
{
    "configure": {
      "model_path": "../yolox/data/models/BM1684X/yolox_s_int8_1b.bmodel",
      "threshold_conf": 0.5,
      "threshold_nms": 0.5,
      "bgr2rgb": true,
      "mean": [
        0,
        0,
        0
      ],
      "std": [
        1,
        1,
        1
      ]
    },
    "shared_object": "../../build/lib/libyolox.so",
    "name": "yolox_group",
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
./main --demo_config_path=../yolox/config/yolox_demo.json
```

The running results of two video streams are as follows
```bash
 total time cost 5272393 us.
frame count is 1422 | fps is 269.707 fps.
```

## 7. Performance Testing

Currently, the YOLOX example supports inference on BM1684X and BM1684 in PCIe and SoC modes, and supports inference on BM1688 in SoC mode.

Modifications in JSON configurations might be necessary when switching between different devices, such as adjusting model paths, input channels, etc. Refer to section 6.1 for JSON configuration methods and section 6.2 for program execution methods.

Due to significant differences in CPU capabilities among PCIe devices, performance data is not meaningful. Therefore, only provide the test results for SOC mode.

The tested video is `elevator-1080p-25fps-4000kbps.h264`. The compilation was done in Release mode. The results are as follows:

| Device | Number of Channels | Algorithm Thread Count | CPU Utilization (%) | System Memory (M) | Peak System Memory (M) | TPU Utilization (%) | Device Memory (M) | Peak Device Memory (M) | Average FPS | Peak FPS |
|----|----|-----|-----|-----|-----|-----|-----|-----|-----|-----|
|SE7|8|4-4-4|204.26|195.29|201.29|99.45|1373.88|1611.00|319.44|329.85|
|SE5-16|4|4-4-4|78.39|122.59|124.93|94.60|1252.69|1424.00|130.26|140.32|
|SE5-8|3|3-3-3|51.42|96.97|98.46|92.77|992.12|1116.00|82.03|91.62|

> **Test Description**:
1. Performance test results exhibit certain fluctuations; it's advisable to conduct multiple tests and calculate the average.
2. Both BM1684 and BM1684X SoC devices utilize an 8-core ARM A53 processor, offering 42320 DMIPS @ 2.3GHz.
3. All aforementioned performance tests are based on the INT8 model.
4. Running models with a batch size of 4 on the BM1684 device can achieve higher FPS.
5. On the BM1684X device, utilizing a batch size of 1 for the model can yield higher FPS.
6. For the settings of input channels and algorithm thread count in the table, please refer to [JSON configuration explanation](#61-json-configuration). CPU utilization and system memory can be checked using the `top` command. TPU utilization and device memory can be checked using the `bm-smi` command. FPS can be obtained from the logs printed during program execution.
7. Performance testing is not currently available on the BM1688 device.