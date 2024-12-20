# sophon-stream lightstereo element

sophon-stream lightstereo element是sophon-stream框架中的一个插件，是一个简单、快速、强大的立体匹配模型。本项目已提供此插件例程，详情请参见 [dwa_lightstereo_encode Demo](../../../samples/dwa_lightstereo_encode/README.md)

## 1. 特性
* 支持2路输入
* 支持BM1688 SoC

## 2. 配置参数
sophon-stream lightstereo插件具有一些可配置的参数，可以根据需求进行设置。以下是一些常用的参数：

```json
{
    "configure": {
      "model_path": "../dwa_lightstereo_encode/data/models/BM1688/LightStereo-S-SceneFlow_int8_1b_480x736.bmodel",
      "bgr2rgb": true,
      "mean": [0.485, 0.456, 0.406],
      "std": [0.229, 0.224, 0.225],
      "stage": ["pre"]
    },
    "shared_object": "../../build/lib/liblightstereo.so",
    "name": "lightstereo",
    "side": "sophgo",
    "thread_number": 1
}

```

|      参数名    |    类型    | 默认值 | 说明 |
|:-------------:| :-------: | :------------------:| :------------------------:|
|  model_path  |   字符串   | "../dwa_lightstereo_encode/data/models/BM1688/LightStereo-S-SceneFlow_int8_1b_480x736.bmodel" | lightstereo模型路径 |
|  bgr2rgb  |   bool   | true | 解码器解出来的图像默认是bgr格式，是否需要将图像转换成rgb格式 |
|  mean  |   浮点数组   | [0.229,0.224,0.225] | 图像前处理均值，长度为3；计算方式为: y=(x-mean)/std；若bgr2rgb=true，数组中数组顺序需为r、g、b，否则需为b、g、r |
|  std  |   浮点数组   | [0.485,0.456,0.406] | 图像前处理方差，长度为3；计算方式同上；若bgr2rgb=true数组中数组顺序需为r、g、b，否则需为b、g、r |
|  stage |   字符串数组   |  pre  | 处理类型，分别有pre、infer、post，表示当前element做前处理、推理或后处理 |
|  shared_object |   字符串   |  "../../../build/lib/liblightstereo.so"  | liblightstereo.so 动态库路径 |
|     name    |    字符串     | "lightstereo" | element 名称 |
|     side    |    字符串     | "sophgo"| 设备类型 |
| thread_number |    整数     | 1 | 启动线程数，目前只支持每个element一个线程 |
