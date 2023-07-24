# sophon-stream osd element

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
    "draw_utils": "OPENCV"
  },
  "shared_object": "../../../build/lib/libosd.so",
  "device_id": 0,
  "id": 0,
  "name": "osd",
  "side": "sophgo",
  "thread_number": 1
}
```

|      参数名      |  类型  |              默认值               |                 说明                 |
| :--------------: | :----: | :-------------------------------: | :----------------------------------: |
|     osd_type     | 字符串 |              "TRACK"              |    画图类型，包括 “DET”、“TRACK“     |
| class_names_file | 字符串 |                无                 |         class name文件的路径         |
|    draw_utils    | 字符串 |             "OPENCV"              |   画图工具，包括 "OPENCV"，"BMCV"    |
|  shared_object   | 字符串 | "../../../build/lib/libencode.so" |         libencode 动态库路径         |
|    device_id     |  整数  |                 0                 |              tpu 设备号              |
|        id        |  整数  |                 0                 |              element id              |
|       name       | 字符串 |               "osd"               |             element 名称             |
|       side       | 字符串 |             "sophgo"              |               设备类型               |
|  thread_number   |  整数  |                 4                 | 启动线程数，需要保证和处理码流数一致 |

> **注意**：
1. osd_type为"DET"时，需提供class_names_file文件地址