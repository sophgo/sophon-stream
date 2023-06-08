# YOLOX

## 1. 概述

- 本例程基于sophon-stream, 运行yolox目标检测
- yolox由旷视提出, 是基于YOLO系列的改进
- 论文地址 (https://arxiv.org/abs/2107.08430)
- 官方源码地址 (https://github.com/Megvii-BaseDetection/YOLOX)

## 2. 编译

请参考[sophon-stream编译](../../docs/how_to_make.md)

## 3. 数据准备

运行[数据准备脚本](../yolox/scripts/download.sh), 下载运行需要的模型、视频文件等

## 4. 运行

### 4.1 例程代码

例程代码位于[yolox例程](../yolox/src/usecase_yolox.cc)

### 4.2 配置文件

配置文件位于[yolox配置文件](../yolox/config/)

<<<<<<< HEAD
其中, [yolox.json](../yolox/config/yolox.json)是例程的整体配置文件, 管理输入码流等信息。在一张图上可以支持多路数据的输入, num_channels_per_graph参数配置输入的路数, decodeConfigure中包含了每一路数据的url等信息。
=======
其中, [yolox.json](../yolox/config/usecase_yolox.json)是例程的整体配置文件, 管理输入码流等信息。在一张图上可以支持多路数据的输入, num_channels_per_graph参数配置输入的路数, channel中包含码流url等信息。
>>>>>>> 3f3dee4 (123)
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
[engine.json](../yolox/config/engine.json)包含对每一张graph的配置信息。这里摘取一部分作为示例：在一张图内, 需要初始化每个element的信息和element之间的连接方式。element_id是唯一的, 起到标识身份的作用。element_config指向该element的详细配置文件地址, port_id是该element的输入输出端口编号, 多输入或多输出的情况下, 输入/输出编号也不可以重复。is_src标志当前端口是否是整张图的输入端口, is_sink标识当前端口是否是整张图的输出端口。
connection是所有element之间的连接方式, 通过element_id和port_id确定。
```
{
        "graph_id": 0,
        "graph_name": "yolox",
        "elements": [
            {
                "element_id": 5000,
                "element_config": "../config/decoder.json",
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
                "element_config": "../config/pre.json",
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
[yolox_pre.json](../yolox/config/yolox_pre.json)等配置文件是对具体某个element的配置细节, 设置了模型参数、动态库路径、阈值等信息。
其中, thread_number是element内部的工作线程数量, 一个线程会对应一个数据队列, 多路输入情况下, 需要合理设置数据队列数目, 来保证线程工作压力均匀且合理。
```
{
    "configure":{
        "model_path":"../data/models/BM1684X/yolox_s_int8_4b.bmodel",
        "threshold_conf":0.5,
        "threshold_nms":0.5,
        "stage":["pre"]
    },
    "shared_object":"../../../element/algorithm/yolox/build/libyolox.so",
    "device_id":0,
    "id":0,
    "name":"yolox",
    "side":"sophgo",
    "thread_number":1
}
```

## 5. 性能测试

目前, yolox例程支持在BM1684、BM1684X的PCIE、SOC模式下进行推理。

使用[数据准备](#3-数据准备)中下载的视频文件进行测试, 结果如下:

| 测试平台 | num_graph | num_channels_per_graph | num_channels | num_threads_pre | num_threads_infer | num_threads_post | frame_count | fps | tpu | mem |
| ------- | --------- | ---------------------- | ------------ | --------------- | ----------------- | ---------------- | ----------- | --- | --- | ---|
| BM1684X PCIE | 1 | 1 | 1 | 1 | 1 | 1 | 711/711 | 241.761 | 80+ | 430-450MB |
| BM1684X PCIE | 1 | 3 | 3 | 1 | 1 | 1 | 2133/2133 | 163.365 | 40+ | 550-650MB |
| BM1684X PCIE | 1 | 3 | 3 | 2 | 2 | 2 | 2131-2133/2133 | 263.541 | 90-100 | 700-900MB |
| BM1684X PCIE | 1 | 9 | 9 | 3 | 3 | 3 | 6382-6384/6399 | 299.129 | 100 | 1200-1400MB |
| BM1684X PCIE | 1 | 12 | 12 | 4 | 4 | 4 | 8454/8532 | 300.345 | 100 | 1900-2100MB |
| BM1684X SOC | 1 | 1 | 1 | 1 | 1 | 1 | 711/711 | 187.44 | 60-80 | 430-450MB |
| BM1684X SOC | 1 | 3 | 3 | 1 | 1 | 1 | 2131-2133/2133 | 142.041 | 40-50 | 550-650MB |
| BM1684X SOC | 1 | 3 | 3 | 2 | 2 | 2 | 2131-2133/2133 | 226.357 | 80-90 | 700-900MB |
| BM1684X SOC | 1 | 9 | 9 | 3 | 3 | 3 | 6382-6384/6399 | 274.262 | 80-100 | 1200-1400MB |
| BM1684X SOC | 1 | 12 | 12 | 4 | 4 | 4 | 8418/8532 | 281.7 | 90-100 | 1900-2100MB |
| BM1684 PCIE | 1 | 1 | 1 | 1 | 1 | 1 | 711/711 | 101.229 | 60-80 | 500-620MB |
| BM1684 PCIE | 1 | 3 | 3 | 1 | 1 | 1 | 2133/2133 | 98.994 | 60-80 | 600-700MB |
| BM1684 PCIE | 1 | 3 | 3 | 2 | 2 | 2 | 2133/2133 | 115.284 | 80-90 | 700-900MB |
| BM1684 PCIE | 1 | 6 | 6 | 2 | 2 | 2 | 4264-4265/4266 | 123.188 | 86-100 | 1000MB |
| BM1684 PCIE | 1 | 6 | 6 | 3 | 3 | 3 | 4264-4265/4266 | 129.95 | 100 | 1100-1200MB |
| BM1684 SOC | 1 | 1 | 1 | 1 | 1 | 1 | 710-711/711 | 91.4327 | 60-80 | 300-500MB |
| BM1684 SOC | 1 | 3 | 3 | 1 | 1 | 1 | 2133/2133 | 95.4266 | 60-90 | 600-700MB |
| BM1684 SOC | 1 | 3 | 3 | 2 | 2 | 2 | 2133/2133 | 109.007 | 70-90 | 700-900MB |
| BM1684 SOC | 1 | 6 | 6 | 2 | 2 | 2 | 4260-4263/4266 | 119.306 | 80-90 | 1000MB |
| BM1684 SOC | 1 | 6 | 6 | 3 | 3 | 3 | 4261-4263/4266 | 125.322 | 80-90 | 1100-1200MB |