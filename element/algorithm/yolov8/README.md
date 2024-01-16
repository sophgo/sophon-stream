# sophon-stream yolov8 element

[English](README_EN.md) | 简体中文

sophon-stream yolov8 element是sophon-stream框架中的一个插件，是一个简单、快速、强大的检测模型。本项目已提供此插件例程，详情请参见 [yolov8 Demo](../../../samples/yolov8/README.md)

## 1. 特性
* 支持多路视频流
* 支持多线程处理

## 2. 配置参数
sophon-stream yolov8插件具有一些可配置的参数，可以根据需求进行设置。以下是一些常用的参数：

```json
{
    "configure": {
        "model_path": "../yolov8/data/models/BM1684X/yolov8s_int8_1b.bmodel",
        "threshold_conf": 0.5,
        "threshold_nms": 0.5,
        "bgr2rgb": true,
        "mean": [
            0,
            0,
            0
        ],
        "std": [
            255,
            255,
            255
        ],
        "roi": {
            "left": 600,
            "top": 400,
            "width": 800,
            "height": 600
        }
    },
    "shared_object": "../../build/lib/libyolov8.so",
    "name": "yolov8_group",
    "side": "sophgo",
    "thread_number": 4
}
```

|      参数名    |    类型    | 默认值 | 说明 |
|:-------------:| :-------: | :------------------:| :------------------------:|
|  model_path  |   字符串   | "../data/models/BM1684X/yolov8s_int8_1b.bmodel" | yolov8模型路径 |
|  threshold_conf   |   浮点数或map   | 0.5 | 目标检测物体置信度阈值，设置为浮点数时，所有类别共用同一个阈值；设置为map时，不同类别可以使用不同阈值，此时还需要正确设置class_names_file |
|  threshold_nms  |   浮点数   | 0.5 | 目标检测NMS IOU阈值 |
|  bgr2rgb  |   bool   | true | 解码器解出来的图像默认是bgr格式，是否需要将图像转换成rgb格式 |
|  mean  |   浮点数组   | 无 | 图像前处理均值，长度为3；计算方式为: y=(x-mean)/std；若bgr2rgb=true，数组中数组顺序需为r、g、b，否则需为b、g、r |
|  std  |   浮点数组   | 无 | 图像前处理方差，长度为3；计算方式同上；若bgr2rgb=true数组中数组顺序需为r、g、b，否则需为b、g、r |
|  stage    |   列表   | ["pre"]  | 标志前处理、推理、后处理三个阶段 |
| roi | map | 无 | 预设的ROI，配置了此参数时，只会对ROI框取的区域进行处理 |
| class_names_file | 字符串 | 无 | threshold_conf为浮点数时不生效，可以不设置；当threshold_conf为map时启用，class name文件的路径 |
|  shared_object |   字符串   |  "../../../build/lib/libyolov8.so"  | libyolov8 动态库路径 |
|     id      |    整数       | 0  | element id |
|  device_id  |    整数       |  0 | tpu 设备号 |
|     name    |    字符串     | "yolov8" | element 名称 |
|     side    |    字符串     | "sophgo"| 设备类型 |
| thread_number |    整数     | 1 | 启动线程数 |

> **注意**：
1. stage参数，需要设置为"pre"，"infer"，"post" 其中之一或相邻项的组合，并且按前处理-推理-后处理的顺序连接element。将三个阶段分配在三个element上的目的是充分利用tpu和cpu资源，提高检测效率。

