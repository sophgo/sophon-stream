# sophon-stream yolox element

English | [简体中文](README.md)

sophon-stream YOLOX element is a plugin in the sophon-stream framework, which is a simple, fast, and powerful detection model. This project has provided an example routine for this plugin, for more details please refer to [YOLOX Demo](../../../samples/yolox/README_EN.md)

## 1. feature
* Support for multiple video streams
* Support for multi-threaded processing

## 2. Configuration Settings
The Sophon-Stream YOLOX plugin has several configurable parameters that can be adjusted according to specific requirements. Here are some commonly used parameters:

```json
{
  "configure": {
    "model_path": "../data/models/BM1684X/yolox_s_int8_4b.bmodel",
    "threshold_conf": 0.5,
    "threshold_nms": 0.5,
    "bgr2rgb": true,
    "mean": [
      0,
      0,
      0
    ],
    "std": [
      0.0039216,
      0.0039216,
      0.0039216
    ],
    "stage": [
      "pre"
    ],
    "roi": {
      "left": 600,
      "top": 400,
      "width": 800,
      "height": 600
    }
  },
  "shared_object": "../../../build/lib/libyolox.so",
  "id": 0,
  "device_id": 0,
  "name": "yolox",
  "side": "sophgo",
  "thread_number": 2
}
```

| Parameter Name | Type             | Default Value                                      | Description                                              |
|:--------------:|:----------------:|:--------------------------------------------------:|:--------------------------------------------------------:|
| model_path     | String           | "../data/models/BM1684X/yolox_s_int8_4b.bmodel"   | Path to the yolox model                                  |
| threshold_conf | Float or Map     | 0.5                                               | Confidence threshold for object detection; when set as a float, all classes share the same threshold; when set as a map, different classes can have different thresholds, and class_names_file should be correctly set |
| threshold_nms  | Float            | 0.5                                               | NMS IOU threshold for object detection                   |
| bgr2rgb        | Boolean          | true                                              | Whether to convert the image from BGR to RGB format, as the decoder output is in BGR format by default |
| mean           | Float Array      | None                                              | Image preprocessing mean values, length 3; calculation: y=(x-mean)/std; if bgr2rgb=true, the array order should be r, g, b, otherwise b, g, r |
| std            | Float Array      | None                                              | Image preprocessing standard deviation values, length 3; calculation as above; if bgr2rgb=true, the array order should be r, g, b, otherwise b, g, r |
| stage          | List             | ["pre"]                                           | Flags for pre-processing, inference, and post-processing stages |
| roi            | Map              | None                                              | Preset Region of Interest (ROI); when configured, processing will only be performed on the region enclosed by the ROI box |
| class_names_file| String           | None                                              | Not effective when threshold_conf is a float; can be omitted; when threshold_conf is a map, the path to the class name file |
| shared_object  | String           | "../../../build/lib/libyolox.so"                   | Path to the libyolox dynamic library                     |
| id             | Integer          | 0                                                | Element ID                                               |
| device_id      | Integer          | 0                                                | TPU device number                                        |
| name           | String           | "yolox"                                          | Element name                                             |
| side           | String           | "sophgo"                                         | Device type                                              |
| thread_number  | Integer          | 1                                                | Number of threads to launch                              |

> **notes**：
1. The `stage` parameter should be set as one of the following: "pre", "infer", "post", or their adjacent combinations. These stages should be connected in sequence to the elements, aligning with the order of preprocessing, inference, and post-processing. Distributing these three stages across three elements aims to maximize the utilization of resources, enhancing detection efficiency.