# sophon-stream osd element

[English](README_EN.md) | 简体中文

sophon-stream osd element是sophon-stream框架中的一个插件，负责算法结果的可视化，支持目标检测、目标跟踪算法结果可视化

## 目录
- [sophon-stream osd element](#sophon-stream-osd-element)
  - [目录](#目录)
  - [1. 特点](#1-特点)
  - [2. 配置参数](#2-配置参数)

## 1. 特点
* 支持目标检测、目标跟踪算法结果的可视化

![track.jpg](pics/track.jpg)

## 2. 配置参数
sophon-stream osd插件具有一些可配置的参数，可以根据需求进行设置。以下是一些常用的参数：

```json
{
  "configure": {
    "osd_type": "TRACK",
    "class_names_file": "../data/coco.names",
    "draw_utils": "OPENCV",
    "draw_interval": false,
    "put_text": false
  },
  "shared_object": "../../../build/lib/libosd.so",
  "device_id": 0,
  "id": 0,
  "name": "osd",
  "side": "sophgo",
  "thread_number": 1
}
```

|      参数名      |  类型  |              默认值               |                 说明                  |
| :--------------: | :----: | :-------------------------------: | :-----------------------------------: |
|     osd_type     | 字符串 |              "TRACK"              | 画图类型，包括 "DET"、"TRACK"、"POSE"、"ALGORITHM"、"TEXT" ，其中ALGORITHM代表使用draw_func_name所对应的osd函数，TEXT代表在原图任意位置使用硬件绘制文字|
| class_names_file | 字符串 |                无                 |         class name文件的路径          |
| recognice_names_file | 字符串 |                无             |         如果有识别子任务的话，表示识别类别名字文件的路径          |
|    draw_utils    | 字符串 |             "OPENCV"              |    画图工具，包括 "OPENCV"，"BMCV"    |
|  draw_interval   | 布尔值 |               false               |          是否画出未采样的帧           |
|     put_text     | 布尔值 |               false               |             是否输出文本              |
|    draw_func_name    | 字符串 |             "default"              |    对应不同ALGORITHM中的osd方式    |
|  heatmap_loss  |   字符串   | "MSELoss" | 姿态识别训练所使用的损失函数，暂只支持MSELoss |
|    tops     |  整数数组  |                 无                 |              在TEXT模式下，texts中每个字符串距离图片顶部的垂直距离               |
|    lefts     |  整数数组  |                 无                 |              在TEXT模式下，texts中每个字符串距离图片左侧的水平距离               |
|    texts     |  字符串数组  |                 无                 |              在TEXT模式下，要显示的文本内容组成的数组               |
|    font_library     |  字符串  |                 无                 |              在TEXT模式下使用的字体库文件的路径               |
|    r    |  整数  |                 0                 |              TEXT模式中文字颜色的r通道值               |
|    g    |  整数  |                 0                 |              TEXT模式中文字颜色的g通道值               |
|    b     |  整数  |                 0                 |              TEXT模式中文字颜色的b通道值               |
|  shared_object   | 字符串 | "../../../build/lib/libosd.so" |         libosd 动态库路径          |
|    device_id     |  整数  |                 0                 |              tpu 设备号               |
|        id        |  整数  |                 0                 |              element id               |
|       name       | 字符串 |               "osd"               |             element 名称              |
|       side       | 字符串 |             "sophgo"              |               设备类型                |
|  thread_number   |  整数  |                 4                 | 启动线程数，需要保证和处理码流数一致  |

> **注意**：
1. osd_type为"DET"时，需提供class_names_file文件地址
