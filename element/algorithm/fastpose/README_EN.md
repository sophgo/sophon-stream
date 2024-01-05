# sophon-stream fastpose element

English | [简体中文](README.md)

sophon-stream fastpose element is a plugin in the sophon-stream framework, and it is a simple, fast, and powerful pose recognition model. This project provides a sample routine for this plugin; for details, please refer to [yolov5_fastpose Demo](../../../samples/yolov5_fastpose/README.md).

## 1. Features
* Support for multiple video streams
* Support for multi-threaded processing

## 2. Configuration Parameters
The sophon-stream fastpose plugin has some configurable parameters that can be set according to your needs. Here are some commonly used parameters:

```json
{
    "configure": {
        "model_path": "../data/models/fastpose/BM1684X/halpe26_fast_res50_256x192_int8_1b.bmodel",
        "stage": [
            "pre"
        ],
        "heatmap_loss": "MSELoss",
        "area_thresh": 0.0
    },
    "shared_object": "../../../build/lib/libfastpose.so",
    "name": "fastpose",
    "side": "sophgo",
    "thread_number": 2
}
```

| Parameter Name | Type | Default Value | Description |
|:-------------:| :-------: | :------------------:| :------------------------:|
| model_path | String | "../data/models/fastpose/BM1684X/halpe26_fast_res50_256x192_int8_1b.bmodel" | Path to the fastpose model |
| stage | List | ["pre"] | Flags for the three stages of pre-processing, inference, and post-processing |
| heatmap_loss | String | "MSELoss" | Loss function used for pose recognition training, currently only supports MSELoss |
| area_thresh | Float | 0.0 | Threshold in pose recognition |
| shared_object | String | "../../../build/lib/libfastpose.so" | Path to the libfastpose dynamic library |
| name | String | "fastpose" | Element name |
| side | String | "sophgo" | Device type |
| thread_number | Integer | 2 | Number of threads to start |

> **Note**:
1. For the stage parameter, it needs to be set as one of "pre," "infer," "post," or a combination of adjacent items. These stages should be connected in the order of pre-processing, inference, and post-processing to elements. The purpose of allocating these three stages to three elements is to maximize the utilization of TPU and CPU resources, enhancing the efficiency of detection.
2. fastpose relies on a preceding detector and should be used in conjunction with a detector that has the capability to detect objects with the class "person."