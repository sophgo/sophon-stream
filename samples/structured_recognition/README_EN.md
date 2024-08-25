# structured_recognition Demo

English | [简体中文](README.md)

## Catalogs

- [structured\_recognition Demo](#structured_recognition-demo)
  - [Catalogs](#catalogs)
  - [1. Introduction](#1-introduction)
  - [2. Features](#2-features)
  - [3. Prepare Models and Data](#3-prepare-models-and-data)
  - [4. Prepare Environment](#4-prepare-environment)
    - [4.1 x86/arm PCIe Platform](#41-x86arm-pcie-platform)
    - [4.2 SoC Platform](#42-soc-platform)
    - [4.3 Installing the Visualization Tool on the SoC Platform](#43-installing-the-visualization-tool-on-the-soc-platform)
  - [5. Program Compilation](#5-program-compilation)
    - [5.1 x86/arm PCIe Platform](#51-x86arm-pcie-platform)
    - [5.2 SoC Platform](#52-soc-platform)
  - [6. Program Execution](#6-program-execution)
    - [6.1 Json Configuration Explanation](#61-json-configuration-explanation)
    - [6.2 Execution](#62-execution)
  - [7. Performance Testing](#7-performance-testing)


## 1. Introduction

This example illustrates how to use sophon-stream to distribute a single stream to multiple algorithm modules, quickly building a structured recognition algorithm application (including motor vehicles, non-motor vehicles, people, faces, and license plates).
The plugin connection method for this example is shown in the following diagram.
After decoding a single video stream, it is distributed to three algorithms via the distributor plugin: yolov5 for recognizing people, motor vehicles, and non-motor vehicles; retinaface for face recognition; and license_plate_recognition for license plate recognition.

![structured_recognition](./pics/structured_recognition.png)

The models used in this example are from the [yolov5](../yolov5/README.md), [retinaface](../retinaface/README.md), and [license_plate_recognition](../license_plate_recognition/README.md) examples.


## 2. Features

- Supports BM1684X, BM1684 (x86 PCIe, SoC), BM1688 (SoC)
- Supports multiple algorithms detecting a single video stream
- Supports multithreading

## 3. Prepare Models and Data

In the `scripts` directory, download scripts for relevant models and data are provided in [download.sh](./scripts/download.sh).

```bash
chmod -R +x scripts/
./scripts/download.sh
```

After the script is executed, a `data` directory will be generated in the current directory. In this directory, `coco_lpr.names` contains the class set for the license plate detection yolov5s model, `coco.names` contains the class set for the regular yolov5s model, and `models` stores the model files.


The downloaded data include:

```bash
.
├── data
│   ├── coco_lpr.names
│   ├── coco.names
│   ├── models
│   │   ├── lprnet
│   │   │   ├── BM1684
│   │   │   │   └── lprnet_int8_1b.bmodel
│   │   │   ├── BM1684X
│   │   │   │   └── lprnet_int8_1b.bmodel
│   │   │   └── BM1688
│   │   │       └── lprnet_int8_1b.bmodel
│   │   ├── retinaface
│   │   │   ├── BM1684
│   │   │   │   └── retinaface_mobilenet0.25_int8_1b.bmodel
│   │   │   ├── BM1684X
│   │   │   │   └── retinaface_mobilenet0.25_int8_1b.bmodel
│   │   │   └── BM1688
│   │   │       ├── retinaface_mobilenet0.25_int8_1b_2core.bmodel
│   │   │       └── retinaface_mobilenet0.25_int8_1b.bmodel
│   │   ├── yolov5s
│   │   │   ├── BM1684
│   │   │   │   └── yolov5s_v6.1_3output_int8_1b.bmodel
│   │   │   ├── BM1684X
│   │   │   │   └── yolov5s_v6.1_3output_int8_1b.bmodel
│   │   │   └── BM1688
│   │   │       ├── yolov5s_v6.1_3output_int8_1b_2core.bmodel
│   │   │       └── yolov5s_v6.1_3output_int8_1b.bmodel
│   │   └── yolov5s-licensePLate
│   │       ├── BM1684
│   │       │   └── yolov5s_v6.1_license_3output_int8_1b.bmodel
│   │       ├── BM1684X
│   │       │   └── yolov5s_v6.1_license_3output_int8_1b.bmodel
│   │       └── BM1688
│   │           ├── yolov5s_v6.1_license_3output_int8_1b_2core.bmodel
│   │           └── yolov5s_v6.1_license_3output_int8_1b.bmodel
│   └── videos
│       └── structs.mp4
└── tools
    └── application-web-linux_arm64.tgz
```

The `application-web-linux_arm64.tgz` is a web visualization tool that runs on the SoC platform.

Model and Data Description: For convenience in downloading and testing, this example only uses int8 precision models. If other precision models are needed, they can be downloaded from the [yolov5](../yolov5/README.md), [retinaface](../retinaface/README.md), and [license_plate_recognition](../license_plate_recognition/README.md) examples.

The data includes: category sets `coco_lpr.names`, `coco.names`, and the test video `structs.mp4`.


## 4. Prepare Environment

### 4.1 x86/arm PCIe Platform

If you have installed a PCIe accelerator card (such as the SC series card) on an x86/arm platform, you can directly use it as the development or runtime environment. You need to install libsophon, sophon-opencv, and sophon-ffmpeg. For specific steps, please refer to [the setup guide for x86-pcie platform](../../docs/EnvironmentInstallGuide_EN.md#3-x86-pcie-platform-development-and-runtime-environment-construction) or [setup guide for arm-pcie platform](../../docs/EnvironmentInstallGuide_EN.md#5-arm-pcie-platform-development-and-runtime-environment-construction).

### 4.2 SoC Platform

If you are using the SoC platform (such as SE or SM series edge devices), after flashing(Upgrade the operating system by SD card.), the corresponding libsophon, sophon-opencv, and sophon-ffmpeg runtime library packages are pre-installed under `/opt/sophon/`, which can be directly used as the runtime environment. Typically, you would also need an x86 machine as the development environment for cross-compiling C++ programs.

### 4.3 Installing the Visualization Tool on the SoC Platform
Execute the following installation commands sequentially on the SoC platform:
```bash
tar -xzvf application-web-linux_arm64.tgz 
cd application_web/
sudo ./install.sh
```
After the installation is complete, open a browser and enter `http://{ip}:8089` to open the page, where ip is the IP address of the SoC platform device. Both the username and password are `admin`.

## 5. Program Compilation

### 5.1 x86/arm PCIe Platform
You can directly compile programs on the PCIe platform. For specifics, please refer to [sophon-stream compilation](../../docs/HowToMake_EN.md).

### 5.2 SoC Platform
Typically, programs are cross-compiled on an x86 computer. You need to set up a cross-compilation environment using SOPHON SDK on the x86 computer. Package the necessary include files and library files for the program into the `sophon_sdk_soc` directory. For specifics, please refer to [sophon-stream compilation](../../docs/HowToMake_EN.md). This example mainly dependes on the libsophon, sophon-opencv, and sophon-ffmpeg runtime library packages.

## 6. Program Execution

### 6.1 Json Configuration Explanation

Various parameters in the license_plate_recognition demo are located in the [config](./config/) directory, structured as follows:

```bash
config/
├── converger.json
├── decode.json
├── distributor_frame.json
├── distributor_time_class.json
├── encode.json
├── engine_group.json
├── lprnet_group.json
├── retinaface_group.json
├── structured_recognition_demo.json
├── yolov5_group.json
└── yolov5_lpr_group.json
```

The [engine_group.json](./config/engine_group.json) file is the overall configuration file for the example, managing information such as input streams. It supports multiple data inputs on one diagram and allows a single stream to be allocated to multiple algorithm detections.

In this file, you need to initialize the information for each element and the connections between elements. The `element_id` is unique and serves as an identifier. The `element_config` points to the detailed configuration file address for that element. The `port_id` is the input/output port number of the element. In cases of multiple inputs or outputs, the input/output numbers must not be duplicated. The `is_src` flag indicates whether the current port is an input port for the entire diagram, and the `is_sink` flag indicates whether the current port is an output port for the entire diagram.
The `connection` defines the connections between all elements, determined by `element_id` and `port_id`.

The configuration of [engine_group.json](./config/engine_group.json) is shown in the figure below:  

![engine_group](./pics/engine_group.png)

Where 1000-1009 are the IDs for each `element`. The `connections` in the configuration file represent the arrows connecting the `elements`, with each `element` having a default port of 0. Since the `distributor` needs to distribute a frame of image to multiple `elements`, it has ports 1-3 in addition to the default port 0. In the figure above, the red numbers indicate the ports of the `distributor` and `converger` plugins. The `converger` plugin works with the `distributor` to achieve data aggregation, and its receiving ports need to match the sending ports of the `distributor`.
Finally, the `encode` plugin converts the data into JSON format and streams it via WebSocket.

### 6.2 Execution

For the PCIe platform, you can directly run the test on the PCIe platform. For the SoC platform, you need to copy the dynamically linked libraries, executable files, required models, and test data generated by cross-compilation to the SoC platform for testing.

On the SoC platform, the directory structure of dynamic libraries, executable files, configuration files, models, and video data should be consistent with the original sophon-stream repository.

The parameters and running methods for testing are the same. The following mainly introduces the PCIe mode.

1. Run the executable file
```bash
./main --demo_config_path=../license_plate_recognition/config/license_plate_recognition_demo.json
```  
2. Open the Visualization Tool  
Open the inference result page and enter the WebSocket link in the format `ws://{ip}:{port}`, such as `ws://192.168.0.101:9002`. The port value is determined by the `wss_port` field in the [encode.json](./config/encode.json) file. For example, if the `wss_port` value is 9000 and the `channel_id` in [engine_group.json](./config/engine_group.json) is 2, then the address for this video stream result is 9002.
<div style="text-align: center;">
  <img src="./pics/web1.png" alt="web1" style="width: 65%;">
</div>
Click play to start playing the image stream of the detection results.
<div style="text-align: center;">
  <img src="./pics/res0.png" alt="res0" style="width: 65%;">
</div>
<div style="text-align: center;">
  <img src="./pics/res1.png" alt="res1" style="width: 65%;">
</div>
Click on debug mode to print the content of each frame result in the browser console, allowing you to view the JSON format of the reported data.
<div style="text-align: center;">
  <img src="./pics/res2.png" alt="res2" style="width: 65%;">
</div>  

## 7. Performance Testing

Due to the entire process relying on the input video FPS and the slow WebSocket upload speed, this example does not provide performance test results. For inference performance of each model, please refer to the corresponding model examples.

**Note**: The encode plugin will base64 encode the images, which is relatively slow; if you remove the encode plugin, the source video can run at 30fps.
