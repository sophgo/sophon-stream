# YOLOv5 Demo

English | [简体中文](README.md)

## Catalogs
- [YOLOv5 Demo](#yolov5-demo)
  - [Catalogs](#catalogs)
  - [1. Introduction](#1-introduction)
  - [2. Feature](#2-feature)
  - [3. Prepare Models and Data](#3-prepare-models-and-data)
  - [4. Prepare Environment](#4-prepare-environment)
    - [4.1 x86/arm PCIe Platform](#41-x86arm-pcie-platform)
    - [4.2 SoC Platform](#42-soc-platform)
  - [5. Program Compilation](#5-program-compilation)
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

**source code** (https://github.com/ultralytics/yolov5) v6.1 version

In this example, the pre-processing, inference, and post-processing of the YOLOv5 algorithm are computed on three separate elements, allowing multiple threads to be utilized within each element, ensuring a certain level of detection efficiency.

## 2. Feature

* Supports BM1684X, BM1684(x86 PCIe、SoC), supports BM1688(arm PCIe、SoC).
* Supports tpu_kernel post-process on BM1684X.
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
│   ├── yolov5s_v6.1_3output_fp32_1b.bmodel         # FP32 BModel for BM1684, with a batch size of 1. Post-processing takes place on the CPU.
│   ├── yolov5s_v6.1_3output_int8_1b.bmodel         # INT8 BModel for BM1684, with a batch size of 1. Post-processing takes place on the CPU.
│   └── yolov5s_v6.1_3output_int8_4b.bmodel         # INT8 BModel for BM1684, with a batch size of 4. Post-processing takes place on the CPU.
├── BM1684X
│   ├── yolov5s_v6.1_3output_fp16_1b.bmodel         # FP16 BModel for BM1684X, with a batch size of 1. Post-processing takes place on the CPU.
│   ├── yolov5s_v6.1_3output_fp32_1b.bmodel         # FP32 BModel for BM1684X, with a batch size of 1. Post-processing takes place on the CPU.
│   ├── yolov5s_v6.1_3output_int8_1b.bmodel         # INT8 BModel for BM1684X, with a batch size of 1. Post-processing takes place on the CPU.
│   └── yolov5s_v6.1_3output_int8_4b.bmodel         # INT8 BModel for BM1684X, with a batch size of 4. Post-processing takes place on the CPU.
├── BM1684X_tpukernel
│   ├── yolov5s_tpukernel_fp16_1b.bmodel            # FP16 BModel for BM1684X, with a batch size of 1. Post-processing utilizes the tpu_kernel.
│   ├── yolov5s_tpukernel_fp32_1b.bmodel            # FP32 BModel for BM1684X, with a batch size of 1. Post-processing utilizes the tpu_kernel.
│   ├── yolov5s_tpukernel_int8_1b.bmodel            # INT8 BModel for BM1684X, with a batch size of 1. Post-processing utilizes the tpu_kernel.
│   └── yolov5s_tpukernel_int8_4b.bmodel            # INT8 BModel for BM1684X, with a batch size of 4. Post-processing utilizes the tpu_kernel.
├── BM1688
│   ├── yolov5s_v6.1_3output_fp16_1b_2core.bmodel   # FP16 BModel for BM1688, batch_size=1, num_core=2
│   ├── yolov5s_v6.1_3output_fp16_1b.bmodel         # FP16 BModel for BM1688, batch_size=1, num_core=1
│   ├── yolov5s_v6.1_3output_fp32_1b_2core.bmodel   # FP32 BModel for BM1688, batch_size=1, num_core=2
│   ├── yolov5s_v6.1_3output_fp32_1b.bmodel         # FP32 BModel for BM1688, batch_size=1, num_core=1
│   ├── yolov5s_v6.1_3output_int8_1b_2core.bmodel   # INT8 BModel for BM1688, batch_size=1, num_core=2
│   ├── yolov5s_v6.1_3output_int8_1b.bmodel         # INT8 BModel for BM1688, batch_size=1, num_core=1
│   ├── yolov5s_v6.1_3output_int8_4b_2core.bmodel   # INT8 BModel for BM1688, batch_size=4, num_core=2
│   └── yolov5s_v6.1_3output_int8_4b.bmodel         # INT8 BModel for BM1688, batch_size=4, num_core=1
└── CV186X
    ├── yolov5s_v6.1_3output_fp16_1b.bmodel         # FP16 BModel for CV186X，batch_size=1
    ├── yolov5s_v6.1_3output_fp32_1b.bmodel         # FP32 BModel for CV186X，batch_size=1
    ├── yolov5s_v6.1_3output_int8_1b.bmodel         # INT8 BModel for CV186X，batch_size=1
    └── yolov5s_v6.1_3output_int8_4b.bmodel         # INT8 BModel for CV186X，batch_size=4
```

Model description:

The above models are ported from the official [yolov5 repository](https://github.com/ultralytics/yolov5). The plugin configuration includes `mean=[0,0,0]`, `std=[255,255,255]`, supporting 80-class detection tasks from the COCO dataset.


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

In the YOLOv5 demo, various parameters for each section are located in [config](./config/) directory, structured as follows:

```bash
./config/
├── decode.json                 # decoding configuration
├── engine_group.json           # sophon-stream Simplified graph configuration
├── engine.json                 # sophon-stream graph configuration requires separate configuration for pre-processing, inference, and post-processing files.
├── yolov5_classthresh_roi_example.json  # reference configuration file for setting thresholds per category in YOLOv5. Please note that setting thresholds per category is only supported in non-tpu_kernel post-processing mode
├── yolov5_demo.json            # input configuration file for the demo
├── yolov5_group.json           # A simplified YOLOv5 configuration file that combines pre-processing, inference, and post-processing into one configuration file.
├── yolov5_infer.json           # YOLOv5 inference configuration file
├── yolov5_post.json            # YOLOv5 post-processing configuration file
└── yolov5_pre.json             # YOLOv5 pre-processing configuration file
```

Indeed, [yolov5_demo.json](./config/yolov5_demo.json) is the overall configuration file for the example, managing input streams and other information. Multiple data inputs can be supported on a single graph, where the `channels` parameter configures the number of input channels, `sample_interval` sets the frame skipping rate, and `loop_num` sets the number of looped plays. The `channel` section contains video stream information such as the URL. The `download_image` parameter controls whether to save the inference results. If set to `false`, results will not be saved. If set to `true`, they will be saved in the `/build/results` directory.

In the configuration file, when the `channel_id` attribute is not specified, the demo will assign default `channel_id` values starting from 0 for each data channel.

```json
{
  "channels": [
    {
      "channel_id": 2,
      "url": "../yolov5/data/videos/test_car_person_1080P.avi",
      "source_type": "VIDEO",
      "sample_interval": 1,
      "loop_num": 1,
      "fps": -1
    },
    {
      "channel_id": 3,
      "url": "../yolov5/data/videos/test_car_person_1080P.avi",
      "source_type": "VIDEO",
      "sample_interval": 1,
      "loop_num": 1,
      "fps": -1
    },
    {
      "channel_id": 20,
      "url": "../yolov5/data/videos/test_car_person_1080P.avi",
      "source_type": "VIDEO",
      "sample_interval": 1,
      "loop_num": 1,
      "fps": -1
    },
    {
      "channel_id": 30,
      "url": "../yolov5/data/videos/test_car_person_1080P.avi",
      "source_type": "VIDEO",
      "sample_interval": 1,
      "loop_num": 1,
      "fps": -1
    }
  ],
  "class_names": "../yolov5/data/coco.names",
  "download_image": true,
  "draw_func_name": "draw_yolov5_results",
  "engine_config_path": "../yolov5/config/engine_group.json"
}
```

[engine_group.json](./config/engine_group.json) contains configuration information for graphs, which, once set, typically remain unchanged.

Here's an excerpt from the configuration file as an example: Within this file, it's necessary to initialize information for each element and specify the connections between elements. The `element_id` serves as a unique identifier. `element_config` points to the detailed configuration file for that element. `port_id` denotes the input/output port number for the element. And in cases of multiple inputs or outputs, these numbers should not be duplicated. `is_src` denotes whether the current port is the input port for the entire graph, while `is_sink` identifies whether the port is the output for the whole graph. `connection` determines how elements are connected, using `element_id` and `port_id` for identification.

```json
[
    {
        "graph_id": 0,
        "device_id": 0,
        "graph_name": "yolov5",
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
                    ]
                }
            },
            {
                "element_id": 5001,
                "element_config": "../config/yolov5_group.json",
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


[yolov5_group.json](./config/yolov5_group.json) and similar configuration files detail the configuration specifics for a particular element, setting parameters such as model settings, dynamic library paths, thresholds, and more. These configuration files don't require specifying the `id` or `device_id` fields, as the demo will pass in the `element_id` and `device_id` specified in the engine_group.json.

Among these configurations, `thread_number` specifies the number of working threads within the `element`. Each thread corresponds to a data queue. In scenarios with multiple inputs, it's essential to set the number of data queues reasonably to ensure an even and adequate workload distribution across threads.

When `use_tpu_kernel` is set to `true`, it will utilize the tpu_kernel post-processing(using tpu to do post process). Note that tpu_kernel post-processing is only supported on BM1684X devices.

```json
{
    "configure": {
        "model_path": "../data/models/BM1684X_tpukernel/yolov5s_tpukernel_int8_1b.bmodel",
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
        ],
        "use_tpu_kernel": true
    },
    "shared_object": "../../../build/lib/libyolov5.so",
    "name": "yolov5_group",
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
./main --demo_config_path=../yolov5/config/yolov5_demo.json
```

The running results of 4 video streams are as follows
```bash
total time cost 12150217 us.
frame count is 2848 | fps is 234.399 fps.
```

## 7. Performance Testing

Currently, the YOLOv5 example supports inference on BM1684X and BM1684 in PCIe and SoC modes, and supports inference on BM1688 in SoC mode.

Modifications in JSON configurations might be necessary when switching between different devices, such as adjusting model paths, input channels, etc. Refer to section 6.1 for JSON configuration methods and section 6.2 for program execution methods.

Due to significant differences in CPU capabilities among PCIe devices, performance data is not meaningful. Therefore, only provide the test results for SOC mode.

The tested video is `elevator-1080p-25fps-4000kbps.h264`. The compilation was done in Release mode. The results are as follows:


| Device | Number of Channels | Algorithm Thread Count | CPU Utilization (%) | System Memory (M) | Peak System Memory (M) | TPU Utilization (%) | Device Memory (M) | Peak Device Memory (M) | Average FPS | Peak FPS |
|----|----|-----|-----|-----|-----|-----|-----|-----|-----|-----|
|SE7|8|4-4-4|136.64|200.95|206.18|100.00|1857.94|2067.00|255.17|265.92|
|SE5-16|4|4-4-4|218.74|187.12|191.89|96.95|1897.06|2151.00|120.84|141.66|
|SE5-8|3|3-3-3|137.50|145.81|149.01|94.89|1273.70|1437.00|80.38|90.00|

> **Test Description**:
1. Performance test results exhibit certain fluctuations; it's advisable to conduct multiple tests and calculate the average.
2. Both BM1684 and BM1684X SoC devices utilize an 8-core ARM A53 processor, offering 42320 DMIPS @ 2.3GHz.
3. All aforementioned performance tests are based on the INT8 model.
4. Running models with a batch size of 4 on the BM1684 device can achieve higher FPS.
5. On the BM1684X device, utilizing a batch size of 1 for the model and enabling the tpu_kernel for post-processing can yield higher FPS.
6. For the settings of input channels and algorithm thread count in the table, please refer to [JSON configuration explanation](#61-json-configuration). CPU utilization and system memory can be checked using the `top` command. TPU utilization and device memory can be checked using the `bm-smi` command. FPS can be obtained from the logs printed during program execution.
7. Performance testing is not currently available on the BM1688 device.