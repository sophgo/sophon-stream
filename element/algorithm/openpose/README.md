# sophon-stream openpose element

[English](README_EN.md) | 简体中文

sophon-stream openpose element是sophon-stream框架中的一个插件，是一个简单、快速、强大的姿态识别模型。本项目已提供此插件例程，详情请参见 [openpose Demo](../../../samples/openpose/README.md)

## 1. 特性
* 支持多路视频流
* 支持多线程处理

## 2. 配置参数
sophon-stream openpose插件具有一些可配置的参数，可以根据需求进行设置。以下是一些常用的参数：

```json
{
    "configure": {
        "model_path": "../data/models/BM1684X/pose_coco_int8_1b.bmodel",
        "threshold_nms": 0.05,
        "stage": [
            "pre"
        ]
    },
    "shared_object": "../../../build/lib/libopenpose.so",
    "name": "openpose",
    "side": "sophgo",
    "thread_number": 4
}
```

|      参数名    |    类型    | 默认值 | 说明 |
|:-------------:| :-------: | :------------------:| :------------------------:|
|  model_path  |   字符串   | "../data/models/BM1684X/pose_coco_int8_1b.bmodel" | openpose模型路径 |
|  threshold_nms  |   浮点数   | 0.05 | 姿态识别NMS IOU阈值 |
|  stage    |   列表   | ["pre"]  | 标志前处理、推理、后处理三个阶段 |
|  shared_object |   字符串   |  "../../../build/lib/libopenpose.so"  | libopenpose 动态库路径 |
|     name    |    字符串     | "openpose" | element 名称 |
|     side    |    字符串     | "sophgo"| 设备类型 |
| thread_number |    整数     | 1 | 启动线程数 |

> **注意**：
1. stage参数，需要设置为"pre"，"infer"，"post" 其中之一或相邻项的组合，并且按前处理-推理-后处理的顺序连接element。将三个阶段分配在三个element上的目的是充分利用tpu和cpu资源，提高检测效率。
