# sophon-stream yolov8 element

English | [简体中文](README.md)

sophon-stream YOLOv8 element is a plugin in the sophon-stream framework, which is a simple, fast, and powerful detection model. This project has provided an example routine for this plugin, for more details please refer to [YOLOv8 Demo](../../../samples/yolov8/README_EN.md)

## 1. feature
* Support for multiple video streams
* Support for multi-threaded processing

## 2. Configuration Settings
The Sophon-Stream YOLOv8 plugin has several configurable parameters that can be adjusted according to specific requirements. Here are some commonly used parameters:

```json
{
    "configure": {
        "model_path": "../yolov8/data/models/BM1684X/yolov8s_int8_1b.bmodel",
        "threshold_conf": 0.5,
        "threshold_nms": 0.5,
        "bgr2rgb": true,
        "task_type": "Pose",
        "mean": [
            0,
            0,
            0
        ],
        "std": [
            255,
            255,
            255
        ],
        "roi": {
            "left": 600,
            "top": 400,
            "width": 800,
            "height": 600
        }
    },
    "shared_object": "../../build/lib/libyolov8.so",
    "name": "yolov8_group",
    "side": "sophgo",
    "thread_number": 4
}
```

|      Parameter Name    |    Type    | Default Value | Description |
|:-------------:| :-------: | :------------------:| :------------------------:|
|  model_path  |   string   | "../data/models/BM1684X/yolov8s_int8_1b.bmodel" | yolov8 model path |
|  threshold_conf   |   float/map   | 0.5 | Object detection confidence threshold. When set as a float number, all categories share the same threshold. When set as a map, different categories can have different thresholds. In second case, it's necessary to correctly set the class_names_file. |
|  threshold_nms  |   float   | 0.5 | NMS Threshold |
|  bgr2rgb  |   bool   | true | The images decoded by the decoder are in the default BGR format. whether a need to convert the images to the RGB format |
|  task_type   | string | "Detect" | yolov8 alg type, supports "Detect", "Cls", "Pose" and "Seg" |
|  mean  |   float[]   | \ | The image preprocessing requires mean values in an array of length 3. The formula used for calculation is `y=(x-mean)/std` . When bgr2rgb is set to true, the array should be in RGB order; otherwise, it should be in BGR order. |
|  std  |   float[]   | \ | The image preprocessing involves variance values in an array of length 3. The calculation method remains the same. When bgr2rgb is set to true, the array should be in RGB order; otherwise, it should be in BGR order. |
|  stage    |   queue   | ["pre"]  | The three stages include preprocessing, inference, and postprocessing. |
| roi | map | \ | Predefined ROI; when this parameter is configured, processing will only be applied to the region obtained from the ROI box. |
| class_names_file | string | \ | When threshold_conf is float , it doesn't take effect and can be left unset. However, when threshold_conf is set as a map, it is activated, requiring the path to the class name file. |
|  shared_object |   string   |  "../../../build/lib/libyolov8.so"  | libyolov8 dynamic library path |
|     id      |    int       | 0  | element id |
|  device_id  |    int       |  0 | tpu device id |
|     name    |    string     | "yolov8" | element name |
|     side    |    string     | "sophgo"| device type |
| thread_number |    int     | 1 | Number of the thread |
| seg_tpu_opt |    bool     | false | Yolov8_seg Specifies whether to use the TPU for post-processing |
| mask_bmodel_path |    string     | \ | The bmodel path of TPU post-processing when seg_tpu_opt is true |

> **notes**：
1. The `stage` parameter should be set as one of the following: "pre", "infer", "post", or their adjacent combinations. These stages should be connected in sequence to the elements, aligning with the order of preprocessing, inference, and post-processing. Distributing these three stages across three elements aims to maximize the utilization of resources, enhancing detection efficiency.