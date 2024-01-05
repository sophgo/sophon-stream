# Detection-Track-UpStreaming Demo

English | [简体中文](README.md)

## Catalogs
- [Detection-Track-UpStreaming Demo](#detection-track-upstreaming-demo)
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

This sample is used to illustrate how to quickly build a video target tracking application using sophon-stream and push stream the algorithm results to output;

The connection method for this example plugin is shown in the following diagram.

![elements.jpg](pics/dec_det_track_osd_enc.png)

## 2. Feature

* Use yolox for detection;
* Use bytetrack for track;
* Supports BM1684X, BM1684(x86 PCIe、SoC), supports BM1688(SoC)
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

In the Detection-Track-UpStreaming Demo, various parameters for each section are located in [config](./config/) directory, structured as follows:

```json
{
  "channels": [
    {
      "channel_id": 2,
      "url": "../yolox_bytetrack_osd_encode/data/videos/mot17_01_frcnn.mp4",
      "source_type": "VIDEO",
      "loop_num": 1
    },
    {
      "channel_id": 3,
      "url": "../yolox_bytetrack_osd_encode/data/videos/mot17_03_frcnn.mp4",
      "source_type": "VIDEO",
      "loop_num": 1
    },
    {
      "channel_id": 20,
      "url": "../yolox_bytetrack_osd_encode/data/videos/mot17_06_frcnn.mp4",
      "source_type": "VIDEO",
      "loop_num": 1
    },
    {
      "channel_id": 30,
      "url": "../yolox_bytetrack_osd_encode/data/videos/mot17_08_frcnn.mp4",
      "source_type": "VIDEO",
      "loop_num": 1
    }
  ],
  "engine_config_path": "../yolox_bytetrack_osd_encode/config/engine_group.json"
}
```

[engine_group.json](./config/engine_group.json) contains configuration information for graphs, which, once set, typically remain unchanged.

Here's an excerpt from the configuration file as an example: Within this file, it's necessary to initialize information for each element and specify the connections between elements. The `element_id` serves as a unique identifier. `element_config` points to the detailed configuration file for that element. `port_id` denotes the input/output port number for the element. And in cases of multiple inputs or outputs, these numbers should not be duplicated. `is_src` denotes whether the current port is the input port for the entire graph, while `is_sink` identifies whether the port is the output for the whole graph. `connection` determines how elements are connected, using `element_id` and `port_id` for identification.

```json
[
    {
        "graph_id": 0,
        "device_id": 0,
        "graph_name": "yolox_osd_encode",
        "elements": [
            {
                "element_id": 5000,
                "element_config": "../yolox_bytetrack_osd_encode/config/decode.json",
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
                "element_config": "../yolox_bytetrack_osd_encode/config/yolox_group.json",
                "inner_elements_id": [10001, 10002, 10003]
            },
            {
                "element_id": 5004,
                "element_config": "../yolox_bytetrack_osd_encode/config/bytetrack.json"
            },
            {
                "element_id": 5005,
                "element_config": "../yolox_bytetrack_osd_encode/config/osd.json"
            },
            {
                "element_id": 5006,
                "element_config": "../yolox_bytetrack_osd_encode/config/encode.json",
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
            },
            {
                "src_element_id": 5001,
                "src_port": 0,
                "dst_element_id": 5004,
                "dst_port": 0
            },
            {
                "src_element_id": 5004,
                "src_port": 0,
                "dst_element_id": 5005,
                "dst_port": 0
            },
            {
                "src_element_id": 5005,
                "src_port": 0,
                "dst_element_id": 5006,
                "dst_port": 0
            }
        ]
    }
]
```

[osd.json](./config/osd.json) and similar configuration files detail the configuration specifics for a particular element, setting parameters such as model settings, dynamic library paths, thresholds, and more. These configuration files don't require specifying the `id` or `device_id` fields, as the demo will pass in the `element_id` and `device_id` specified in the engine_group.json.

Among these configurations, `thread_number` specifies the number of working threads within the `element`. Each thread corresponds to a data queue. In scenarios with multiple inputs, it's essential to set the number of data queues reasonably to ensure an even and adequate workload distribution across threads.

```json
{
  "configure": {
    "osd_type": "TRACK",
    "class_names_file": "../yolox_bytetrack_osd_encode/data/coco.names",
    "draw_utils": "OPENCV",
    "draw_interval": false,
    "put_text": false
  },
  "shared_object": "../../build/lib/libosd.so",
  "name": "osd",
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
./main --demo_config_path=../yolox_bytetrack_osd_encode/config/yolox_bytetrack_osd_encode_demo.json
```

The running results are as follows
```bash
total time cost 74520023 us.
frame count is 3077 | fps is 41.2909 fps.
```

If encode selects RTSP mode, you need to start the push streaming server. You can use VLC software to open the push streaming address to view the video algorithm results, see [encode plugin documentation](../../element/multimedia/encode/README.md) for details.

## 7. Performance Testing

Due to the slow drawing speed of Osd plugin, this sample does not provide performance test results for the time being. If you need the inference performance of each model, please go to the corresponding model sample to check.