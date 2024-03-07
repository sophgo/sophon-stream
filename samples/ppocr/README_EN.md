# PP-OCR Demo
English | [简体中文](README.md)
## Catalogs
- [PP-OCR Demo](#PP-OCR-demo)
  - [Directory](#Catalogs)
  - [1. Introduction](#1-Introduction)
  - [2. Feature](#2-Feature)
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

PP-OCRv3 is an ultra-lightweight OCR series model open source by Baidu Feizhu team, including text detection, text classification and text recognition models, and is one of the important components of PaddleOCR tool library. It supports Chinese and English digit combination recognition, vertical text recognition, long text recognition, and its performance and accuracy are significantly improved compared with the previous PP-OCR version. This demo includes the model and algorithm from[PaddleOCR-release-2.6](https://github.com/PaddlePaddle/PaddleOCR/tree/release/2.6) with `ch_PP-OCRv3_xx`.


## 2. Feature
* Supports BM1684,BM1684X(x86 PCIe、SoC),BM1688(SoC)
* Support FP32, FP16 model compilation and inference
* Support image data set testing

## 3. Prepare Models and Data

The scripts directory contains download scripts for relevant models and data [download.sh](./scripts/download.sh)。

```bash
# Install unzip. Skip this step if already installed. If not Ubuntu systems, use yum or other methods as needed.
sudo apt install unzip
chmod -R +x scripts/
./scripts/download.sh
```

After the script execution, data directory will be generated in the current directory, containing two subdirectories: models and datasets

The downloaded models include：

```bash
├── BM1684
│   ├── ch_PP-OCRv3_det_fp32_1b.bmodel
│   ├── ch_PP-OCRv3_rec_fp32_1b_320.bmodel
│   └── ch_PP-OCRv3_rec_fp32_1b_640.bmodel
├── BM1684X
│   ├── ch_PP-OCRv3_det_fp16_1b.bmodel
│   ├── ch_PP-OCRv3_det_fp32_1b.bmodel
│   ├── ch_PP-OCRv3_rec_fp16_1b_320.bmodel
│   ├── ch_PP-OCRv3_rec_fp16_1b_640.bmodel
│   ├── ch_PP-OCRv3_rec_fp32_1b_320.bmodel
│   └── ch_PP-OCRv3_rec_fp32_1b_640.bmodel
└── BM1688
    ├── ch_PP-OCRv3_det_fp16_1b.bmodel
    ├── ch_PP-OCRv3_det_fp32_1b.bmodel
    ├── ch_PP-OCRv3_rec_fp16_1b_320.bmodel
    ├── ch_PP-OCRv3_rec_fp16_1b_640.bmodel
    ├── ch_PP-OCRv3_rec_fp32_1b_320.bmodel
    └── ch_PP-OCRv3_rec_fp32_1b_640.bmodel
```

The downloaded datasets include：
```bash
data
├── class.names
├── datasets
│   ├── ppocr_keys_v1.txt
│   ├── train_full_images_0
│   └── train_full_images_0.json
├── models
└── wqy-microhei.ttc

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

In the ppocr demo, various parameters for each section are located in config directory, structured as follows:"

```bash
./config
├── converger.json                # converger configuration
├── decode.json                   # decoder configuration
├── distributor_frame_class.json  # distributor configuration
├── engine_group.json             # sophon-stream graph configuration
├── ppocr_demo.json               # ppocr demo configuration
├── ppocr_det_group.json          # detection configuration
└── ppocr_rec_group.json          # recognition configuration
```

[ppocr_demo.json](./config/ppocr_demo.json) is the overall configuration file of the routine, managing information such as input stream. Multiple data input can be supported on a single graph. The channels parameter configs the number of input channels, and the channel contains information such as stream url.

If the 'channel_id' attribute is not specified in the configuration file, the demo assigns a default value of 'channel_id' starting from 0 for each data line.

```json
{
  "channels": [
    {
      "channel_id": 2,
      "url": "../ppocr/data/train_full_images_0",
      "source_type": "IMG_DIR",
      "sample_interval": 1,
      "loop_num": 1,
      "fps": -1
    },
    {
      "channel_id": 3,
      "url": "../ppocr/data/train_full_images_0",
      "source_type": "IMG_DIR",
      "sample_interval": 1,
      "loop_num": 1,
      "fps": -1
    },
    {
      "channel_id": 20,
      "url": "../ppocr/data/train_full_images_0",
      "source_type": "IMG_DIR",
      "sample_interval": 1,
      "loop_num": 1,
      "fps": -1
    },
    {
      "channel_id": 30,
      "url": "../ppocr/data/train_full_images_0",
      "source_type": "IMG_DIR",
      "sample_interval": 1,
      "loop_num": 1,
      "fps": -1
    }
  ],
  "engine_config_path": "../ppocr/config/engine_group.json",
  "draw_func_name": "draw_ppocr_results",
  "download_image": true
}
```

[engine_group.json](./config/engine_group.json) contains the configuration information for the graph, which is basically unchanged after it is determined.

Here's an excerpt from the configuration file as an example: Within this file, it's necessary to initialize information for each element and specify the connections between elements. The `element_id` serves as a unique identifier. `element_config` points to the detailed configuration file for that element. `port_id` denotes the input/output port number for the element. And in cases of multiple inputs or outputs, these numbers should not be duplicated. `is_src` denotes whether the current port is the input port for the entire graph, while `is_sink` identifies whether the port is the output for the whole graph. `connection` determines how elements are connected, using `element_id` and `port_id` for identification.

```json
[
    {
        "graph_id": 0,
        "device_id": 1,
        "graph_name": "ppocr",
        "elements": [
            {
                "element_id": 5000,
                "element_config": "../ppocr/config/decode.json",
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
                "element_config": "../ppocr/config/ppocr_det_group.json",
                "inner_elements_id": [10001, 10002, 10003]
            },
            {
                "element_id": 5004,
                "element_config": "../ppocr/config/distributor_frame_class.json"
            },
            {
                "element_id": 6001,
                "element_config": "../ppocr/config/ppocr_rec_group.json",
                "inner_elements_id": [20001, 20002, 20003]
            },
            {
                "element_id": 5005,
                "element_config": "../ppocr/config/converger.json",
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
                "src_element_id": 5004,
                "src_port": 1,
                "dst_element_id": 6001,
                "dst_port": 0
            },
            {
                "src_element_id": 6001,
                "src_port": 0,
                "dst_element_id": 5005,
                "dst_port": 1
            }


        ]
    }
]
```

### 6.2 Execute

For PCIe platforms, you can directly run tests on the PCIe platform. For SoC platforms, you'll need to copy the dynamically linked libraries, executable files, required models, and test data generated from cross-compilation to the SoC platform for testing.

On the SoC platform, maintain a directory structure for dynamic libraries, executable files, configuration files, models, and video data consistent with the original sophon-stream repository's structure.

The parameters for testing and the method of execution remain consistent. Therefore, it'll primarily explain in terms of PCIe mode.

Run the executable file
```bash
./main --demo_config_path=../ppocr/config/ppocr_demo.json
```
## 7. Performance Testing

Currently, the ppocr example supports inference on BM1684X and BM1684 in PCIe and SOC modes, and BM1688 SOC mode.

Modifications in JSON configurations might be necessary when switching between different devices, such as adjusting model paths, input channels, etc. Refer to section 6.1 for JSON configuration methods and section 6.2 for program execution methods.

Due to significant differences in CPU capabilities among PCIe devices, performance data is not meaningful. Therefore, only provide the test results for SOC mode.

The test dataset is train_full_images_0，The compilation was done in Release mode. The results are as follows:

|Device|Model|Number of Channels|Algorithm Thread Count|CPU Utilization(%)|System Memory(M)|TPU Utilization(%)|Device Memory(M)|Average FPS|
|----|----|----|-----|-----|-----|-----|-----|-----|
|SE5-16|fp32|1|1-1-1|227.8|410.1|90|1066|23.14|
|SE7|fp32|1|1-1-1|313.6|438.0|88|1085|38.67|
|SE7|fp16|2|2-2-2|519.6|445.1 |52|1046|66.96|



> **Test Description**：
1. Performance test results exhibit certain fluctuations; it's advisable to conduct multiple tests and calculate the average.
2. Both BM1684 and BM1684X SoC devices utilize an 8-core ARM A53 processor, offering 42320 DMIPS @ 2.3GHz.
3. For the settings of input channels and algorithm thread count in the table, please refer to JSON configuration explanation. CPU utilization and system memory can be checked using the top command. TPU utilization and device memory can be checked using the bm-smi command. FPS can be obtained from the logs printed during program execution.
4. Performance testing is not currently available on the BM1688 device.
5. In this test data, SDK version on SE5 is 0.4.8; SDK version on SE7 is 0.4.9. Different version may bring different performance.