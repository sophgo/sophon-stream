# YOLOV5-FASTPOSE-POSEC3D Demo

English | [简体中文](README.md)

## Catalogs
- [YOLOV5-FASTPOSE-POSEC3D Demo](#yolov5-fastpose-posec3d-demo)
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

This example demonstrates how to use sophon-stream to quickly build a video pose recognition application.

The connection method for this example plugin is shown in the following diagram.

![process](./pics/posec3d.jpg)

**Source Code** (https://github.com/MVIG-SJTU/AlphaPose和https://github.com/open-mmlab/mmaction2) 

In this example, the pre-processing, inference, and post-processing of the YOLOv5, fastpose and posec3d algorithm are computed on 9 separate elements, allowing multiple threads to be utilized within each element, ensuring a certain level of detection efficiency.

## 2. Feature

* Supports BM1684X(x86 PCIe、SoC)
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
│   ├── yolov5s_v6.1_3output_fp32_1b.bmodel         # YOLOV5 FP32 BModel for BM1684，batch_size=1，post process on CPU
│   ├── yolov5s_v6.1_3output_int8_1b.bmodel         # YOLOV5 INT8 BModel for BM1684，batch_size=1，post process on CPU
│   └── yolov5s_v6.1_3output_int8_4b.bmodel         # YOLOV5 INT8 BModel for BM1684，batch_size=4，post process on CPU
├── BM1684X
│   ├── fast_res50_256x192_coco17_1b_fp16.bmodel    # FASTPOSE FP16 Bmodel for BM1684X，batch_size=1，17 key points
│   ├── fast_res50_256x192_coco17_1b_fp32.bmodel    # FASTPOSE FP32 Bmodel for BM1684X，batch_size=1，17 key points
│   ├── fast_res50_256x192_coco17_1b_int8.bmodel    # FASTPOSE INT8 Bmodel for BM1684X，batch_size=1，17 key points
│   ├── fast_res50_256x192_coco17_4b_int8.bmodel    # FASTPOSE INT8 Bmodel for BM1684X，batch_size=4，17 key points
│   ├── posec3d_gym_fp16.bmodel                     # POSEC3D FP16 Bmodel for BM1684X，gym 99 classes
│   ├── posec3d_gym_fp32.bmodel                     # POSEC3D FP32 Bmodel for BM1684X，gym 99 classes
│   ├── posec3d_ntu60_fp16.bmodel                   # POSEC3D FP16 Bmodel for BM1684X，ntu 60 classes
│   ├── posec3d_ntu60_fp32.bmodel                   # POSEC3D FP32 Bmodel for BM1684X，ntu 60 classes
│   ├── posec3d_ntu60_int8.bmodel                   # POSEC3D INT8 Bmodel for BM1684X，ntu 60 classes
│   ├── yolov5s_v6.1_3output_fp16_1b.bmodel         # YOLOV5 FP16 BModel for BM1684X，batch_size=1，post process on CPU
│   ├── yolov5s_v6.1_3output_fp32_1b.bmodel         # YOLOV5 FP32 BModel for BM1684X，batch_size=1，post process on CPU
│   ├── yolov5s_v6.1_3output_int8_1b.bmodel         # YOLOV5 INT8 BModel for BM1684X，batch_size=1，post process on CPU
│   └── yolov5s_v6.1_3output_int8_4b.bmodel         # YOLOV5 INT8 BModel for BM1684X，batch_size=4，post process on CPU
└── BM1684X_tpukernel
    ├── yolov5s_tpukernel_fp16_1b.bmodel            # YOLOV5 FP16 BModel for BM1684X，batch_size=1，post process with tpu_kernel
    ├── yolov5s_tpukernel_fp32_1b.bmodel            # YOLOV5 FP32 BModel for BM1684X，batch_size=1，post process with tpu_kernel
    ├── yolov5s_tpukernel_int8_1b.bmodel            # YOLOV5 INT8 BModel for BM1684X，batch_size=1，post process with tpu_kernel
    └── yolov5s_tpukernel_int8_4b.bmodel            # YOLOV5 INT8 BModel for BM1684X，batch_size=4，post process with tpu_kernel
```

The downloaded data include:

```bash
./videos
├── demo_skeleton.mp4                         # Human Detection + Key Point Detection + Behavior Recognition Test Video
├── S017C001P003R001A001_rgb.avi              # Human Detection + Key Point Detection + Behavior Recognition Test Video
├── S017C001P003R002A008_rgb.avi              # Human Detection + Key Point Detection + Behavior Recognition Test Video
└── test.mp4                                  # Human Detection + Key Point Detection Test Video
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

In the yolov5-fastpose demo, various parameters for each section are located in [config](./config/) directory, structured as follows:

```bash
./config/
├── decode.json                         # decoding configuration
├── engine_group_alphapose.json         # sophon-stream human detect + key points detect graph configuration
├── engine_group.json                   # sophon-stream human detect + key points detect + behavior recognition graph configuration
├── fastpose_group.json                 # fastpose configuration
├── fastpose_infer.json                 # fastpose inference configuration
├── fastpose_post.json                  # fastpose post-process configuration
├── fastpose_pre.json                   # fastpose pre-process configuration
├── posec3d_group.json                  # posec3d configuration
├── yolov5_fastpose_posec3d_demo.json   # yolov5-fastpose-posec3d demo configuration
├── yolov5_group.json                   # yolov5 configuration
├── yolov5_infer.json                   # yolov5 inference configuration file
├── yolov5_post.json                    # yolov5 post-processing configuration file
└── yolov5_pre.json                     # yolov5 pre-processing configuration file
```

[engine_group_alphapose.json](./config/engine_group_alphapose.json) is the configuration file for the AlphaPose algorithm accompanying YOLOv5 as a detector. It outputs bounding boxes and key points of human bodies, as shown in the following image:

<img src="./pics/yolov5_fastpose.jpg" width="800">

Built on top of this, the posec3d behavior recognition model is added to form the configuration file [engine_group.json](./config/engine_group.json). It outputs human detection boxes, key points, and behavior categories, as illustrated below:

<img src="./pics/yolov5_fastpose_posec3d.jpg" width="800">

Note that in this image, posec3d takes 72 frames as input, and the final category label is placed on the 0th frame. The video is from the demo_skeleton.mp4 downloaded by the download.sh script.

Among them, [yolov5_fastpose_posec3d_demo.json](./config/yolov5_fastpose_posec3d_demo.json) is the overall configuration file for the demo, managing input streams and other information. Multiple streams of data can be supported on one image, and the `channels` section includes information such as the stream URL for each channel.

In cases where the `channel_id` property is not specified in the configuration file, the `channel_id` for each data stream in the demo is assigned a default value starting from 0.

The `heatmap_loss` parameter determines the post-processing flow of fastpose. Currently, based on the official [model configuration](https://github.com/MVIG-SJTU/AlphaPose/blob/master/docs/MODEL_ZOO.md), only `MSELoss` is supported.


```json
{
  "channels": [
    {
      "channel_id": 0,
      "url": "../yolov5_fastpose_posec3d/data/nturgb+d_rgb/S017C001P003R002A008_rgb.avi",
      "source_type": "VIDEO",
      "sample_interval": 1,
      "loop_num": 1,
      "fps": -1
    }
  ],
  "class_names": "../yolov5_fastpose_posec3d/data/coco.names",
  "download_image": true,
  "draw_func_name": "draw_yolov5_fastpose_posec3d_results",
  "engine_config_path": "../yolov5_fastpose_posec3d/config/engine_group.json",
  "heatmap_loss": "MSELoss"
}
```

[engine_group.json](./config/engine_group.json) contains configuration information for graphs, which, once set, typically remain unchanged.

Here's an excerpt from the configuration file as an example: Within this file, it's necessary to initialize information for each element and specify the connections between elements. The `element_id` serves as a unique identifier. `element_config` points to the detailed configuration file for that element. `port_id` denotes the input/output port number for the element. And in cases of multiple inputs or outputs, these numbers should not be duplicated. `is_src` denotes whether the current port is the input port for the entire graph, while `is_sink` identifies whether the port is the output for the whole graph. `connection` determines how elements are connected, using `element_id` and `port_id` for identification.

```json
[
    {
        "graph_id": 0,
        "device_id": 0,
        "graph_name": "yolov5_fastpose_posec3d",
        "elements": [
            {
                "element_id": 5000,
                "element_config": "../yolov5_fastpose_posec3d/config/decode.json",
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
                "element_config": "../yolov5_fastpose_posec3d/config/yolov5_group.json",
                "inner_elements_id": [10001, 10002, 10003]
            },
            {
                "element_id": 6001,
                "element_config": "../yolov5_fastpose_posec3d/config/fastpose_group.json",
                "inner_elements_id": [20001, 20002, 20003]
            },
            {
                "element_id": 7001,
                "element_config": "../yolov5_fastpose_posec3d/config/posec3d_group.json",
                "inner_elements_id": [30001, 30002, 30003],
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
                "dst_element_id": 6001,
                "dst_port": 0
            },
            {
                "src_element_id": 6001,
                "src_port": 0,
                "dst_element_id": 7001,
                "dst_port": 0
            }
        ]
    }
]
```

[fastpose_group.json](./config/fastpose_group.json) and similar configuration files detail the configuration specifics for a particular element, setting parameters such as model settings, dynamic library paths, thresholds, and more. These configuration files don't require specifying the `id` or `device_id` fields, as the demo will pass in the `element_id` and `device_id` specified in the engine_group.json.

Among these configurations, `thread_number` specifies the number of working threads within the `element`. Each thread corresponds to a data queue. In scenarios with multiple inputs, it's essential to set the number of data queues reasonably to ensure an even and adequate workload distribution across threads.

When `use_tpu_kernel` is set to `true`, it will utilize the tpu_kernel post-processing(using tpu to do post process). Note that tpu_kernel post-processing is only supported on BM1684X devices.

```json
{
    "configure": {
        "model_path": "../yolov5_fastpose_posec3d/data/models/BM1684X/halpe26_fast_res50_256x192_int8_1b.bmodel",
        "stage": [
            "pre"
        ],
        "heatmap_loss": "MSELoss",
        "area_thresh": 0.0
    },
    "shared_object": "../../build/lib/libfastpose.so",
    "name": "fastpose",
    "side": "sophgo",
    "thread_number": 1
}
```

### 6.2 Execute

For PCIe platforms, you can directly run tests on the PCIe platform. For SoC platforms, you'll need to copy the dynamically linked libraries, executable files, required models, and test data generated from cross-compilation to the SoC platform for testing.

On the SoC platform, maintain a directory structure for dynamic libraries, executable files, configuration files, models, and video data consistent with the original sophon-stream repository's structure.

The parameters for testing and the method of execution remain consistent. Therefore, it'll primarily explain in terms of PCIe mode.

Run the executable file
```bash
./main --demo_config_path=../yolov5_fastpose_posec3d/config/yolov5_fastpose_posec3d_demo.json
```

The running results of three video streams are as follows
```bash
 total time cost 5453888 us.
frame count is 291 | fps is 53.3564 fps.
```

## 7. Performance Testing

Performance varies greatly from video to video, please subject to actual.