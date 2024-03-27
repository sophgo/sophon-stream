# YOLOv8 Demo

English | [简体中文](README.md)

## Catalogs
- [YOLOv8 Demo](#yolov8-demo)
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

**source code** (https://github.com/ultralytics/ultralytics)

In this example, the pre-processing, inference, and post-processing of the YOLOv8 algorithm are computed on three separate elements, allowing multiple threads to be utilized within each element, ensuring a certain level of detection efficiency.

## 2. Feature

* Supports BM1684X, BM1684(x86 PCIe、SoC), supports BM1688(SoC)
* On the BM1684X platform, the TPU_kernel post-processing is supported.
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
|   ├── yolov8n_cls_fp32_1b.bmodel  # Compile with TPU-MLIR，FP32 yolov8-cls BModel for BM1684，batch_size=1
│   ├── yolov8n_pose_fp32_1b.bmodel # Compile with TPU-MLIR，FP32 yolov8-pose BModel for BM1684，batch_size=1
│   ├── yolov8n_pose_int8_1b.bmodel # Compile with TPU-MLIR，INT8 yolov8-pose BModel for BM1684，batch_size=1
│   ├── yolov8s_fp32_1b.bmodel   # Compile with TPU-MLIR, FP32 BModel for BM1684, batch_size=1
│   ├── yolov8s_int8_1b.bmodel   # Compile with TPU-MLIR, INT8 BModel for BM1684, batch_size=1
│   └── yolov8s_int8_4b.bmodel   # Compile with TPU-MLIR, INT8 BModel for BM1684, batch_size=4
├── BM1684X
|   ├── yolov8n_cls_fp32_1b.bmodel  # Compile with TPU-MLIR，FP32 yolov8-cls BModel for BM1684X，batch_size=1
│   ├── yolov8n_pose_fp32_1b.bmodel # Compile with TPU-MLIR，FP32 yolov8-pose BModel for BM1684X，batch_size=1
│   ├── yolov8n_pose_int8_1b.bmodel # Compile with TPU-MLIR，INT8 yolov8-pose BModel for BM1684X，batch_size=1
│   ├── yolov8s_fp32_1b.bmodel   # Compile with TPU-MLIR, FP32 BModel for BM1684X, batch_size=1
│   ├── yolov8s_fp16_1b.bmodel   # Compile with TPU-MLIR, FP16 BModel for BM1684X, batch_size=1
│   ├── yolov8s_int8_1b.bmodel   # Compile with TPU-MLIR, INT8 BModel for BM1684X, batch_size=1
│   └── yolov8s_int8_4b.bmodel   # Compile with TPU-MLIR, INT8 BModel for BM1684X, batch_size=4
└── BM1688
    ├── yolov8n_cls_fp32_1b.bmodel    # Compile with TPU-MLIR，FP32 1 core yolov8-cls BModel for BM1688 batch_size=1
    ├── yolov8n_cls_fp32_1b_2core.bmodel    # Compile with TPU-MLIR，FP32 2 core yolov8-cls BModel for BM1688，batch_size=1
    ├── yolov8n_pose_fp32_1b_1core.bmodel   # Compile with TPU-MLIR, FP32 1core yolov8-pose BModel for BM1688, batch_size=1
    ├── yolov8n_pose_fp32_1b_2core.bmodel   # Compile with TPU-MLIR, FP32 2core yolov8-pose BModel for BM1688, batch_size=1
    ├── yolov8n_pose_int8_1b_1core.bmodel   # Compile with TPU-MLIR, INT8 1core yolov8-pose BModel for BM1688, batch_size=1
    ├── yolov8n_pose_int8_1b_2core.bmodel   # Compile with TPU-MLIR, INT8 2core yolov8-pose BModel for BM1688, batch_size=1
    ├── yolov8s_fp16_1b_2core.bmodel  # Compile with TPU-MLIR, FP16 2 core BModel for BM1688, batch_size=1
    ├── yolov8s_fp16_1b.bmodel        # Compile with TPU-MLIR, FP16 1 core BModel for BM1688, batch_size=1
    ├── yolov8s_fp16_4b_2core.bmodel  # Compile with TPU-MLIR, FP16 2 core BModel for BM1688, batch_size=4
    ├── yolov8s_fp16_4b.bmodel        # Compile with TPU-MLIR, FP16 1 core BModel for BM1688, batch_size=4
    ├── yolov8s_fp32_1b_2core.bmodel  # Compile with TPU-MLIR, FP32 2 core BModel for BM1688, batch_size=1
    ├── yolov8s_fp32_1b.bmodel        # Compile with TPU-MLIR, FP32 1 core BModel for BM1688, batch_size=1
    ├── yolov8s_fp32_4b_2core.bmodel  # Compile with TPU-MLIR, FP32 2 core BModel for BM1688, batch_size=4
    ├── yolov8s_fp32_4b.bmodel        # Compile with TPU-MLIR, FP32 1 core BModel for BM1688, batch_size=4
    ├── yolov8s_int8_1b_2core.bmodel  # Compile with TPU-MLIR, INT8 2 core BModel for BM1688, batch_size=1
    ├── yolov8s_int8_1b.bmodel        # Compile with TPU-MLIR, INT8 1 core BModel for BM1688, batch_size=1
    ├── yolov8s_int8_4b_2core.bmodel  # Compile with TPU-MLIR, INT8 2 core BModel for BM1688, batch_size=4
    └── yolov8s_int8_4b.bmodel        # Compile with TPU-MLIR, INT8 1 core BModel for BM1688, batch_size=4
```

Model description:

The above models are ported from the official [yolov8 repository](https://github.com/ultralytics/ultralytics). The plugin configuration includes `mean=[0,0,0]`, `std=[255,255,255]`, supporting 80-class detection tasks from the COCO dataset.

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

In the YOLOv8 demo, various parameters for each section are located in [config](./config/) directory, structured as follows:

```bash
./config/
├── decode.json                 # decoding configuration
├── engine_group.json           # sophon-stream graph configuration
├── yolov8_classthresh_roi_example.json  # reference configuration file for setting thresholds per category in YOLOv8. Please note that setting thresholds per category is only supported in non-tpu_kernel post-processing mode
├── yolov8_demo.json            # input configuration file for the demo
├── yolov8_group.json           # A simplified YOLOv8 configuration file that combines pre-processing, inference, and post-processing into one configuration file.
```

Indeed, [yolov8_demo.json](./config/yolov8_demo.json) is the overall configuration file for the example, managing input streams and other information. Multiple data inputs can be supported on a single graph, where the `channels` parameter configures the number of input channels, `sample_interval` sets the frame skipping rate, and `loop_num` sets the number of looped plays. The `channel` section contains video stream information such as the URL. The `download_image` parameter controls whether to save the inference results. If set to `false`, results will not be saved. If set to `true`, they will be saved in the `/build/results` directory.

In the configuration file, when the `channel_id` attribute is not specified, the demo will assign default `channel_id` values starting from 0 for each data channel.

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

[engine_group.json](./config/engine_group.json) contains configuration information for graphs, which, once set, typically remain unchanged.

Here's an excerpt from the configuration file as an example: Within this file, it's necessary to initialize information for each element and specify the connections between elements. The `element_id` serves as a unique identifier. `element_config` points to the detailed configuration file for that element. `port_id` denotes the input/output port number for the element. And in cases of multiple inputs or outputs, these numbers should not be duplicated. `is_src` denotes whether the current port is the input port for the entire graph, while `is_sink` identifies whether the port is the output for the whole graph. `connection` determines how elements are connected, using `element_id` and `port_id` for identification.

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

[yolov8_group.json](./config/yolov8_group.json) and similar configuration files detail the configuration specifics for a particular element, setting parameters such as model settings, dynamic library paths, thresholds, and more. These configuration files don't require specifying the `id` or `device_id` fields, as the demo will pass in the `element_id` and `device_id` specified in the engine_group.json.

Among these configurations, `thread_number` specifies the number of working threads within the `element`. Each thread corresponds to a data queue. In scenarios with multiple inputs, it's essential to set the number of data queues reasonably to ensure an even and adequate workload distribution across threads.

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

### 6.2 Execute

For PCIe platforms, you can directly run tests on the PCIe platform. For SoC platforms, you'll need to copy the dynamically linked libraries, executable files, required models, and test data generated from cross-compilation to the SoC platform for testing.

On the SoC platform, maintain a directory structure for dynamic libraries, executable files, configuration files, models, and video data consistent with the original sophon-stream repository's structure.

The parameters for testing and the method of execution remain consistent. Therefore, it'll primarily explain in terms of PCIe mode.

Run the executable file
```bash
./main --demo_config_path=../yolov8/config/yolov8_demo.json
```

The running results of two video streams are as follows
```bash
 total time cost 13714673 us.
frame count is 1424 | fps is 103.83 fps.
```

## 7. Performance Testing

Currently, the YOLOv5 example supports inference on BM1684X and BM1684 in PCIe and SoC modes, and supports inference on BM1688 in SoC mode.

Modifications in JSON configurations might be necessary when switching between different devices, such as adjusting model paths, input channels, etc. Refer to section 6.1 for JSON configuration methods and section 6.2 for program execution methods.

Due to significant differences in CPU capabilities among PCIe devices, performance data is not meaningful. Therefore, only provide the test results for SOC mode.

The tested video is `elevator-1080p-25fps-4000kbps.h264`. The compilation was done in Release mode. The results are as follows:


| Device | Number of Channels | Algorithm Thread Count | CPU Utilization (%) | System Memory (M) | TPU Utilization (%) |  Peak Device Memory (M) | Average FPS | 
|----|----|-----|-----|-----|-----|-----|-----|
|SE7|4|4-4-4|470|96|40|2388|95.24|
|SE5-16|4|4-4-4|453|96|90|2700|89.02|
|SE9-16|4|4-4-4|489|111|70|2700|72|

> **Test Description**:
1. Performance test results exhibit certain fluctuations; it's advisable to conduct multiple tests and calculate the average.
2. Both BM1684 and BM1684X SoC devices utilize an 8-core ARM A53 processor, offering 42320 DMIPS @ 2.3GHz.
3. All aforementioned performance tests are based on the INT8 model.
4. Running models with a batch size of 4 on the BM1684 device can achieve higher FPS.
5. On the BM1684X device, utilizing a batch size of 1 for the model and enabling the tpu_kernel for post-processing can yield higher FPS.
6. For the settings of input channels and algorithm thread count in the table, please refer to [JSON configuration explanation](#61-json-configuration). CPU utilization and system memory can be checked using the `top` command. TPU utilization and device memory can be checked using the `bm-smi` command. FPS can be obtained from the logs printed during program execution.
7. Performance testing is not currently supported on the BM1688 device.