# sophon-stream osd element

English | [简体中文](README.md)

sophon-stream osd element is a plugin within the sophon-stream framework responsible for the visualization of algorithmic results. It supports the visualization of object detection and object tracking algorithm results.

## Content
- [sophon-stream osd element](#sophon-stream-osd-element)
  - [Content](#content)
  - [1. Feature](#1-Feature)
  - [2. Configuration parameters](#2-Configuration-parameters)

## 1. Feature
* Supports visualization of object detection and object tracking algorithm results.

![track.jpg](pics/track.jpg)

## 2. Configuration parameters
The sophon-stream osd plugin has several configurable parameters that can be adjusted according to specific requirements. Here are some commonly used parameters:

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

| Parameter Name   |  name  |        Default value             |                    Description         |
| :--------------: | :----: | :-------------------------------: | :-----------------------------------: |
|     osd_type     | string |              "TRACK"              | drawing type,include "DET","TRACK","POSE" |
| class_names_file | string |                \                 |        file path of class name        |
|    draw_utils    | string |             "OPENCV"              |    drawing function，include "OPENCV"，"BMCV"    |
|  draw_interval   | bool |               false               |         Whether to draw unsampled frames  |
|     put_text     | bool |               false               |             Whether to output text        |
|  shared_object   | string | "../../../build/lib/libosd.so" |         libosd dynamic library path  |
|    device_id     |  int  |                 0                 |              tpu device id               |
|        id        |  int  |                 0                 |              element id               |
|       name       | string |               "osd"               |             element name             |
|       side       | string |             "sophgo"              |               device type                |
|  thread_number   |  int  |                 4                 | Thread number, it should be consistent with the number of streams being processed.  |

> **notes**：
1. if osd_type is "DET", the address of the class_names_file should be provided.
