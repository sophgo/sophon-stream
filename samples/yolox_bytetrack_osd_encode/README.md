# 目标跟踪算法结果推流Demo

## 目录
- [目标跟踪算法结果推流Demo](#目标跟踪算法结果推流Demo)
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

## 1. 概述

- 本例程用于说明如何使用sophon-stream快速构建视频目标跟踪应用，并将算法结果推流输出；
- 检测模型使用yolox；
- 跟踪模型使用bytetrack；
- 本例程可以在SOPHON BM1684、BM1684X设备，PCIE、SOC模式下运行。

## 2. 编译

### 2.1 x86/arm PCIe平台
可以直接在PCIe平台上编译程序，具体请参考[sophon-stream编译](../docs/HowToMake.md)

### 2.2 SoC平台
通常在x86主机上交叉编译程序，您需要在x86主机上使用SOPHON SDK搭建交叉编译环境，将程序所依赖的头文件和库文件打包至sophon_sdk_soc目录中，具体请参考[sophon-stream编译](../docs/HowToMake.md)。本例程主要依赖libsophon、sophon-opencv和sophon-ffmpeg运行库包。

## 3. 数据准备

运行[数据准备脚本](../yolox_bytetrack_osd_encode/scripts/download.sh)，下载运行需要的模型、视频文件等

## 4. 运行

### 4.1 例程代码

例程代码位于[yolox例程](../yolox_bytetrack_osd_encode/src/yolox_bytetrack_osd_encode_demo.cc)

### 4.2 配置文件

配置文件位于[配置文件](../yolox_bytetrack_osd_encode/config)

其中，[yolox_bytetrack_osd_encode_demo.json](../yolox_bytetrack_osd_encode/config/yolox_bytetrack_osd_encode_demo.json)是例程的整体配置文件，管理输入码流等信息。在一张图上可以支持多路数据的输入，channels中包含每一路码流url等信息。

```
{
  "channels": [
    {
      "channel_id": 2,
      "url": "../data/videos/mot17_01_frcnn.mp4",
      "source_type": 0
    },
    {
      "channel_id": 3,
      "url": "../data/videos/mot17_03_frcnn.mp4",
      "source_type": 0
    },
    {
      "channel_id": 20,
      "url": "../data/videos/mot17_06_frcnn.mp4",
      "source_type": 0
    },
    {
      "channel_id": 30,
      "url": "../data/videos/mot17_08_frcnn.mp4",
      "source_type": 0
    }
  ],
  "engine_config_path": "../config/engine.json"
}
```

[engine.json](../yolox_bytetrack_osd_encode/config/engine.json) 包含对graph的配置信息，这部分配置确定之后基本不会发生更改。

在本例程中完整的配置文件中，element中的连接方式如下图所示: 

![elements.jpg](pics/elements.jpg)

这里摘取配置文件的一部分作为示例：在该文件内，需要初始化每个element的信息和element之间的连接方式。element_id是唯一的，起到标识身份的作用。element_config指向该element的详细配置文件地址，port_id是该element的输入输出端口编号，多输入或多输出的情况下，输入/输出编号也不可以重复。is_src标志当前端口是否是整张图的输入端口，is_sink标识当前端口是否是整张图的输出端口。
connection是所有element之间的连接方式，通过element_id和port_id确定。

```
[
    {
        "graph_id": 0,
        "graph_name": "yolox",
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
                "element_config": "../config/yolox_pre.json",
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
                "element_config": "../config/yolox_infer.json",
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
                "element_config": "../config/yolox_post.json",
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
                "element_config": "../config/bytetrack.json",
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
                "element_id": 5005,
                "element_config": "../config/osd.json",
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
                "element_id": 5006,
                "element_config": "../config/encode.json",
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

[osd.json](../yolox_bytetrack_osd_encode/config/osd.json)等配置文件是对具体某个element的配置细节，设置了模型参数、动态库路径、阈值等信息。
其中，thread_number是element内部的工作线程数量，一个线程会对应一个数据队列，多路输入情况下，需要合理设置数据队列数目，来保证线程工作压力均匀且合理。
```
{
  "configure": {
    "osd_type": "track",
    "class_names": "../data/coco.names"
  },
  "shared_object": "../../../build/lib/libosd.so",
  "device_id": 0,
  "id": 0,
  "name": "osd",
  "side": "sophgo",
  "thread_number": 1
}
```

## 5. 性能测试
由于Osd插件画图速度慢，本例程暂不提供性能测试结果，如需各模型推理性能，请到对应模型例程查看。