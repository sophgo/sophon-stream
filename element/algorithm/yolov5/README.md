# sophon-stream yolov5 element

[English](README_EN.md) | 简体中文

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
        "use_tpu_kernel": true,
        "roi": {
            "left": 600,
            "top": 400,
            "width": 800,
            "height": 600
        },
        "maxdet":1280,
        "mindet":50
    },
    "shared_object":"../../../build/lib/libyolov5.so",
    "id":0,
    "device_id":0,
    "name":"yolov5_group",
    "side":"sophgo",
    "thread_number":1
}
```

|      参数名    |    类型    | 默认值 | 说明 |
|:-------------:| :-------: | :------------------:| :------------------------:|
|  model_path  |   字符串   | "../data/models/yolov5s_tpukernel_int8_4b.bmodel" | yolov5模型路径 |
|  threshold_conf   |   浮点数或map   | 0.5 | 目标检测物体置信度阈值，设置为浮点数时，所有类别共用同一个阈值；设置为map时，不同类别可以使用不同阈值，此时还需要正确设置class_names_file |
|  threshold_nms  |   浮点数   | 0.5 | 目标检测NMS IOU阈值 |
|  bgr2rgb  |   bool   | true | 解码器解出来的图像默认是bgr格式，是否需要将图像转换成rgb格式 |
|  mean  |   浮点数组   | 无 | 图像前处理均值，长度为3；计算方式为: y=(x-mean)/std；若bgr2rgb=true，数组中数组顺序需为r、g、b，否则需为b、g、r |
|  std  |   浮点数组   | 无 | 图像前处理方差，长度为3；计算方式同上；若bgr2rgb=true数组中数组顺序需为r、g、b，否则需为b、g、r |
|  stage    |   列表   | ["pre"]  | 标志前处理、推理、后处理三个阶段 |
| roi | map | 无 | 预设的ROI，配置了此参数时，只会对ROI框取的区域进行处理 |
|  use_tpu_kernel  |   布尔值    |  true | 是否启用tpu_kernel后处理 |
| class_names_file | 字符串 | 无 | threshold_conf为浮点数时不生效，可以不设置；当threshold_conf为map时启用，class name文件的路径 |
|  shared_object |   字符串   |  "../../../build/lib/libyolov5.so"  | libyolov5 动态库路径 |
|     id      |    整数       | 0  | element id |
|  device_id  |    整数       |  0 | tpu 设备号 |
|     name    |    字符串     | "yolov5" | element 名称 |
|     side    |    字符串     | "sophgo"| 设备类型 |
| thread_number |    整数     | 1 | 启动线程数 |
|   maxdet    |    整数     | MAX_INT| 仅接受宽高都小于maxdet的检测框 |
|   mindet    |    整数     | 0 | 仅接受宽高都大于mindet的检测框 |

> **注意**：
1. stage参数，需要设置为"pre"，"infer"，"post" 其中之一或相邻项的组合，并且按前处理-推理-后处理的顺序连接element。将三个阶段分配在三个element上的目的是充分利用各项资源，提高检测效率。
3. tpu_kernel后处理仅适配BM1684X设备，若不启用，则需要设置为false


## 3. 动态修改参数

目前，yolov5插件支持在stream运行时通过外部http请求修改某些参数。代码中提供了动态修改置信度阈值的功能，可以使用如下python脚本进行验证：

```python
import requests
import json
import sys

url = "http://localhost:8000/yolov5/SetConfThreshold/10003"
payload = {"value": 1.0}
headers = {'Content-Type': 'application/json'}
response = requests.request("POST", url, headers=headers, data=json.dumps(payload))
print(response)
```

其中，10003为实际运行时yolov5插件的id；请求中value字段表示期望设置的置信度阈值。例如，上述请求会将置信度改为1.0，也就是说几乎任何情况都无法检测到目标。

目前设置的此置信度阈值，只有启用cpu后处理时生效。

> **需要注意：启用动态修改参数功能，需要参考 [README.md](../../../samples/README.md) 设置监听的ip和端口**