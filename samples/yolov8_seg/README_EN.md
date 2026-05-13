# YOLOv8 Seg Demo

English | [简体中文](README.md)

## Table of Contents
- [YOLOv8 Seg Demo](#yolov8-seg-demo)
  - [Table of Contents](#table-of-contents)
  - [1. Introduction](#1-introduction)
  - [2. Features](#2-features)
  - [3. Prepare Models and Data](#3-prepare-models-and-data)
  - [4. Environment Setup](#4-environment-setup)
    - [4.1 x86/arm PCIe Platform](#41-x86arm-pcie-platform)
    - [4.2 SoC Platform](#42-soc-platform)
  - [5. Program Compilation](#5-program-compilation)
    - [5.1 x86/arm PCIe Platform](#51-x86arm-pcie-platform)
    - [5.2 SoC Platform](#52-soc-platform)
  - [6. Program Execution](#6-program-execution)
    - [6.1 JSON Configuration](#61-json-configuration)
    - [6.2 Running](#62-running)
  - [7. Performance Test](#7-performance-test)

## 1. Introduction

This example demonstrates how to use sophon-stream to quickly build a video instance segmentation application.

**Source Code** (https://github.com/ultralytics/ultralytics)

In this example, the pre-processing, inference, and post-processing of the yolov8-seg algorithm are performed on three separate elements. Each element can open multiple threads internally, ensuring a certain detection efficiency.

The post-processing references the C++ example implementation of [sophon-demo YOLOv8_plus_seg](https://github.com/sophon-ai/sophon-demo) and uses CPU to complete mask generation and rendering. Visualization results are output through the osd element, which simultaneously draws detection boxes and segmentation masks.

## 2. Features

* Supports BM1684X (x86 PCIe, SoC)
* Supports multiple video streams
* Supports multi-threading
* Supports visualization of detection boxes + segmentation masks
* Post-processing aligned with sophon-demo YOLOv8_plus_seg

## 3. Prepare Models and Data

The models come from the sophon-demo YOLOv8_plus_seg example, available from the sophon-demo repository.

```bash
# Download models from sophon-demo
# Download and extract from open@sophgo.com:sophon-demo/YOLOv8_plus_seg/BM1684X.tar.gz
tar -xzf BM1684X.tar.gz
cp -r BM1684X/* ../yolov8_seg/data/models/BM1684X/
```

The downloaded models include:

```bash
./models/BM1684X/
├── yolov8s_fp32_1b.bmodel   # TPU-MLIR compiled, FP32 yolov8-seg BModel for BM1684X, batch_size=1
├── yolov8s_fp16_1b.bmodel   # TPU-MLIR compiled, FP16 yolov8-seg BModel for BM1684X, batch_size=1
├── yolov8s_int8_1b.bmodel   # TPU-MLIR compiled, INT8 yolov8-seg BModel for BM1684X, batch_size=1
├── yolov8s_int8_4b.bmodel   # TPU-MLIR compiled, INT8 yolov8-seg BModel for BM1684X, batch_size=4
├── yolov9s_fp32_1b.bmodel   # TPU-MLIR compiled, FP32 yolov9-seg BModel for BM1684X, batch_size=1
├── yolov9s_fp16_1b.bmodel   # TPU-MLIR compiled, FP16 yolov9-seg BModel for BM1684X, batch_size=1
├── yolov9c_fp32_1b.bmodel   # TPU-MLIR compiled, FP32 yolov9-seg BModel for BM1684X, batch_size=1
├── yolov9c_fp16_1b.bmodel   # TPU-MLIR compiled, FP16 yolov9-seg BModel for BM1684X, batch_size=1
├── yolov9c_int8_1b.bmodel   # TPU-MLIR compiled, INT8 yolov9-seg BModel for BM1684X, batch_size=1
└── yolov9c_int8_4b.bmodel   # TPU-MLIR compiled, INT8 yolov9-seg BModel for BM1684X, batch_size=4
```

Model description:

The above models are ported from [yolov8 official](https://github.com/ultralytics/ultralytics). Plugin configuration: `mean=[0,0,0]`, `std=[255,255,255]`.

The model output contains 2 tensors:
- Detection output (transposed format `[1, 8400, 116]`): contains bbox coordinates (4), class scores (80), mask coefficients (32)
- Segmentation output (`[1, 32, 160, 160]`): mask prototypes

> **Note**: Unlike yolov8 detect models, segmentation models use a transposed format (anchor-major). The post-processing code has been adapted accordingly.

The test data is shared with the yolov8 sample via a symlink pointing to the `../yolov8/data/` directory.

## 4. Environment Setup

### 4.1 x86/arm PCIe Platform

If you have installed a PCIe accelerator card (such as SC series accelerator cards) on an x86/arm platform, you can directly use it as both the development and runtime environment. You need to install libsophon, sophon-opencv, and sophon-ffmpeg. For detailed steps, refer to [x86-pcie platform development and runtime environment setup](../../docs/EnvironmentInstallGuide.md#3-x86-pcie-platform-development-and-runtime-environment-setup) or [arm-pcie platform development and runtime environment setup](../../docs/EnvironmentInstallGuide.md#5-arm-pcie-platform-development-and-runtime-environment-setup).

### 4.2 SoC Platform

If you are using an SoC platform (such as SE, SM series edge devices), the corresponding libsophon, sophon-opencv, and sophon-ffmpeg runtime library packages are pre-installed under `/opt/sophon/` after flashing. You can use it directly as the runtime environment. Typically, an x86 host is also needed as the development environment for cross-compiling C++ programs.

## 5. Program Compilation

### 5.1 x86/arm PCIe Platform
Programs can be compiled directly on the PCIe platform. For details, refer to [sophon-stream compilation](../../docs/HowToMake.md).

### 5.2 SoC Platform
Programs are typically cross-compiled on an x86 host. You need to set up a cross-compilation environment on the x86 host using the SOPHON SDK, and package the required header files and library files into the sophon_sdk_soc directory. For details, refer to [sophon-stream compilation](../../docs/HowToMake.md). This example mainly depends on the libsophon, sophon-opencv, and sophon-ffmpeg runtime library packages.

## 6. Program Execution

### 6.1 JSON Configuration

The parameters for each part of the yolov8_seg demo are located in the [config](./config/) directory, structured as follows:

```bash
./config/
├── decode.json                 # Decode configuration
├── engine_group.json           # sophon-stream graph configuration
├── yolov8_seg_demo.json        # Demo input configuration file
├── yolov8_group.json           # Simplified yolov8 configuration, combining pre-processing, inference, and post-processing into one config file
└── osd.json                    # OSD visualization configuration
```

[yolov8_seg_demo.json](./config/yolov8_seg_demo.json) is the overall configuration file for the example, managing input stream information.

The graph topology is `decode → yolov8_group → osd → encode`, with output saved in the `./results` directory.

Configure task_type as "Seg" in [yolov8_group.json](./config/yolov8_group.json). FP32 BModel is recommended (FP32 models produce more accurate confidence scores):

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

Configure draw_utils as "OPENCV" to support mask rendering, and osd_type as "DET" to draw detection boxes and segmentation masks in [osd.json](./config/osd.json):

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

> **Note**:
> 1. Segmentation mask rendering requires OPENCV draw_utils (BMCV mode does not support mask rendering).
> 2. When mSegmentedObjectMetadatas is not empty, osd automatically overlays segmentation masks on top of detection boxes.
> 3. INT8 models may produce lower confidence scores under the current configuration. FP32 models are recommended for better segmentation results.

### 6.2 Running

For PCIe platforms, tests can be run directly on the PCIe platform. For SoC platforms, copy the cross-compiled dynamic libraries, executable files, required models, and test data to the SoC platform for testing.

1. Run the executable
```bash
./main --demo_config_path=../yolov8_seg/config/yolov8_seg_demo.json
```

Sample output for 4 video streams:
```bash
 total time cost 114256300 us.
frame count is 2848 | fps is 24.93 fps.
```

## 7. Performance Test

Currently, the yolov8_seg example supports inference on BM1684X in both PCIe and SoC modes.

Test video `test_car_person_1080P.avi`, compiled in Release mode, testing yolov8 seg model performance:

| Device | Streams | Algorithm Threads | CPU Usage(%) | TPU Usage(%) | Peak Device Memory(M) | Average FPS |
|--------|---------|-------------------|--------------|--------------|----------------------|-------------|
| SE7    | 4       | 4-4-4             | -            | -            | -                    | 24.93       |

> **Test Notes**:
> 1. Performance test results have some variability; it is recommended to run multiple tests and take the average;
> 2. The above performance tests are based on the yolov8s_fp32_1b segmentation model;
> 3. Segmentation post-processing (CPU get_mask) and mask rendering (OpenCV) consume additional CPU time, resulting in lower fps compared to pure detection tasks;
> 4. In the table above, input streams and algorithm thread settings refer to [JSON Configuration](#61-json-configuration). CPU utilization and system memory can be checked with the `top` command, TPU utilization and device memory with the `bm-smi` command, and fps from the program's printed logs;
