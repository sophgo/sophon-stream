# sophon-stream lprnet element

[English](README_EN.md) | 简体中文

sophon-stream lprnet element 是 sophon-stream 框架中的一个插件，是一个简单、快速、强大的车牌识别模型。本项目已提供此 yolo 识别+lprnet 检测车牌的插件例程，详情请参见 [license_plate_recognition](../../../samples/license_plate_recognition/README.md)

## 特性

- 支持多路视频流
- 支持多线程处理

## 2. 配置参数

sophon-stream lprnet 插件分为预处理、推理、后处理三个部分，均具有一些可配置的参数，可以根据需求进行设置。以推理为例，是一些常用的参数：

```json
{
  "configure": {
    "model_path": "../models/BM1684/lprnet_fp32_1b.bmodel",
    "stage": ["infer"]
  },
  "shared_object": "../../../build/lib/liblprnet.so",
  "name": "lprnet",
  "side": "sophgo",
  "thread_number": 4
}
```

|    参数名     |  类型  |                  默认值                  |               说明               |
| :-----------: | :----: | :--------------------------------------: | :------------------------------: |
|  model_path   | 字符串 | "../models/BM1684/lprnet_fp32_1b.bmodel" |         lprnet 模型路径          |
|     stage     |  列表  |                 ["pre"]                  | 标志前处理、推理、后处理三个阶段 |
| shared_object | 字符串 |    "../../../build/lib/liblprnet.so"     |       liblprnet 动态库路径       |
|     name      | 字符串 |                 "lprnet"                 |           element 名称           |
|     side      | 字符串 |                 "sophgo"                 |             设备类型             |
| thread_number |  整数  |                    1                     |            启动线程数            |

> **注意**：

1. stage 参数，需要设置为"pre"，"infer"，"post" 其中之一或相邻项的组合，并且按前处理-推理-后处理的顺序连接 element。将三个阶段分配在三个 element 上的目的是充分利用各项资源，提高检测效率。
