# OPENPOSE Demo

English | [简体中文](README.md)

## Catalogs

## 1. Introduction
- [OPENPOSE Demo](#openpose-demo)
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

This example demonstrates how to use sophon-stream to quickly build a video pose recognition application.

**source code** (https://github.com/CMU-Perceptual-Computing-Lab/openpose)

In this example, the pre-processing, inference, and post-processing of the openpose algorithm are computed on three separate elements, allowing multiple threads to be utilized within each element, ensuring a certain level of detection efficiency.

## 2. Features

* Supports BM1684X, BM1684(x86, PCIe, SoC)
* Supports multiple video streams.
* Supports multi-threading.
* On the BM1684X platform, the TPU_kernel post-processing is supported.

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
./models
├── BM1684
│   ├── pose_coco_fp32_1b.bmodel              # Compile with TPU-NNTC，FP32 BModel for BM1684，batch_size=1，18 body keypoints
│   ├── pose_coco_int8_1b.bmodel              # Compile with TPU-NNTC，INT8 BModel for BM1684，batch_size=1，18 body keypoints
│   ├── pose_coco_int8_4b.bmodel              # Compile with TPU-NNTC，INT8 BModel for BM1684，batch_size=4，18 body keypoints
│   └── pose_body_25_fp32_1b.bmodel           # Compile with TPU-NNTC，FP32 BModel for BM1684，batch_size=1，25 body keypoints
└── BM1684X
    ├── pose_coco_fp32_1b.bmodel              # Compile with TPU-MLIR，FP32 BModel for BM1684X，batch_size=1，18 body keypoints
    ├── pose_coco_fp16_1b.bmodel              # Compile with TPU-MLIR，FP16 BModel for BM1684X，batch_size=1，18 body keypoints
    ├── pose_coco_int8_1b.bmodel              # Compile with TPU-MLIR，INT8 BModel for BM1684X，batch_size=1，18 body keypoints
    ├── pose_coco_int8_4b.bmodel              # Compile with TPU-MLIR，INT8 BModel for BM1684X，batch_size=4，18 body keypoints
    ├── pose_body_25_fp32_1b.bmodel           # Compile with TPU-MLIR，FP32 BModel for BM1684X，batch_size=1，25 body keypoints
    └── pose_body_25_fp16_1b.bmodel           # Compile with TPU-MLIR，FP16 BModel for BM1684X，batch_size=1，25 body keypoints
```

The downloaded data include:

下载的数据包括：
```bash
./videos
└── test.mp4                                  # test video                               
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

In the openpose demo, various parameters for each section are located in [config](./config/) directory, structured as follows:

```bash
./config/
├── decode.json                 # decoding configuration
├── engine_group.json           # sophon-stream simplified graph configuration
├── engine.json                 # sophon-stream graph configuration requires separate configuration for pre-processing, inference, and post-processing files.
├── openpose_demo.json          # input configuration file for the demo
├── openpose_group.json         # A simplified openpose configuration file that combines pre-processing, inference, and post-processing into one configuration file.
├── openpose_infer.json         # openpose inference configuration file
├── openpose_post.json          # openpose post-processing configuration file
└── openpose_pre.json           # openpose pre-processing configuration file
```

Indeed, [openpose_demo.json](./config/openpose_demo.json) is the overall configuration file for the example, managing input streams and other information. Multiple data inputs can be supported on a single graph, where the `channels` parameter configures the number of input channels, `sample_interval` sets the frame skipping rate, and `loop_num` sets the number of looped plays. The `channel` section contains video stream information such as the URL. The `download_image` parameter controls whether to save the inference results. If set to `false`, results will not be saved. If set to `true`, they will be saved in the `/build/results` directory.

In the configuration file, when the `channel_id` attribute is not specified, the demo will assign default `channel_id` values starting from 0 for each data channel.

```json
{
  "channels": [
    {
      "channel_id": 2,
      "url": "../openpose/data/videos/test.mp4",
      "source_type": "VIDEO",
      "loop_num": 1,
      "fps": -1
    },
    {
      "channel_id": 3,
      "url": "../openpose/data/videos/test.mp4",
      "source_type": "VIDEO",
      "loop_num": 1,
      "fps": -1
    },
    {
      "channel_id": 20,
      "url": "../openpose/data/videos/test.mp4",
      "source_type": "VIDEO",
      "loop_num": 1,
      "fps": -1
    },
    {
      "channel_id": 30,
      "url": "../openpose/data/videos/test.mp4",
      "source_type": "VIDEO",
      "loop_num": 1,
      "fps": -1
    }
  ],
  "download_image": false,
  "draw_func_name": "draw_openpose_results",
  "engine_config_path": "../openpose/config/engine_group.json"
}
```

[engine_group.json](./config/engine_group.json) contains configuration information for graphs, which, once set, typically remain unchanged.

Here's an excerpt from the configuration file as an example: Within this file, it's necessary to initialize information for each element and specify the connections between elements. The `element_id` serves as a unique identifier. `element_config` points to the detailed configuration file for that element. `port_id` denotes the input/output port number for the element. And in cases of multiple inputs or outputs, these numbers should not be duplicated. `is_src` denotes whether the current port is the input port for the entire graph, while `is_sink` identifies whether the port is the output for the whole graph. `connection` determines how elements are connected, using `element_id` and `port_id` for identification.

```json
[
    {
        "graph_id": 0,
        "device_id": 0,
        "graph_name": "openpose",
        "elements": [
            {
                "element_id": 5000,
                "element_config": "../openpose/config/decode.json",
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
                "element_config": "../openpose/config/openpose_group.json",
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

[openpose_group.json](./config/openpose_group.json) and similar configuration files detail the configuration specifics for a particular element, setting parameters such as model settings, dynamic library paths, thresholds, and more. These configuration files don't require specifying the `id` or `device_id` fields, as the demo will pass in the `element_id` and `device_id` specified in the engine_group.json.

Among these configurations, `thread_number` specifies the number of working threads within the `element`. Each thread corresponds to a data queue. In scenarios with multiple inputs, it's essential to set the number of data queues reasonably to ensure an even and adequate workload distribution across threads.

When `use_tpu_kernel` is set to `true`, it will utilize the tpu_kernel post-processing(using tpu to do post process). Note that tpu_kernel post-processing is only supported on BM1684X devices.

```json
{
    "configure": {
        "model_path": "../openpose/data/models/BM1684X/pose_coco_int8_1b.bmodel",
        "threshold_nms": 0.05,
        "stage": [
            "pre"
        ],
        "use_tpu_kernel": true
    },
    "shared_object": "../../build/lib/libopenpose.so",
    "name": "openpose",
    "side": "sophgo",
    "thread_number": 2
}
```

### 6.2 Execute

For PCIe platforms, you can directly run tests on the PCIe platform. For SoC platforms, you'll need to copy the dynamically linked libraries, executable files, required models, and test data generated from cross-compilation to the SoC platform for testing.

On the SoC platform, maintain a directory structure for dynamic libraries, executable files, configuration files, models, and video data consistent with the original sophon-stream repository's structure.

The parameters for testing and the method of execution remain consistent. Therefore, it'll primarily explain in terms of PCIe mode.

Run the executable file
```bash
./main --demo_config_path=../openpose/config/openpose_demo.json
```

The running results of two video streams are as follows
```bash
 total time cost 29882447 us.
frame count is 1432 | fps is 47.9211 fps.
```

## 7. Performance Testing

Currently, the openpose sample supports inference in the PCIE, SOC modes of the BM1684X and BM1684.

The tested video is `test.mp4`. The compilation was done in Release mode. The results are as follows:

| Device   | Number of Channels | Alorithm Thread Count | CPU Utilization(%) | System Memory(M) | Peak System Memory(M) | TPU Utilization(%) | Device Memory(M) | Peak Device Memory(M) | Average FPS | Peak FPS | Model                     |
| ------ | ---- | ---------- | ------------ | ----------- | ---------------- | ------------ | ----------- | --------------- | ------- | -------- | ------------------------ |
| SE7    | 8    | 8-8-8      | 453.26       | 329.96      | 471.59           | 99.36        | 1301.89     | 1330.00         | 91.12   | 115.94   | pose_coco_int8_1b.bmodel |
| SE7    | 4    | 4-4-4      | 430.31       | 192.94      | 278.14           | 97.72        | 715.61      | 730.00          | 88.84   | 105.93   | pose_coco_int8_1b.bmodel |
| SE7    | 2    | 2-2-2      | 383.83       | 117.92      | 165.36           | 91.80        | 422.17      | 429.00          | 81.00   | 92.73    | pose_coco_int8_1b.bmodel |
| SE5-16 | 4    | 4-4-4      | 571.45       | 100.10      | 101.43           | 99.78        | 703.38      | 743.00          | 61.16   | 68.92    | pose_coco_int8_4b.bmodel |
| SE5-8  | 3    | 3-3-3      | 324.26       | 80.91       | 82.16            | 99.68        | 522.47      | 553.00          | 36.14   | 39.37    | pose_coco_int8_4b.bmodel |

> **Test Description**:
1. Performance test results exhibit certain fluctuations; it's advisable to conduct multiple tests and calculate the average.
2. Both BM1684 and BM1684X SoC devices utilize an 8-core ARM A53 processor, offering 42320 DMIPS @ 2.3GHz.