# sophon-stream yolov5 element

sophon-stream yolov5 element是sophon-stream框架中的一个插件，是一个简单、快速、强大的检测模型。本项目已提供此插件例程，详情请参见 [YOLOv5 Demo](../../../samples/yolov5/README.md)

## 1. 特性
* 支持多路视频流
* 支持多线程处理

## 2. 配置参数
sophon-stream yolov5插件具有一些可配置的参数，可以根据需求进行设置。以下是一些常用的参数：

```json
{
    "configure":{
        "model_path":"../data/models/yolov5s_tpukernel_int8_4b.bmodel",
        "threshold_conf":0.5,
        "threshold_nms":0.5,
        "stage":["pre"],
        "use_tpu_kernel": true
    },
    "shared_object":"../../../build/lib/libyolov5.so",
    "device_id":0,
    "name":"yolov5",
    "side":"sophgo",
    "thread_number":1
}
```

|      参数名    |    类型    | 默认值 | 说明 |
|:-------------:| :-------: | :------------------:| :------------------------:|
|  model_path  |   字符串   | "../data/models/yolov5s_tpukernel_int8_4b.bmodel" | yolov5模型路径 |
|  threshold_conf   |   浮点数   | 0.5 | 目标检测物体置信度阈值 |
|  threshold_nms  |   浮点数   | 0.5 | 目标检测NMS IOU阈值 |
|  stage    |   列表   | ["pre"]  | 标志前处理、推理、后处理三个阶段 |
|  use_tpu_kernel  |   布尔值    |  true | 是否启用tpu_kernel后处理 |
|  shared_object |   字符串   |  "../../../build/lib/libyolov5.so"  | libyolov5 动态库路径 |
|  device_id  |    整数       |  0 | tpu 设备号 |
|     name    |    字符串     | "yolov5" | element 名称 |
|     side    |    字符串     | "sophgo"| 设备类型 |
| thread_number |    整数     | 1 | 启动线程数 |

> **注意**：
1. stage参数，需要设置为"pre"，"infer"，"post" 其中之一或相邻项的组合，并且按前处理-推理-后处理的顺序连接element。将三个阶段分配在三个element上的目的是充分利用tpu和cpu资源，提高检测效率。
3. tpu_kernel后处理仅适配BM1684X设备，若不启用，则需要设置为false