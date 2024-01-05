# Target Detection-Tracking-Distribution-Attribute Recognition Demo

English | [简体中文](README.md)

## Catalogs
- [Target Detection-Tracking-Distribution-Attribute Recognition Demo](#target-detection-tracking-distribution-attribute-recognition-demo)
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

This example demonstrates how to use sophon-stream to quickly build a complex application that contain multiple algorithms and send to different branches by category.

The connection method for this example plugin is shown in the following diagram.

![distributor.png](pics/distributor.png)

## 2. Feature

* Use yolov5 for detection;
* Use bytetrack for track;
* Use resnet18 for classification;
* Supports BM1684X(x86 PCIe, SoC) and BM1684(x86 PCIe, SoC, arm PCIe);
* Supports multiple video streams;
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
./models
├── BM1684
│   ├── resnet50_fp32_1b.bmodel                     # RESNET50 FP32 Bmodel for BM1684，batch_size=1，imagenet
│   ├── resnet50_fp32_4b.bmodel                     # RESNET50 FP32 Bmodel for BM1684，batch_size=4，imagenet
│   ├── resnet50_int8_1b.bmodel                     # RESNET50 INT8 Bmodel for BM1684，batch_size=1，imagenet
│   ├── resnet50_int8_4b.bmodel                     # RESNET50 INT8 Bmodel for BM1684，batch_size=4，imagenet
│   ├── resnet_pedestrian_gender_fp32_1b.bmodel     # RESNET18 FP32 Bmodel for BM1684，batch_size=1，pedestrian gender classification
│   ├── resnet_pedestrian_gender_fp32_4b.bmodel     # RESNET18 FP32 Bmodel for BM1684，batch_size=4，pedestrian gender classification
│   ├── resnet_pedestrian_gender_int8_1b.bmodel     # RESNET18 INT8 Bmodel for BM1684，batch_size=1，pedestrian gender classification
│   ├── resnet_pedestrian_gender_int8_4b.bmodel     # RESNET18 INT8 Bmodel for BM1684，batch_size=4，pedestrian gender classification
│   ├── resnet_vehicle_color_fp32_1b.bmodel         # RESNET18 FP32 Bmodel for BM1684，batch_size=1，vehicle color classification
│   ├── resnet_vehicle_color_fp32_4b.bmodel         # RESNET18 FP32 Bmodel for BM1684，batch_size=4，vehicle color classification
│   ├── resnet_vehicle_color_int8_1b.bmodel         # RESNET18 INT8 Bmodel for BM1684，batch_size=1，vehicle color classification
│   ├── resnet_vehicle_color_int8_4b.bmodel         # RESNET18 INT8 Bmodel for BM1684，batch_size=4，vehicle color classification
│   ├── yolov5s_v6.1_3output_fp32_1b.bmodel         # YOLOV5 FP32 BModel for BM1684，batch_size=1，post-process on CPU
│   ├── yolov5s_v6.1_3output_int8_1b.bmodel         # YOLOV5 INT8 BModel for BM1684，batch_size=1，post-process on CPU
│   └── yolov5s_v6.1_3output_int8_4b.bmodel         # YOLOV5 INT8 BModel for BM1684，batch_size=4，post-process on CPU
├── BM1684X
│   ├── resnet50_fp16_1b.bmodel                     # RESNET50 FP16 Bmodel for BM1684X，batch_size=1，imagenet
│   ├── resnet50_fp32_1b.bmodel                     # RESNET50 FP32 Bmodel for BM1684X，batch_size=1，imagenet
│   ├── resnet50_fp32_4b.bmodel                     # RESNET50 FP32 Bmodel for BM1684X，batch_size=4，imagenet
│   ├── resnet50_int8_1b.bmodel                     # RESNET50 INT8 Bmodel for BM1684X，batch_size=1，imagenet
│   ├── resnet50_int8_4b.bmodel                     # RESNET50 INT8 Bmodel for BM1684X，batch_size=4，imagenet
│   ├── resnet_pedestrian_gender_fp16_1b.bmodel     # RESNET18 FP16 Bmodel for BM1684X，batch_size=1，pedestrian gender classification
│   ├── resnet_pedestrian_gender_fp32_1b.bmodel     # RESNET18 FP32 Bmodel for BM1684X，batch_size=1，pedestrian gender classification
│   ├── resnet_pedestrian_gender_fp32_4b.bmodel     # RESNET18 FP32 Bmodel for BM1684X，batch_size=4，pedestrian gender classification
│   ├── resnet_pedestrian_gender_int8_1b.bmodel     # RESNET18 INT8 Bmodel for BM1684X，batch_size=1，pedestrian gender classification
│   ├── resnet_pedestrian_gender_int8_4b.bmodel     # RESNET18 INT8 Bmodel for BM1684X，batch_size=4，pedestrian gender classification
│   ├── resnet_vehicle_color_fp16_1b.bmodel         # RESNET18 FP16 Bmodel for BM1684X，batch_size=1，vehicle color classification
│   ├── resnet_vehicle_color_fp32_1b.bmodel         # RESNET18 FP32 Bmodel for BM1684X，batch_size=1，vehicle color classification
│   ├── resnet_vehicle_color_fp32_4b.bmodel         # RESNET18 FP32 Bmodel for BM1684X，batch_size=4，vehicle color classification
│   ├── resnet_vehicle_color_int8_1b.bmodel         # RESNET18 INT8 Bmodel for BM1684X，batch_size=1，vehicle color classification
│   ├── resnet_vehicle_color_int8_4b.bmodel         # RESNET18 INT8 Bmodel for BM1684X，batch_size=4，vehicle color classification
│   ├── yolov5s_v6.1_3output_fp16_1b.bmodel         # YOLOV5 FP16 BModel for BM1684X，batch_size=1，post-process on CPU
│   ├── yolov5s_v6.1_3output_fp32_1b.bmodel         # YOLOV5 FP32 BModel for BM1684X，batch_size=1，post-process on CPU
│   ├── yolov5s_v6.1_3output_int8_1b.bmodel         # YOLOV5 INT8 BModel for BM1684X，batch_size=1，post-process on CPU
│   └── yolov5s_v6.1_3output_int8_4b.bmodel         # YOLOV5 INT8 BModel for BM1684X，batch_size=4，post-process on CPU
└── BM1684X_tpukernel
    ├── yolov5s_tpukernel_fp16_1b.bmodel            # YOLOV5 FP16 BModel for BM1684X，batch_size=1，post-process with tpu_kernel
    ├── yolov5s_tpukernel_fp32_1b.bmodel            # YOLOV5 FP32 BModel for BM1684X，batch_size=1，post-process with tpu_kernel
    ├── yolov5s_tpukernel_int8_1b.bmodel            # YOLOV5 INT8 BModel for BM1684X，batch_size=1，post-process with tpu_kernel
    └── yolov5s_tpukernel_int8_4b.bmodel            # YOLOV5 INT8 BModel for BM1684X，batch_size=4，post-process with tpu_kernel
```

The downloaded data include:

```bash
videos/
├── carvana_video.mp4   # test video
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

Configuration files are located on [config](./config/) directory, structured as follows:

```bash
./config
├── bytetrack.json                                                    # bytetrack configuration
├── converger.json                                                    # converger element configuration
├── decode.json                                                       # decoding configuration
├── distributor_class.json                                            # distribute every frame in class
├── distributor_frame_class.json                                      # distribute frame with frame interval in class
├── distributor_frame.json                                            # distribute full frame with frame interval
├── distributor_time_class.json                                       # distribute frame with time interval (default)
├── distributor_time.json                                             # distribute full frame with time interval
├── engine.json                                                       # graph configuration
├── engine_group.json                                                 # sophon-stream Simplified graph configuration
├── resnet_car.json                                                   # resnet vehicle color classification
├── resnet_person.json                                                # resnet pedestrian gender classification
├── yolov5_bytetrack_distributor_resnet_converger_demo.json           # demo configuration
├── yolov5_group.json                                                 # A simplified YOLOv5 configuration file that combines pre-processing, inference, and post-processing into one configuration file.
├── yolov5_infer.json                                                 # yolov5 inference configuration
├── yolov5_post.json                                                  # yolov5 post-process configuration
└── yolov5_pre.json                                                   # yolov5 pre-process configuration
```

Indeed, [yolov5_bytetrack_distributor_resnet_converger_demo.json](./config/yolov5_bytetrack_distributor_resnet_converger_demo.json) is the overall configuration file for the example, managing input streams and other information. Multiple data inputs can be supported on a single graph, where the `channels` parameter configures the number of input channels, `sample_interval` sets the frame skipping rate, and `loop_num` sets the number of looped plays. The `channel` section contains video stream information such as the URL. The `download_image` parameter controls whether to save the inference results. If set to `false`, results will not be saved. If set to `true`, they will be saved in the `/build/results` directory.

In the configuration file, when the `channel_id` attribute is not specified, the demo will assign default `channel_id` values starting from 0 for each data channel.

```json
{
  "channels": [
    {
      "channel_id": 0,
      "url": "../yolov5_bytetrack_distributor_resnet_converger/data/videos/traffic.mp4",
      "source_type": "VIDEO",
      "loop_num": 1,
      "fps": 25
    },
    {
      "channel_id": 1,
      "url": "../yolov5_bytetrack_distributor_resnet_converger/data/videos/traffic.mp4",
      "source_type": "VIDEO",
      "loop_num": 1,
      "fps": 25
    },
    {
      "channel_id": 2,
      "url": "../yolov5_bytetrack_distributor_resnet_converger/data/videos/traffic.mp4",
      "source_type": "VIDEO",
      "loop_num": 1,
      "fps": 25
    },
    {
      "channel_id": 3,
      "url": "../yolov5_bytetrack_distributor_resnet_converger/data/videos/traffic.mp4",
      "source_type": "VIDEO",
      "loop_num": 1,
      "fps": 25
    }
  ],
  "class_names": "../yolov5_bytetrack_distributor_resnet_converger/data/coco.names",
  "car_attributes": "../yolov5_bytetrack_distributor_resnet_converger/data/car.attributes",
  "person_attributes": "../yolov5_bytetrack_distributor_resnet_converger/data/person.attributes",
  "download_image": true,
  "draw_func_name": "draw_yolov5_bytetrack_distributor_resnet_converger_results",
  "engine_config_path": "../yolov5_bytetrack_distributor_resnet_converger/config/engine_group.json"
}
```

[engine_group.json](./config/engine_group.json) contains configuration information for graphs, which, once set, typically remain unchanged.

Here's an excerpt from the configuration file as an example: Within this file, it's necessary to initialize information for each element and specify the connections between elements. The `element_id` serves as a unique identifier. `element_config` points to the detailed configuration file for that element. `port_id` denotes the input/output port number for the element. And in cases of multiple inputs or outputs, these numbers should not be duplicated. `is_src` denotes whether the current port is the input port for the entire graph, while `is_sink` identifies whether the port is the output for the whole graph. `connection` determines how elements are connected, using `element_id` and `port_id` for identification.

[yolov5_group.json](./config/yolov5_group.json) and similar configuration files detail the configuration specifics for a particular element, setting parameters such as model settings, dynamic library paths, thresholds, and more. These configuration files don't require specifying the `id` or `device_id` fields, as the demo will pass in the `element_id` and `device_id` specified in the engine_group.json.

Among these configurations, `thread_number` specifies the number of working threads within the `element`. Each thread corresponds to a data queue. In scenarios with multiple inputs, it's essential to set the number of data queues reasonably to ensure an even and adequate workload distribution across threads.

When `use_tpu_kernel` is set to `true`, it will utilize the tpu_kernel post-processing(using tpu to do post process). Note that tpu_kernel post-processing is only supported on BM1684X devices.

### 6.2 Execute

For PCIe platforms, you can directly run tests on the PCIe platform. For SoC platforms, you'll need to copy the dynamically linked libraries, executable files, required models, and test data generated from cross-compilation to the SoC platform for testing.

On the SoC platform, maintain a directory structure for dynamic libraries, executable files, configuration files, models, and video data consistent with the original sophon-stream repository's structure.

The parameters for testing and the method of execution remain consistent. Therefore, it'll primarily explain in terms of PCIe mode.

Run the executable file
```bash
./main --demo_config_path=../yolov5_bytetrack_distributor_resnet_converger/config/yolov5_bytetrack_distributor_resnet_converger_demo.json
```

The results are stored in the `./build/results` directory. This sample is configured by default to be sent to the resnet branch per second per category, and will save one frame per second in the results directory with the target box, track_id, and specific attributes drawn.

## 7. Performance Testing

Since the whole process depends on the input video fps and the drawing speed is slow, this routine does not provide performance test results for the time being, if you need the inference performance of each model, please go to the corresponding model routine to check.