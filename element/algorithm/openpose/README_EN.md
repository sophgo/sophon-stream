# sophon-stream openpose element

English | [简体中文](README.md)

The sophon-stream openpose element is a plugin in the sophon-stream framework, serving as a simple, fast, and powerful pose recognition model. This project provides a sample routine for this plugin; for details, please refer to [openpose Demo](../../../samples/openpose/README.md).

## 1. Features
* Support for multiple video streams
* Support for multi-threaded processing

## 2. Configuration Parameters
The sophon-stream openpose plugin has some configurable parameters that can be set according to your needs. Here are some commonly used parameters:

```json
{
    "configure": {
        "model_path": "../data/models/BM1684X/pose_coco_int8_1b.bmodel",
        "threshold_nms": 0.05,
        "stage": [
            "pre"
        ]
    },
    "shared_object": "../../../build/lib/libopenpose.so",
    "name": "openpose",
    "side": "sophgo",
    "thread_number": 4
}
```

| Parameter Name | Type | Default Value | Description |
|:-------------:| :-------: | :------------------:| :------------------------:|
| model_path | String | "../data/models/BM1684X/pose_coco_int8_1b.bmodel" | Path to the openpose model |
| threshold_nms | Float | 0.05 | NMS IOU threshold for pose recognition |
| stage | List | ["pre"] | Flags for the three stages of pre-processing, inference, and post-processing |
| shared_object | String | "../../../build/lib/libopenpose.so" | Path to the libopenpose dynamic library |
| name | String | "openpose" | Element name |
| side | String | "sophgo" | Device type |
| thread_number | Integer | 1 | Number of threads to start |

> **Note**:
1. For the stage parameter, it needs to be set as one of "pre," "infer," "post," or a combination of adjacent items. These stages should be connected in the order of pre-processing, inference, and post-processing to elements. The purpose of allocating these three stages to three elements is to maximize the utilization of resources, enhancing the efficiency of detection.