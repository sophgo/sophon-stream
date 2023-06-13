# YOLOv5 Demo

## 目录
- [YOLOv5](#yolov5-demo)
    - [目录](#目录)
    - [1. 概述](#1-概述)
    - [2. 编译](#2-编译)
        - [2.1 x86/arm PCIe平台](#21-x86arm-pcie平台)
        - [2.2 SoC平台](#22-soc平台)
    - [3. 数据准备](#3-数据准备)
    - [4. 运行](#4-运行)
        - [4.1 例程代码](#41-例程代码)
        - [4.2 配置文件](#42-配置文件)
    - [5. 性能测试](#5-性能测试)

## 1 概述

- 本例程基于sophon-stream，运行yolov5目标检测；
- 源码地址 [​YOLOv5官方开源仓库](https://github.com/ultralytics/yolov5) v6.1版本；
- 本例程中，yolov5算法的前处理、推理、后处理分别在三个element上进行运算，element内部可以开启多个线程，保证了一定的检测效率；
- 特别地，在BM1684X机器上，本例程中后处理部分可以依赖tpu_kernel，不占用cpu性能；
- 本例程可以在SOPHON BM1684、BM1684X设备， PCIE、SOC模式下运行。

## 2 编译

### 2.1 x86/arm PCIe平台
可以直接在PCIe平台上编译程序，具体请参考[sophon-stream编译](../docs/HowToMake.md)

### 2.2 SoC平台
通常在x86主机上交叉编译程序，您需要在x86主机上使用SOPHON SDK搭建交叉编译环境，将程序所依赖的头文件和库文件打包至sophon_sdk_soc目录中，具体请参考[sophon-stream编译](../docs/HowToMake.md)。本例程主要依赖libsophon、sophon-opencv和sophon-ffmpeg运行库包。

## 3. 数据准备

运行[数据准备脚本](../yolov5/scripts/download.sh)， 下载运行需要的模型、视频文件等

## 4. 运行

### 4.1 例程代码

例程代码位于[yolov5例程](../yolov5/src/yolov5_demo.cc)

### 4.2 配置文件

配置文件位于[yolov5配置文件](../yolov5/config/)

其中， [yolov5_demo.json](../yolov5/config/yolov5_demo.json)是例程的整体配置文件， 管理输入码流等信息。在一张图上可以支持多路数据的输入， num_channels_per_graph参数配置输入的路数， channel中包含码流url等信息。

```
{
  "num_channels_per_graph": 3,
  "channel": {
    "url": "../data/videos/test_car_person_1080P.avi",
    "source_type": 0
  },
  "class_names": "../data/coco.names",
  "download_image": false,
  "engine_config_path": "../config/engine.json"
}
```

[engine.json](../yolov5/config/engine.json) 包含对graph的配置信息， 这部分配置确定之后基本不会发生更改。

在本例程中完整的配置文件中， element中的连接方式如下图所示: 

![elements.jpg](pics/elements.jpg)

这里摘取配置文件的一部分作为示例：在该文件内， 需要初始化每个element的信息和element之间的连接方式。element_id是唯一的， 起到标识身份的作用。element_config指向该element的详细配置文件地址， port_id是该element的输入输出端口编号， 多输入或多输出的情况下， 输入/输出编号也不可以重复。is_src标志当前端口是否是整张图的输入端口， is_sink标识当前端口是否是整张图的输出端口。
connection是所有element之间的连接方式， 通过element_id和port_id确定。

```
{
        "graph_id": 0,
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
                "element_config": "../config/yolov5_pre.json",
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
            }
        ]
    }
```

[yolov5_pre.json](../yolov5/config/yolov5_pre.json)等配置文件是对具体某个element的配置细节， 设置了模型参数、动态库路径、阈值等信息。
其中， thread_number是element内部的工作线程数量， 一个线程会对应一个数据队列， 多路输入情况下， 需要合理设置数据队列数目， 来保证线程工作压力均匀且合理。
```
{
    "configure":{
        "model_path":"../data/models/yolov5s_tpukernel_int8_4b.bmodel",
        "threshold_conf":0.5,
        "threshold_nms":0.5,
        "stage":["pre"],
        "use_tpu_kernel": true
    },
    "shared_object":"../../../element/algorithm/yolov5/build/libyolov5.so",
    "device_id":0,
    "id":0,
    "name":"yolov5",
    "side":"sophgo",
    "thread_number":1
}
```

## 5. 性能测试

目前，yolov5例程支持在BM1684、BM1684X的PCIE、SOC模式下进行推理。

使用[数据准备](#3-数据准备)中下载的视频文件进行测试， 结果如下:

| 测试平台 | num_channels | num_threads_pre | num_threads_infer | num_threads_post | frame_count | fps | tpu | mem |
| ------- | ------------ | --------------- | ----------------- | ---------------- | ----------- | --- | --- | ---|
| BM1684X PCIE | 1 | 1 | 1 | 1 | 711/711 | 168.715 | 80 | 800MB |
| BM1684X PCIE | 3 | 1 | 1 | 1 | 2130-2133/2133 | 131.026 | 60 | 1000MB |
| BM1684X PCIE | 3 | 2 | 2 | 2 | 2130-2133/2133 | 189.617 | 90-100 | 1500-1700MB |
| BM1684X PCIE | 9 | 3 | 3 | 3 | 6382-6384/6399 | 212.57 | 100 | 2000-2200MB |
| BM1684X SOC  | 1 | 1 | 1 | 1 | 711/711 | 119.934 | 50-60 | 700MB |
| BM1684X SOC  | 3 | 1 | 1 | 1 | 2130-2133/2133 | 117.844 | 50 | 700-800MB |
| BM1684X SOC  | 3 | 2 | 2 | 2 | 2127-2133/2133 | 155.123 | 80-100 | 1100-1400MB |
| BM1684X SOC  | 9 | 3 | 3 | 3 | 6298-6312/6399 | 194.906 | 90-100 | 2000-2200MB |