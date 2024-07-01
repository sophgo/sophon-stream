# sophon-stream retinaface element

English | [简体中文](README.md)

The sophon-stream retinaface element is a plugin in the sophon-stream framework that utilizes a combination of additional supervised (extra-supervised) and self-supervised learning (self-supervised) through multi-task learning. It is designed for pixel-level localization of faces of different sizes. This project provides a sample routine for this plugin; for details, please refer to [retinaface Demo](../../../samples/retinaface/README.md).

## 1. Features
* Support for multiple video streams
* Support for multi-threaded processing

## 2. Configuration Parameters
The sophon-stream retinaface plugin has some configurable parameters that can be set according to your needs. Here are some commonly used parameters:

```json
{
    "configure": {
        "model_path": "../data/models/BM1684X/retinaface_mobilenet0.25_fp32_1b.bmodel",
        "max_face_count":50,
        "score_threshold":0.1,
        "threshold_nms": 0.4,
        "bgr2rgb": false,
        "mean": [
            104,
            117,
            123
        ],
        "std": [
            1,
            1,
            1
        ],
        "stage": [
            "pre"
        ]
    },
    "shared_object": "../../../build/lib/libretinaface.so",
    "name": "retinaface",
    "side": "sophgo",
    "thread_number": 1
}
```

| Parameter Name | Type | Default Value | Description |
|:-------------:| :-------: | :------------------:| :------------------------:|
| model_path | String | "../data/models/retinaface_mobilenet0.25_int8_1b.bmodel" | Path to the retinaface model |
| max_face_count | Integer | 50 | Maximum number of faces to detect |
| score_threshold | Float | 0.1 | Confidence threshold for target detection |
| threshold_nms | Float | 0.4 | NMS IOU threshold for target detection |
| bgr2rgb | Bool | false | Whether to convert the image from BGR to RGB format; the default is BGR |
| mean | Float Array | None | Mean values for image preprocessing, with a length of 3. The calculation is y=(x-mean)/std. If bgr2rgb=true, the order of the array should be R, G, B; otherwise, it should be B, G, R |
| std | Float Array | None | Standard deviations for image preprocessing, with a length of 3. The calculation is the same as above. If bgr2rgb=true, the order of the array should be R, G, B; otherwise, it should be B, G, R |
| stage | List | ["pre"] | Flags for the three stages of pre-processing, inference, and post-processing |
| shared_object | String | "../../../build/lib/libretinaface.so" | Path to the libretinaface dynamic library |
| id | Integer | 0 | Element ID |
| device_id | Integer | 0 | TPU device number |
| name | String | "retinaface" | Element name |
| side | String | "sophgo" | Device type |
| thread_number | Integer | 1 | Number of threads to start |

> **Note**:
1. For the stage parameter, it needs to be set as one of "pre," "infer," "post," or a combination of adjacent items. These stages should be connected in the order of pre-processing, inference, and post-processing to elements. The purpose of allocating these three stages to three elements is to maximize the utilization of resources, enhancing the efficiency of detection.