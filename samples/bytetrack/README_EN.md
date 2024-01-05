# ByteTrack Demo

English | [简体中文](README.md)

## Catalogs
- [ByteTrack Demo](#bytetrack-demo)
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

This example demonstrates how to use sophon-stream to quickly build a video object track application.

The connection method for this example plugin is shown in the following diagram.

![elements.jpg](pics/tracker.png)

Bytetrack is a simple, fast and powerful multi-target tracker, and does not rely on feature extraction models.

**paper** (https://arxiv.org/abs/2110.06864)

**source code** (https://github.com/ifzhang/ByteTrack)

## 2. Features

* Supports BM1684X(x86 PCIe、SoC) and BM1684(x86 PCIe、SoC、arm PCIe)
* Support the decoupling of detection module and tracking module, can be adapted to a variety of detectors, this routine is mainly used as a detector YOLOX
* Supports multiple video streams.
* Supports multi-threading.

## 3. Prepare Models and Data

The `scripts` directory contains download scripts for relevant models and data. [download.sh](./scripts/download.sh)。

```bash
# Install unzip. Skip this step if already installed. If not Ubuntu systems, use yum or other methods as needed.
sudo apt install unzip
chmod -R +x scripts/
./scripts/download.sh
```

After the script execution, `data` directory will be generated in the current directory, containing two subdirectories: `models` and `videos`

The downloaded models include:

```bash
./data/models
├── BM1684
│   ├── yolox_s_fp32_1b.bmodel    # FP32 BModel for BM1684，batch_size=1
│   ├── yolox_s_fp32_4b.bmodel    # FP32 BModel for BM1684，batch_size=4
│   ├── yolox_s_int8_1b.bmodel    # INT8 BModel for BM1684，batch_size=1
│   └── yolox_s_int8_4b.bmodel    # INT8 BModel for BM1684，batch_size=4
├── BM1684X
│   ├── yolox_s_fp32_1b.bmodel    # FP32 BModel for BM1684X，batch_size=1
│   ├── yolox_s_fp32_4b.bmodel    # FP32 BModel for BM1684X，batch_size=4
│   ├── yolox_s_int8_1b.bmodel    # INT8 BModel for BM1684X，batch_size=1
└── └── yolox_s_int8_4b.bmodel    # INT8 BModel for BM1684X，batch_size=4
```

The downloaded data include:
```bash
./data/videos
└──  test_car_person_1080P.avi                 # test video
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

In the ByteTrack demo, various parameters for each section are located in [config](./config/) directory, structured as follows:

```bash
./config
   ├── bytetrack_demo.json       # bytetrack demo configuration
   ├── bytetrack.json            # bytetrack tracker configuration
   ├── decoder.json              # decoding configuration
   ├── engine.json               # sophon-stream graph configuration
   ├── infer.json                # detector inference configuration
   ├── post.json                 # detector post-process configuration
   └── pre.json                  # detector pre-process configuration
```

Indeed, [bytetrack_demo.json](./config/bytetrack_demo.json) is the overall configuration file for the example, managing input streams and other information. Multiple data inputs can be supported on a single graph, where the `channels` parameter configures the number of input channels, `sample_interval` sets the frame skipping rate, and `loop_num` sets the number of looped plays. The `channel` section contains video stream information such as the URL. The `download_image` parameter controls whether to save the inference results. If set to `false`, results will not be saved. If set to `true`, they will be saved in the `/build/results` directory.

In the configuration file, when the `channel_id` attribute is not specified, the demo will assign default `channel_id` values starting from 0 for each data channel.

```json
{
  "channels": [
    {
      "channel_id": 2,
      "url": "../bytetrack/data/videos/test_car_person_1080P.avi",
      "source_type": "VIDEO",
      "sample_interval": 1,
      "loop_num": 1,
      "fps": -1
    },
    {
      "channel_id": 3,
      "url": "../bytetrack/data/videos/test_car_person_1080P.avi",
      "source_type": "VIDEO",
      "sample_interval": 1,
      "loop_num": 1,
      "fps": -1
    },
    {
      "channel_id": 20,
      "url": "../bytetrack/data/videos/test_car_person_1080P.avi",
      "source_type": "VIDEO",
      "sample_interval": 1,
      "loop_num": 1,
      "fps": -1
    },
    {
      "channel_id": 30,
      "url": "../bytetrack/data/videos/test_car_person_1080P.avi",
      "source_type": "VIDEO",
      "sample_interval": 1,
      "loop_num": 1,
      "fps": -1
    }
  ],
  "download_image": true,
  "draw_func_name": "draw_bytetrack_results",
  "engine_config_path": "../bytetrack/config/engine.json"
}
```

[engine.json](./config/engine.json) contains configuration information for graphs, which, once set, typically remain unchanged.

Here's an excerpt from the configuration file as an example: Within this file, it's necessary to initialize information for each element and specify the connections between elements. The `element_id` serves as a unique identifier. `element_config` points to the detailed configuration file for that element. `port_id` denotes the input/output port number for the element. And in cases of multiple inputs or outputs, these numbers should not be duplicated. `is_src` denotes whether the current port is the input port for the entire graph, while `is_sink` identifies whether the port is the output for the whole graph. `connection` determines how elements are connected, using `element_id` and `port_id` for identification.

```json
[
    {
        "graph_id": 0,
        "device_id": 0,
        "graph_name": "bytetrack",
        "elements": [
            {
                "element_id": 5000,
                "element_config": "../bytetrack/config/decode.json",
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
                "element_config": "../bytetrack/config/yolox_pre.json",
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
                            "is_sink": false,
                            "is_src": false
                        }
                    ]
                }
            },
            {
                "element_id": 5002,
                "element_config": "../bytetrack/config/yolox_infer.json",
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
                            "is_sink": false,
                            "is_src": false
                        }
                    ]
                }
            },
            {
                "element_id": 5003,
                "element_config": "../bytetrack/config/yolox_post.json",
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
                            "is_sink": false,
                            "is_src": false
                        }
                    ]
                }
            },
            {
                "element_id": 5004,
                "element_config": "../bytetrack/config/bytetrack.json",
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
                "dst_element_id": 5002,
                "dst_port": 0
            },
            {
                "src_element_id": 5002,
                "src_port": 0,
                "dst_element_id": 5003,
                "dst_port": 0
            },
            {
                "src_element_id": 5003,
                "src_port": 0,
                "dst_element_id": 5004,
                "dst_port": 0
            }
        ]
    }
]
```

[bytetrack.json](./config/bytetrack.json) and similar configuration files detail the configuration specifics for a particular element, setting parameters such as model settings, dynamic library paths, thresholds, and more. These configuration files don't require specifying the `id` or `device_id` fields, as the demo will pass in the `element_id` and `device_id` specified in the engine_group.json.

Among these configurations, `thread_number` specifies the number of working threads within the `element`. Each thread corresponds to a data queue. In scenarios with multiple inputs, it's essential to set the number of data queues reasonably to ensure an even and adequate workload distribution across threads.

```json
{
    "configure": {
        "track_thresh": 0.5,
        "high_thresh": 0.6,
        "match_thresh": 0.7,
        "frame_rate": 30,
        "track_buffer": 30
    },
    "shared_object": "../../build/lib/libbytetrack.so",
    "name": "bytetrack",
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
./main --demo_config_path=../bytetrack/config/bytetrack_demo.json
```

The running results of two video streams are as follows
```bash
total time cost 5246889 us.
frame count is 1422 | fps is 271.018 fps.
```

## 7. Performance Testing

The tested video is `elevator-1080p-25fps-4000kbps.h264`. The compilation was done in Release mode. The results are as follows:

|Device|Number of Channels|Algorithm Thread Count|CPU Utilization(%)|System Memory(M)|Peak System Memory(M)|TPU Utilization(%)|Device Memory(M)|Peak Device Memory(M)|Average FPS|Peak FPS|
|----|----|-----|-----|-----|-----|-----|-----|-----|-----|-----|
|SE7|8|4-4-4|505.98|208.26|216.78|99.94|1432.99|1623.00|318.06|332.26|
|SE5-16|4|4-4-4|238.21|118.36|119.81|94.40|1258.89|1397.00|130.36|143.47|
|SE5-8|3|3-3-3|160.97|96.19|97.71|92.69|993.08|1128.00|82.01|91.24|

> **Test Description**:
1. Performance test results exhibit certain fluctuations; it's advisable to conduct multiple tests and calculate the average.
2. Both BM1684 and BM1684X SoC devices utilize an 8-core ARM A53 processor, offering 42320 DMIPS @ 2.3GHz.