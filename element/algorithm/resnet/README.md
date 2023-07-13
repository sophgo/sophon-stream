# sophon-stream resnet element

sophon-stream resnet element是sophon-stream框架中的一个插件，是一个简单、快速、强大的分类模型。本项目已提供此插件例程，详情请参见 [ResNet Demo](../../../samples/resnet/README.md)

## 1. 特性
* 支持多路视频流
* 支持多线程处理

## 2. 配置参数
sophon-stream resnet插件具有一些可配置的参数，可以根据需求进行设置。以下是一些常用的参数：

```json
{
  "configure": {
    "model_path": "../data/models/BM1684X/resnet50_int8_4b.bmodel",
    "bgr2rgb": true,
    "mean": [
      0.229,
      0.224,
      0.225
    ],
    "std": [
      0.485,
      0.456,
      0.406
    ],
    "roi": {
      "left": 600,
      "top": 400,
      "width": 800,
      "height": 600
    }
  },
  "shared_object": "../../../build/lib/libresnet.so",
  "name": "resnet",
  "side": "sophgo",
  "thread_number": 1
}
```

|      参数名    |    类型    | 默认值 | 说明 |
|:-------------:| :-------: | :------------------:| :------------------------:|
|  model_path  |   字符串   | "../data/models/BM1684X/resnet_car_int8_4b.bmodel" | resnet模型路径 |
|  bgr2rgb  |   bool   | true | 解码器解出来的图像默认是bgr格式，是否需要将图像转换成rgb格式 |
|  mean  |   浮点数组   | [0.229,0.224,0.225] | 图像前处理均值，长度为3；计算方式为: y=(x-mean)/std；若bgr2rgb=true，数组中数组顺序需为r、g、b，否则需为b、g、r |
|  std  |   浮点数组   | [0.485,0.456,0.406] | 图像前处理方差，长度为3；计算方式同上；若bgr2rgb=true数组中数组顺序需为r、g、b，否则需为b、g、r |
| roi | map | 无 | 预设的ROI，配置了此参数时，只会对ROI框取的区域进行处理 |
|  shared_object |   字符串   |  "../../../build/lib/libresnet.so"  | libresnet 动态库路径 |
|     id      |    整数       | 0  | element id |
|  device_id  |    整数       |  0 | tpu 设备号 |
|     name    |    字符串     | "resnet" | element 名称 |
|     side    |    字符串     | "sophgo"| 设备类型 |
| thread_number |    整数     | 1 | 启动线程数 |
