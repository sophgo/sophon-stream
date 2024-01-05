# sophon-stream posec3d element

[English](README_EN.md) | 简体中文

sophon-stream posec3d element 是 sophon-stream 框架中的一个插件，是一个简单、快速、强大的行为识别模型。本项目已提供Alphapose + PoseC3D 检测和识别行为的插件例程，详情请参见 [yolov5_fastpose_posec3d](../../../samples/yolov5_fastpose_posec3d/README.md)

## 特性

- 支持多路视频流
- 支持多线程处理

## 2. 配置参数

sophon-stream posec3d 插件分为预处理、推理、后处理三个部分，均具有一些可配置的参数，可以根据需求进行设置。以推理为例，是一些常用的参数：

```json
{
  "configure": {
    "model_path": "../yolov5_fastpose_posec3d/data/models/BM1684X/posec3d_ntu60_int8.bmodel",
    "class_names_file": "../yolov5_fastpose_posec3d/data/label_map_ntu60.txt",
    "frames_num": 72
  },
  "shared_object": "../../build/lib/libposec3d.so",
  "name": "posec3d_group",
  "side": "sophgo",
  "thread_number": 3
}
```

|    参数名         |  类型  |                  默认值                                                    |               说明                |
| :--------------: | :----: | :------------------------------------------------------------------------: | :------------------------------: |
|  model_path      | 字符串 | "../yolov5_fastpose_posec3d/data/models/BM1684X/posec3d_ntu60_int8.bmodel" |         posec3d 模型路径          |
| class_names_file | 字符串 |      "../yolov5_fastpose_posec3d/data/label_map_ntu60.txt"                 |            行为类别名文件          |
|    frames_num    |  整数  |                    72                                                      |       行为识别时一起处理的帧数      |
|  shared_object   | 字符串 |    "../../build/lib/libposec3d.so"                                         |       libposec3d 动态库路径        |
|     name         | 字符串 |                 "posec3d_group"                                            |           element 名称            |
|     side         | 字符串 |                 "sophgo"                                                   |             设备类型             |
| thread_number    |  整数  |                    1                                                       |            启动线程数            |

> **注意**：

1. 按前处理-推理-后处理的顺序连接 element。将三个阶段分配在三个 element 上的目的是充分利用 tpu 和 cpu 资源，提高检测效率。
