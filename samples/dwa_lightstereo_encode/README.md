# dwa_lightstereo_encode Demo

## 目录
- [dwa\_lightstereo\_encode Demo](#dwa_lightstereo_encode-demo)
  - [目录](#目录)
  - [1. 简介](#1-简介)
  - [2. 特性](#2-特性)
  - [3. 准备数据](#3-准备数据)
  - [4. 环境准备](#4-环境准备)
    - [4. SoC平台](#4-soc平台)
  - [5. 程序编译](#5-程序编译)
    - [5.1 SoC平台](#51-soc平台)
  - [6. 程序运行](#6-程序运行)
    - [6.1 Json配置说明](#61-json配置说明)
    - [6.2 运行](#62-运行)
  - [7. 性能测试](#7-性能测试)

## 1. 简介

本例程用于说明如何使用sophon-stream快速构建深度估计应用。

本例程中，深度估计算法的镜头畸变矫正、深度估计、染色分别在不同的element上进行运算，element内部可以开启多个线程，保证了一定的运行效率。下图是深度估计应用的流程图：
![pipeline](pic/image.png)

其中，输入的`/dev/video0`是指左目摄像头，`/dev/video1`是指右目摄像头，encode element的输出默认是rtsp视频流，对应原视频流的视差。

## 2. 特性

* 支持BM1688(SoC)

## 3. 准备数据

​在`scripts`目录下提供了相关数据的下载脚本 [download.sh](./scripts/download.sh)。

```bash
chmod -R +x scripts/
./scripts/download.sh
```

脚本执行完毕后，会在当前目录下生成`data`目录，目录结构如下：
```bash
.
├── cvi_sdr_bin # isp参数文件
├── gridinfo # 用于dwa模块的参数文件
└── models   # lightstereo的bmodel，来源可参考https://github.com/sophgo/sophon-demo/sample/LightStereo
```

## 4. 环境准备

### 4. SoC平台

如果您使用SoC平台（如SE、SM系列边缘设备），刷机后在`/opt/sophon/`下已经预装了相应的libsophon、sophon-opencv和sophon-ffmpeg运行库包，可直接使用它作为运行环境。通常还需要一台x86主机作为开发环境，用于交叉编译C++程序。
(1) 安装驱动
安装驱动需要进入到超级权限，接着系统驱动目录，安装驱动：
```bash
sudo -s
insmod /mnt/system/ko/v4l2_os04a10_sync.ko
```

（2）isp参数文件配置,需要在当前dwa_lightstereo_encode目录下

```bash
sudo -s
mkdir -p /mnt/cfg/param
cp ./data/cvi_sdr_bin /mnt/cfg/param/
```
备注：如需标定，请参考[摄像头标定](../dwa_dpu_encode/Calibration.md)
## 5. 程序编译

### 5.1 SoC平台
通常在x86主机上交叉编译程序，您需要在x86主机上使用SOPHON SDK搭建交叉编译环境，将程序所依赖的头文件和库文件打包至sophon_sdk_soc目录中，具体请参考[sophon-stream编译](../../docs/HowToMake.md)。本例程主要依赖libsophon、sophon-opencv和sophon-ffmpeg运行库包。

## 6. 程序运行

### 6.1 Json配置说明

dwa_lightstereo_encode demo中各部分参数位于 [config](./config/) 目录，结构如下所示：

```bash
./config/
├── decode.json                 # 解码配置
├── encode.json                 # 编码配置
├── engine.json                 # sophon-stream graph配置
├── camera_dwa_lightstereo_encode_demo.json # demo配置文件
├── dwa_L.json                  # 左侧输入对应的畸变矫正配置文件
├── dwa_R.json                  # 右侧输入对应的畸变矫正配置文件
├── lightstereo_pre.json        # lightstereo深度估计配置文件-前处理
├── lightstereo_infer.json      # lightstereo深度估计配置文件-推理
└── lightstereo_post.json       # lightstereo深度估计配置文件-后处理
```

其中，[camera_dwa_lightstereo_encode_demo.json](./config/camera_dwa_lightstereo_encode_demo.json)是例程的整体配置文件，管理输入码流等信息。在一张图上可以支持2路数据的输入，channels参数配置输入的路数，sample_interval设置跳帧数，loop_num设置循环播放次数，channel中包含码流url等信息。此例程不支持download选项。

配置文件中不指定`channel_id`属性的情况，会在demo中对每一路数据的`channel_id`从0开始默认赋值。


### 6.2 运行

对于SoC平台，需将交叉编译生成的动态链接库、可执行文件、所需的模型和测试数据拷贝到SoC平台中测试。

SoC平台上，动态库、可执行文件、配置文件、模型、视频数据的目录结构关系应与原始sophon-stream仓库中的关系保持一致。


1. 参考[rtsp使用说明](../../element/multimedia/encode/README.md#3-rtsp使用说明)，在SoC平台上运行rtsp服务器。
  
2. 运行可执行文件,sensor的出图需要root权限
```bash
sudo -s
./main --demo_config_path=../dwa_lightstereo_encode/config/camera_dwa_lightstereo_encode_demo.json
```

3. 在可以ping通SoC平台的主机上，使用VLC拉流。

## 7. 性能测试

目前，深度估计算法只支持在BM1688 SOC模式下进行推理。按sensor输入按照默认设置可以达到25fps。可通过运行中打印的log确认fps是否正常。