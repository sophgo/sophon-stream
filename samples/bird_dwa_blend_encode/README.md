# dwa_blend_encode Demo

## 目录
- [dwa\_blend\_encode Demo](#dwa_blend_encode-demo)
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
  - [8. web ui使用](#8-web-ui使用)
    - [8.1 安装nodejs](#81-安装nodejs)
    - [8.2 web ui编译](#82-web-ui编译)
    - [8.3 运行web ui](#83-运行web-ui)

## 1. 简介

本例程用于说明如何使用sophon-stream快速构建鸟瞰应用。

本例程中，鸟瞰拼接算法的鱼眼展开分别在四个element上进行运算，鸟瞰拼接在三个element上运算。

## 2. 特性

* 支持BM1688(SoC)
* 支持多路视频流
* 支持多线程

## 3. 准备数据

​在`scripts`目录下提供了相关数据的下载脚本 [download.sh](./scripts/download.sh)。

```bash
# 安装unzip，若已安装请跳过，非ubuntu系统视情况使用yum或其他方式安装
sudo apt install unzip
chmod -R +x scripts/
./scripts/download.sh
```

脚本执行完毕后，会在当前目录下生成`data`目录，其中包含`models`和`videos`两个子目录。
```bash
.
├── gridinfo
│   ├── 0grid_info_bev_60_60_2920_60_60_dst_1920x1920_src_1920x1080.dat
│   ├── 1grid_info_bev_60_60_2381_60_60_dst_1920x1920_src_1920x1080.dat
│   ├── 2grid_info_bev_60_60_2772_60_60_dst_1920x1920_src_1920x1080.dat
│   ├── 3grid_info_bev_60_60_2904_60_60_dst_1920x1920_src_1920x1080.dat
│   ├── Dgrid_info_64_16_1024_64_16_dst_2048x512_src_992x3072.dat
│   ├── Lgrid_info_16_96_1536_16_96_dst_512x3072_src_2240x2240.dat
│   ├── Lgrid_info_68_68_4624_70_70_dst_2240x2240_src_2240x2240.dat
│   ├── Rgrid_info_16_96_1536_16_96_dst_512x3072_src_2240x2240.dat
│   ├── Rgrid_info_68_67_4556_70_70_dst_2240x2240_src_2240x2240.dat
│   └── Ugrid_info_64_16_1024_64_16_dst_2048x512_src_992x3072.dat
├── images
│   ├── 0
│   │   └── sensor00.jpg
│   ├── 1
│   │   └── sensor1.jpg
│   ├── 2
│   │   └── sensor2.jpg
│   ├── 3
│   │   └── sensor3.jpg
│   ├── left
│   │   └── dc_src_2240x2240_L.png
│   ├── right
│   │   └── dc_src_2240x2240_R.png
│   ├── v2l
│   │   └── dc_srcl.jpg
│   └── v2r
│       └── dc_src.jpg
├── param
│   ├── 04E10_fisheye_dual.bin
│   ├── 04a10_zhiyuan_zhang.bin
│   ├── 04a10_zhiyuan_zhang01-12.bin
│   ├── A2_04A10_linear_rgb.bin
│   ├── cvi_sdr_bin
│   ├── cvi_sdr_bin16.bin
│   ├── duan-ce-04a10.bin
│   ├── os04a10-sdr-test.bin
│   ├── zbcv.bin
│   └── zhiyuan.zhang-04a10-2.bin
├── test_img
│   ├── 1920x1080.jpg
│   ├── dc_src_2240x2240_L.png
│   ├── dc_src_2240x2240_L.yuv
│   ├── dc_src_2240x2240_R.png
│   ├── dc_src_2240x2240_R.yuv
│   ├── sensor0.ppm
│   ├── sensor1.ppm
│   ├── sensor2.ppm
│   └── sensor3.ppm
└── wgt
    ├── alpha_weight_0.bin
    ├── alpha_weight_1.bin
    ├── alpha_weight_2.bin
    ├── beta_weight_0.bin
    ├── beta_weight_1.bin
    ├── beta_weight_2.bin
    ├── c01_alpha_444p_m2__0_2240x32.bin
    ├── c01_alpha_444p_m2__0_3072x32.bin
    ├── c01_beta_444p_m2__0_2240x32.bin
    └── c01_beta_444p_m2__0_3072x32.bin
```

## 4. 环境准备

### 4. SoC平台

如果您使用SoC平台（如SE、SM系列边缘设备），刷机后在`/opt/sophon/`下已经预装了相应的libsophon、sophon-opencv和sophon-ffmpeg运行库包，可直接使用它作为运行环境。通常还需要一台x86主机作为开发环境，用于交叉编译C++程序。
(1) 安装驱动
安装驱动需要进入到超级权限，接着系统驱动目录，安装驱动：

```bash

sudo -s
insmod /mnt/system/ko/v4l2_pr2100.ko force_bus=1,1,1,1,-1,-1  force_i2caddr=0x5F,0x5F,0x5C,0x5C,0x5F,0x5F force_slave=0,0,1,1,0,0
```

## 5. 程序编译

### 5.1 SoC平台
通常在x86主机上交叉编译程序，您需要在x86主机上使用SOPHON SDK搭建交叉编译环境，将程序所依赖的头文件和库文件打包至sophon_sdk_soc目录中，具体请参考[sophon-stream编译](../../docs/HowToMake.md)。本例程主要依赖libsophon、sophon-opencv和sophon-ffmpeg运行库包。

## 6. 程序运行

### 6.1 Json配置说明

dwa_blend_encode demo中各部分参数位于 [config](./config/) 目录，结构如下所示：

```bash
./config/
├── decode.json                 # 解码配置
├── engine.json                 # sophon-stream graph配置，需要分别配置dwa、blend、resize、encode等文件
├── camera_dwa_blend_encode_demo.json          # demo按sensor输入的配置文件
├── dwa_blend_encode_demo.json            # demo按图片输入的配置文件
├── dwa_0.json                  # 左侧输入的鱼眼展开配置文件
├── dwa_1.json                  # 右侧输入的鱼眼展开配置文件
├── dwa_2.json                  # 上侧输入的鱼眼展开配置文件
├── dwa_3.json                  # 下侧输入的鱼眼展开配置文件
├── blend1.json                  # 拼接1配置文件
├── blend2.json                  # 拼接2配置文件
├── blend3.json                  # 拼接3配置文件
├── encode.json                  # 编码配置
└── resize.json                 # 尺寸缩放配置文件
```

其中，[camera_dwa_blend_encode_demo.json](./config/camera_dwa_blend_encode_demo.json)是例程的整体配置文件，管理输入码流等信息。在一张图上可以支持多路数据的输入，channels参数配置输入的路数，sample_interval设置跳帧数，loop_num设置循环播放次数，channel中包含码流url等信息。download_image控制是否保存推理结果，若为false则不保存，若为true，则会保存在/build/results目录下。

配置文件中不指定`channel_id`属性的情况，会在demo中对每一路数据的`channel_id`从0开始默认赋值。


### 6.2 运行

对于PCIe平台，可以直接在PCIe平台上运行测试；对于SoC平台，需将交叉编译生成的动态链接库、可执行文件、所需的模型和测试数据拷贝到SoC平台中测试。

SoC平台上，动态库、可执行文件、配置文件、模型、视频数据的目录结构关系应与原始sophon-stream仓库中的关系保持一致。


1. 运行可执行文件
```bash
./main --demo_config_path=../bird_dwa_blend_encode/config/camera_dwa_blend_encode_demo.json
```

## 7. 性能测试

目前，鱼眼拼接算法只支持在BM1688 SOC模式下进行推理。按照默认设置可以达到25fps。

## 8. web ui使用
### 8.1 安装nodejs
访问https://nodejs.org/en/download/，根据说明完成nodejs的安装，推荐使用node-v20.11.1版本。

### 8.2 web ui编译
进入sophon-stream/sample/dwa_dpu_encode/web_ui目录，执行以下命令：
```bash
npm install
npm run build
```
编译完成后会在该目录下生产build文件夹。
### 8.3 运行web ui
进入sophon-stream/sample/dwa_dpu_encode/web_ui/build目录，执行以下命令：
```bash
python3 -m http.server 3000
```
其中，3000是web ui的端口号，可以根据需要修改。
在浏览器中访问http://localhost:3000/，即可打开web ui界面。（localhost更改为运行环境的ip）


