# Face Detection-Distribution-Recognition Demo

English | [简体中文](README.md)

## Catalogs
- [Face Detection-Distribution-Recognition Demo](#face-detection-distribution-recognition-demo)
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
  - [8. Database Generation](#8-database-generation)

## 1. Introduction

This sample is used to illustrate how to use sophon-stream to quickly build complex applications that contain multiple algorithms and send to different branches by classes.

## 2. Features
* Use retinaface for detection;
* Use resnet for face feature extraction;
* Use faiss for face recognition;
* Support BM1684X (x86 PCIe, SoC);
* Supports multiple video streams;
* Support multi-threading.

## 3. Prepare Models and Data

The `scripts` directory contains download scripts for relevant models and data. [download.sh](./scripts/download.sh).

```bash
# Install unzip. Skip this step if already installed. If not Ubuntu systems, use yum or other methods as needed.
sudo apt install unzip
chmod -R +x scripts/
./scripts/download.sh
```

After the script execution, `data` directory will be generated in the current directory, containing three subdirectories: `models`, `face_data` and `image`.

The downloaded models include:

```bash
./models
└── BM1684X
    ├── resnet_arcface_fp32_1b.bmodel           # resnet for face feature extraction
    └── retinaface_mobilenet0.25_fp32_1b.bmodel # retinaface for face detection
```

The downloaded data include:

```bash
├── class.names           # distributor labels 
├── face_data 
│   ├── faiss_db_data.txt # database of features
│   └── faiss_index_label.name # database of label
├── images
│   ├── face_data_test  # dataset for test
│   └── face_data_train # dataset for generating the database
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

The configuration file is located in [./config](./config) directory with the following structure:

```bash
./config
├── converger.json                                                    # converger element configuration file
├── decode.json                                                       # decoding configuration
├── distributor_class.json                                            # distribute every frame in class
├── distributor_frame_class.json                                      # distribute frame with frame interval in class
├── distributor_frame.json                                            # distribute full frame with frame interval
├── distributor_time_class.json                                       # distribute frame with time interval (default)
├── distributor_time.json                                             # distribute full frame with time interval
├── engine.json                                                       # graph configuration
├── engine_group.json                                                 # 简化的graph配置 simplified graph configuration
├── faiss.json                                                        # faiss configuration
├── resnet_face.json                                                  # resnet for face feature extraction
├── retinaface_distributor_resnet_faiss.json                          # demo configuration
├── retinaface_group.json                                             # A simplified retinaface configuration file that combines pre-processing, inference, and post-processing into one configuration file
├── retinaface_infer.json                                             # retinaface inference configuration
├── retinaface_post.json                                              # retinaface post-process configuration
└── retinaface_pre.json                                               # retinaface pre-process configuration
```

Indeed, [retinaface_distributor_resnet_faiss.json](./config/retinaface_distributor_resnet_faiss.json) is the overall configuration file for the example, managing input streams and other information. Multiple data inputs can be supported on a single graph, where the `channels` parameter configures the number of input channels, `sample_interval` sets the frame skipping rate, and `loop_num` sets the number of looped plays. The `channel` section contains video stream information such as the URL. The `download_image` parameter controls whether to save the inference results. If set to `false`, results will not be saved. If set to `true`, they will be saved in the `/build/results` directory.

In the configuration file, when the `channel_id` attribute is not specified, the demo will assign default `channel_id` values starting from 0 for each data channel.

```json
{
  "channels": [
    {
      "channel_id": 0,
      "url": "../retinaface_distributor_resnet_faiss_converger/data/images/face_data_test",
      "source_type": "IMG_DIR",
      "loop_num": 1,
      "sample_interval": 1,
      "fps": -1
    },
    {
      "channel_id": 1,
      "url": "../retinaface_distributor_resnet_faiss_converger/data/images/face_data_test",
      "source_type": "IMG_DIR",
      "loop_num": 1,
      "sample_interval": 1,
      "fps": -1
    },
    {
      "channel_id": 2,
      "url": "../retinaface_distributor_resnet_faiss_converger/data/images/face_data_test",
      "source_type": "IMG_DIR",
      "loop_num": 1,
      "sample_interval": 1,
      "fps": -1
    },
    {
      "channel_id": 3,
      "url": "../retinaface_distributor_resnet_faiss_converger/data/images/face_data_test",
      "source_type": "IMG_DIR",
      "loop_num": 1,
      "sample_interval": 1,
      "fps": -1
    }
  ],
  "download_image": true,
  "draw_func_name": "draw_retinaface_distributor_resnet_faiss_converger_results",
  "engine_config_path": "../retinaface_distributor_resnet_faiss_converger/config/engine_group.json"
}
```

[engine_group.json](./config/engine_group.json) contains configuration information for graphs, which, once set, typically remain unchanged.

Here's an excerpt from the configuration file as an example: Within this file, it's necessary to initialize information for each element and specify the connections between elements. The `element_id` serves as a unique identifier. `element_config` points to the detailed configuration file for that element. `port_id` denotes the input/output port number for the element. And in cases of multiple inputs or outputs, these numbers should not be duplicated. `is_src` denotes whether the current port is the input port for the entire graph, while `is_sink` identifies whether the port is the output for the whole graph. `connection` determines how elements are connected, using `element_id` and `port_id` for identification.

[retinaface_group.json](./config/retinaface_group.json) and similar configuration files detail the configuration specifics for a particular element, setting parameters such as model settings, dynamic library paths, thresholds, and more. These configuration files don't require specifying the `id` or `device_id` fields, as the demo will pass in the `element_id` and `device_id` specified in the engine_group.json.

Among these configurations, `thread_number` specifies the number of working threads within the `element`. Each thread corresponds to a data queue. In scenarios with multiple inputs, it's essential to set the number of data queues reasonably to ensure an even and adequate workload distribution across threads.


### 6.2 Execute

For PCIe platforms, you can directly run tests on the PCIe platform. For SoC platforms, you'll need to copy the dynamically linked libraries, executable files, required models, and test data generated from cross-compilation to the SoC platform for testing.

On the SoC platform, maintain a directory structure for dynamic libraries, executable files, configuration files, models, and video data consistent with the original sophon-stream repository's structure.

The parameters for testing and the method of execution remain consistent. Therefore, it'll primarily explain in terms of PCIe mode.

Run the executable file
```bash
./main --demo_config_path=../retinaface_distributor_resnet_faiss_converger/config/retinaface_distributor_resnet_faiss_converger.json
```

The results are stored in the `./build/results` directory. This routine is configured by default to be sent to the resnet branch per second by category, and will save one frame per second of the drawn face label image in the results directory.

## 7. Performance Testing

Since the whole process depends on the input video fps and the drawing speed is slow, this routine does not provide performance test results for the time being, if you need the inference performance of each model, please go to the corresponding model routine to check.

## 8. Database Generation

In this sample, the face index is returned by comparing the vectors output from the resnet model with the vectors already in the faiss index library, and then the corresponding face labels are obtained through the index. Here, we also provide the method to generate the face database as follows:

```bash
python3 scripts/resnet_opencv_faiss_write.py --input data/images/face_data_train --bmodel data/models/BM1684X/resnet_arcface_fp32_1b.bmodel --db_data faiss_db_data.txt --index_label faiss_index_label.name --dev_id 0 
```

the parameters are as follow:

```bash
usage:resnet_opencv_faiss_write.py [--input IMG_PATH] [--bmodel BMODEL] [--db_data DB_DATA] [--index_label INDEX_LABEL] [--dev_id DEV_ID]
--input: image folder path;
--bmodel: bmodel path;
--db_data: face database;
--index_label: label file；
--dev_id: tpu id。
```