# license_plate_recognition Demo

English | [简体中文](README.md)

## Catalogs

- [license_plate_recognition Demo](#license_plate_recognition-demo)
  - [Catalogs](#catalogs)
  - [1. Introduction](#1-introduction)
  - [2. Features](#2-features)
  - [3. Prepare Models and Data](#3-prepare-models-and-data)
  - [4. Prepare Environment](#4-prepare-environment)
    - [4.1 x86/arm PCIe Platform](#41-x86arm-pcie-platform)
    - [4.2 SoC Platform](#42-soc-platform)
  - [5. Program Compilation](#5-program-compilation)
    - [5.1 x86/arm PCIe Platform](#51-x86arm-pcie-platform)
    - [5.2 SoC Platform](#52-soc-platform)
  - [6. Program Execution](#6-program-execution)
    - [6.1 Json Configuration Explanation](#61-json-configuration-explanation)
    - [6.2 Execution](#62-execution)
  - [7. Performance Testing](#7-performance-testing)


## 1. Introduction

This example is intended to illustrate how to use sophon-stream to quickly build license plate detection based on YOLOv5 and license plate recognition based on LPRNet.

**LPRNET License Plate Detection Source Code** (https://github.com/sirius-ai/LPRNet_Pytorch)

In this example, the preprocessing, inference, and post-processing of the YOLOv5 and LPRNet algorithms are separately computed on three elements. Multiple threads can be enabled within each element to ensure a certain level of detection efficiency.

## 2. Features

- Support for BM1684X, BM1684 (x86 PCIe, SoC), BM1688 (SoC)
- Support for multiple video streams
- Support for multi-threading

## 3. Prepare Models and Data

In the `scripts` directory, download scripts for relevant models and data are provided in [download.sh](./scripts/download.sh).

```bash
# Install 7z, unzip. Skip if already installed. For non-Ubuntu systems, use yum or other methods as appropriate.
sudo apt install p7zip
sudo apt install p7zip-full
sudo apt install unzip
chmod -R +x scripts/
./scripts/download.sh
```

After the script execution, the data and models directories will be generated in the current directory. The data directory contains the vehicle dataset, while the models directory contains the model files for YOLOv5 and LPRNet.

The downloaded models include:

```bash
./models/
├── lprnet
│   ├── BM1684
│   │   ├── lprnet_fp32_1b.bmodel
│   │   ├── lprnet_int8_1b.bmodel
│   │   └── lprnet_int8_4b.bmodel
│   ├── BM1684X
│   │   ├── lprnet_fp16_1b.bmodel
│   │   ├── lprnet_fp32_1b.bmodel
│   │   ├── lprnet_int8_1b.bmodel
│   │   └── lprnet_int8_4b.bmodel
│   ├── BM1688
│   │   ├── lprnet_fp32_1b.bmodel
│   │   └── lprnet_int8_1b.bmodel
│   ├── onnx
│   │   ├── lprnet_1b.onnx
│   │   └── lprnet_4b.onnx
│   └── torch
│       ├── Final_LPRNet_model.pth
│       └── LPRNet_model.torchscript
└── yolov5s-licensePLate
    ├── BM1684
    │   ├── yolov5s_v6.1_license_3output_fp32_1b.bmodel
    │   ├── yolov5s_v6.1_license_3output_fp32_4b.bmodel
    │   └── yolov5s_v6.1_license_3output_int8_1b.bmodel
    ├── BM1684X
    │   ├── yolov5s_v6.1_license_3output_fp32_1b.bmodel
    │   ├── yolov5s_v6.1_license_3output_fp32_4b.bmodel
    │   └── yolov5s_v6.1_license_3output_int8_1b.bmodel
    └── BM1688
        ├── yolov5s_v6.1_license_3output_fp32_1b_2core.bmodel
        ├── yolov5s_v6.1_license_3output_fp32_1b.bmodel
        ├── yolov5s_v6.1_license_3output_fp32_4b_2core.bmodel
        ├── yolov5s_v6.1_license_3output_fp32_4b.bmodel
        ├── yolov5s_v6.1_license_3output_int8_1b_2core.bmodel
        ├── yolov5s_v6.1_license_3output_int8_1b.bmodel
        ├── yolov5s_v6.1_license_3output_int8_4b_2core.bmodel
        └── yolov5s_v6.1_license_3output_int8_4b.bmodel
```

Model Description:

The LPRNet model mentioned above is ported from [LNRNet_Pytorch](https://github.com/sirius-ai/LPRNet_Pytorch). The yolov5s-licensePLate model is trained on the Green License Plate dataset.

Downloaded data includes:

```bash
./datasets
├── coco.names
└── test // Vehicle dataset for testing
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

### 6.1 Json Configuration Explanation

Various parameters in the license_plate_recognition demo are located in the [config](./config/) directory, structured as follows:

```bash
./config
├── converger.json
├── decode.json
├── distributor_time_class.json
├── engine_group.json
├── engine.json
├── license_plate_recognition_demo.json
├── lprnet_group.json
├── lprnet_infer.json
├── lprnet_post.json
├── lprnet_pre.json
├── yolov5_group.json
├── yolov5_infer.json
├── yolov5_post.json
└── yolov5_pre.json
```

Among them, [license_plate_recognition.json](./config/license_plate_recognition.json) is the overall configuration file for the example, managing information such as input streams. Multiple channels can be supported on a single image, with the channels parameter configuring the number of input channels, sample_interval setting the frame skip interval, and loop_num setting the number of loop plays. The channel section includes information such as stream URL.

In cases where the channel_id property is not specified in the configuration file, the demo will default to assigning channel_id for each data channel, starting from 0.

```json
{
    "channels": [
      {
        "channel_id": 0,
        "url": "../license_plate_recognition/data/test",
        "source_type": "IMG_DIR",
        "loop_num": 1,
        "fps": -1
      },
      {
        "channel_id": 1,
        "url": "../license_plate_recognition/data/test",
        "source_type": "IMG_DIR",
        "loop_num": 1,
        "fps": -1
      },
      {
        "channel_id": 2,
        "url": "../license_plate_recognition/data/test",
        "source_type": "IMG_DIR",
        "loop_num": 1,
        "fps": -1
      },
      {
        "channel_id": 3,
        "url": "../license_plate_recognition/data/test",
        "source_type": "IMG_DIR",
        "loop_num": 1,
        "fps": -1
      }
    ],
    "class_names": "../license_plate_recognition/data/coco.names",
    "download_image": true,
    "draw_func_name": "draw_license_plate_recognition_results",
    "engine_config_path": "../license_plate_recognition/config/engine_group.json"
}
```

[engine.json](./config/engine.json) contains configuration information for the graph, and this configuration is unlikely to change once determined.

In this file, it is necessary to initialize information for each element and the connection between elements. The element_id is unique and serves as an identifier. The element_config points to the detailed configuration file address of the element, port_id is the input/output port number of the element, and in cases of multiple inputs or outputs, input/output numbers should not be duplicated. is_src flags whether the current port is the input port for the entire graph, and is_sink identifies whether the current port is the output port for the entire graph. 
Connection specifies the connection method between all elements, determined by element_id and port_id.

Configuration files like [lprnet_pre.json](./config/lprnet_pre.json) detail the configuration for a specific element, setting model parameters, dynamic library paths, thresholds, and other information. This configuration file does not need to specify the `id` and `device_id` fields; the example will pass the `element_id` and `device_id` specified in `engine.json`. Among them, `thread_number` is the number of working threads within the element; one thread corresponds to one data queue. In the case of multiple inputs, it is necessary to set the number of data queues reasonably to ensure that the thread workload is even and reasonable.

```json
{
    "configure": {
        "model_path": "../license_plate_recognition/models/lprnet/BM1684X/lprnet_fp32_1b.bmodel",
        "stage": [
            "pre"
        ]
    },
    "shared_object": "../../build/lib/liblprnet.so",
    "name": "lprnet",
    "side": "sophgo",
    "thread_number": 1
}
```

### 6.2 Execution

For the PCIe platform, you can directly run the test on the PCIe platform. For the SoC platform, you need to copy the dynamically linked libraries, executable files, required models, and test data generated by cross-compilation to the SoC platform for testing.

On the SoC platform, the directory structure of dynamic libraries, executable files, configuration files, models, and video data should be consistent with the original sophon-stream repository.

The parameters and running methods for testing are the same. The following mainly introduces the PCIe mode.

1. Run the executable file, pay attention to providing the region.
```bash
./main --demo_config_path=../license_plate_recognition/config/license_plate_recognition_demo.json
```

The inference results are saved in the /build/results path.

When saving images is disabled, the inference result for 1 image on 1684X PCIe is as follows. The performance on PCIe may vary significantly due to different CPUs:

```bash
 total time cost 24724501 us.
frame count is 5007 | fps is 202.512 fps.
```

## 7. Performance Testing
This example is for reference in the process and currently does not have optimal performance data.

