# sophon-stream posec3d element

English | [简体中文](README.md)

The sophon-stream posec3d element is a plugin in the sophon-stream framework, serving as a simple, fast, and powerful behavior recognition model. This project provides a plugin routine for detecting and recognizing behaviors using Alphapose + PoseC3D. For details, please refer to [yolov5_fastpose_posec3d Demo](../../../samples/yolov5_fastpose_posec3d/README.md)

## 1. Features
* Support for multiple video streams
* Support for multi-threaded processing

## 2. Configuration Parameters
The sophon-stream posec3d plugin has some configurable parameters that can be set according to your needs. Here are some commonly used parameters:

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

| Parameter Name   | Type   | Default Value                                                       | Description                      |
| :--------------: | :----: | :------------------------------------------------------------------: | :-------------------------------: |
| model_path       | String | "../yolov5_fastpose_posec3d/data/models/BM1684X/posec3d_ntu60_int8.bmodel" | Path to the posec3d model        |
| class_names_file  | String | "../yolov5_fastpose_posec3d/data/label_map_ntu60.txt"                | File containing behavior class names |
| frames_num       | Integer| 72                                                                  | Number of frames to process together during behavior recognition |
| shared_object    | String | "../../build/lib/libposec3d.so"                                    | Path to the libposec3d dynamic library |
| name             | String | "posec3d_group"                                                   | Element name                     |
| side             | String | "sophgo"                                                           | Device type                      |
| thread_number    | Integer| 1                                                                   | Number of threads to start       |

> **Note**:
1. For the stage parameter, it needs to be set as one of "pre," "infer," "post," or a combination of adjacent items. These stages should be connected in the order of pre-processing, inference, and post-processing to elements. The purpose of allocating these three stages to three elements is to maximize the utilization of resources, enhancing the efficiency of detection.