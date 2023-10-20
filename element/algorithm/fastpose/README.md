# sophon-stream fastpose element

sophon-stream fastpose element是sophon-stream框架中的一个插件，是一个简单、快速、强大的姿态识别模型。本项目已提供此插件例程，详情请参见 [yolov5_fastpose Demo](../../../samples/yolov5_fastpose/README.md)

## 1. 特性
* 支持多路视频流
* 支持多线程处理

## 2. 配置参数
sophon-stream fastpose插件具有一些可配置的参数，可以根据需求进行设置。以下是一些常用的参数：

```json

{
    "configure": {
        "model_path": "../data/models/fastpose/BM1684X/halpe26_fast_res50_256x192_int8_1b.bmodel",
        "stage": [
            "pre"
        ],
        "heatmap_loss": "MSELoss",
        "area_thresh": 0.0
    },
    "shared_object": "../../../build/lib/libfastpose.so",
    "name": "fastpose",
    "side": "sophgo",
    "thread_number": 2
}
```

|      参数名    |    类型    | 默认值 | 说明 |
|:-------------:| :-------: | :------------------:| :------------------------:|
|  model_path  |   字符串   | "../data/models/fastpose/BM1684X/halpe26_fast_res50_256x192_int8_1b.bmodel" | fastpose模型路径 |
|  stage    |   列表   | ["pre"]  | 标志前处理、推理、后处理三个阶段 |
|  heatmap_loss  |   字符串   | "MSELoss" | 姿态识别训练所使用的损失函数，暂只支持MSELoss |
|  area_thresh |   浮点数   |  0.0  | 姿态识别中的阈值 |
|  shared_object |   字符串   |  "../../../build/lib/libfastpose.so"  | libfastpose 动态库路径 |
|     name    |    字符串     | "fastpose" | element 名称 |
|     side    |    字符串     | "sophgo"| 设备类型 |
| thread_number |    整数     | 2 | 启动线程数 |

> **注意**：
1. stage参数，需要设置为"pre"，"infer"，"post" 其中之一或相邻项的组合，并且按前处理-推理-后处理的顺序连接element。将三个阶段分配在三个element上的目的是充分利用tpu和cpu资源，提高检测效率。
2. fastpose依赖前序的检测器，需要搭配具有类别“person”检测能力的检测器一起使用。
