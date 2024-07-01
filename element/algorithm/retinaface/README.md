# sophon-stream retinaface element

[English](README_EN.md) | 简体中文

sophon-stream retinaface element是sophon-stream框架中的一个插件，利用额外监督(extra-supervised)和自监督(self-supervised)结合的多任务学习(multi-task learning)，对不同尺寸的人脸进行像素级定位。本项目已提供此插件例程，详情请参见 [retinaface Demo](../../../samples/retinaface/README.md)

## 1. 特性
* 支持多路视频流
* 支持多线程处理

## 2. 配置参数
sophon-stream retinaface插件具有一些可配置的参数，可以根据需求进行设置。以下是一些常用的参数：

```json
{
    "configure": {
        "model_path": "../data/models/BM1684X/retinaface_mobilenet0.25_fp32_1b.bmodel",
        "max_face_count":50,
        "score_threshold":0.1,
        "threshold_nms": 0.4,
        "bgr2rgb": false,
        "mean": [
            104,
            117,
            123
        ],
        "std": [
            1,
            1,
            1
        ],
        "stage": [
            "pre"
        ]
    },
    "shared_object": "../../../build/lib/libretinaface.so",
    "name": "retinaface",
    "side": "sophgo",
    "thread_number": 1
}
```

|      参数名    |    类型    | 默认值 | 说明 |
|:-------------:| :-------: | :------------------:| :------------------------:|
|  model_path  |   字符串   | "../data/models/retinaface_mobilenet0.25_int8_1b.bmodel" | retinaface模型路径 |
|  max_face_count   |   整数   | 50 | 最大的人脸数量 |
|  score_threshold  |   浮点数   | 0.1 | 目标检测置信度阈值 |
|  threshold_nms  |  浮点数 | 0.4 | 目标检测NMS IOU阈值 |
|  bgr2rgb  |   bool   | false | 解码器解出来的图像默认是bgr格式，是否需要将图像转换成rgb格式 |
|  mean  |   浮点数组   | 无 | 图像前处理均值，长度为3；计算方式为: y=(x-mean)/std；若bgr2rgb=true，数组中数组顺序需为r、g、b，否则需为b、g、r |
|  std  |   浮点数组   | 无 | 图像前处理方差，长度为3；计算方式同上；若bgr2rgb=true数组中数组顺序需为r、g、b，否则需为b、g、r |
|  stage    |   列表   | ["pre"]  | 标志前处理、推理、后处理三个阶段 |
|  shared_object |   字符串   |  "../../../build/lib/libretinaface.so"  | libretinaface 动态库路径 |
|     id      |    整数       | 0  | element id |
|  device_id  |    整数       |  0 | tpu 设备号 |
|     name    |    字符串     | "retinaface" | element 名称 |
|     side    |    字符串     | "sophgo"| 设备类型 |
| thread_number |    整数     | 1 | 启动线程数 |

> **注意**：
1. stage参数，需要设置为"pre"，"infer"，"post" 其中之一或相邻项的组合，并且按前处理-推理-后处理的顺序连接element。将三个阶段分配在三个element上的目的是充分利用各项资源，提高检测效率。
