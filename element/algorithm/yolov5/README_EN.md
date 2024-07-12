# sophon-stream yolov5 element

English | [简体中文](README.md)

sophon-stream YOLOv5 element is a plugin in the sophon-stream framework, which is a simple, fast, and powerful detection model. This project has provided an example routine for this plugin, for more details please refer to [YOLOv5 Demo](../../../samples/yolov5/README_EN.md)

## 1. feature
* Support for multiple video streams
* Support for multi-threaded processing

## 2. Configuration Settings
The Sophon-Stream YOLOv5 plugin has several configurable parameters that can be adjusted according to specific requirements. Here are some commonly used parameters:

```json
{
    "configure":{
        "model_path":"../data/models/yolov5s_tpukernel_int8_4b.bmodel",
        "threshold_conf":0.5,
        "threshold_nms":0.5,
        "bgr2rgb": true,
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
        "use_tpu_kernel": true,
        "roi": {
            "left": 600,
            "top": 400,
            "width": 800,
            "height": 600
        },
        "maxdet":1280,
        "mindet":50
    },
    "shared_object":"../../../build/lib/libyolov5.so",
    "id":0,
    "device_id":0,
    "name":"yolov5_group",
    "side":"sophgo",
    "thread_number":1
}
```

|      Parameter Name    |    Type    | Default Value | Description |
|:-------------:| :-------: | :------------------:| :------------------------:|
|  model_path  |   string   | "../data/models/yolov5s_tpukernel_int8_4b.bmodel" | yolov5 model path |
|  threshold_conf   |   float/map   | 0.5 | Object detection confidence threshold. When set as a float number, all categories share the same threshold. When set as a map, different categories can have different thresholds. In second case, it's necessary to correctly set the class_names_file. |
|  threshold_nms  |   float   | 0.5 | NMS Threshold |
|  bgr2rgb  |   bool   | true | The images decoded by the decoder are in the default BGR format. whether a need to convert the images to the RGB format |
|  mean  |   float[]   | \ | The image preprocessing requires mean values in an array of length 3. The formula used for calculation is `y=(x-mean)/std` . When bgr2rgb is set to true, the array should be in RGB order; otherwise, it should be in BGR order. |
|  std  |   float[]   | \ | The image preprocessing involves variance values in an array of length 3. The calculation method remains the same. When bgr2rgb is set to true, the array should be in RGB order; otherwise, it should be in BGR order. |
|  stage    |   queue   | ["pre"]  | The three stages include preprocessing, inference, and postprocessing. |
| roi | map | \ | Predefined ROI; when this parameter is configured, processing will only be applied to the region obtained from the ROI box. |
|  use_tpu_kernel  |   bool    |  true | Whether to enable post-processing with TPU kernel |
| class_names_file | string | \ | When threshold_conf is float , it doesn't take effect and can be left unset. However, when threshold_conf is set as a map, it is activated, requiring the path to the class name file. |
|  shared_object |   string   |  "../../../build/lib/libyolov5.so"  | libyolov5 dynamic library path |
|     id      |    int       | 0  | element id |
|  device_id  |    int       |  0 | tpu device id |
|     name    |    string     | "yolov5" | element name |
|     side    |    string     | "sophgo"| device type |
| thread_number |    int     | 1 | Number of the thread |
|Maxdet | integer | MAX_ INT | Only accepts detection boxes with width and height less than maxdet|
|Mindet | integer | 0 | Only accept detection boxes with width and height greater than mindet|

> **notes**：
1. The `stage` parameter should be set as one of the following: "pre", "infer", "post", or their adjacent combinations. These stages should be connected in sequence to the elements, aligning with the order of preprocessing, inference, and post-processing. Distributing these three stages across three elements aims to maximize the utilization of resources, enhancing detection efficiency.
2. TPU kernel post-processing is specifically designed for BM1684X devices. If it's not enabled, it should be set to false.

## 3. Dynamic parameter modification

Currently, the yolov5 plugin supports the modification of certain parameters at stream runtime via external http requests. The code provides the ability to dynamically modify confidence thresholds, which can be verified using the following python script:

``` python
import requests
import json
import sys

url = "http://localhost:8000/yolov5/SetConfThreshold/10003"
payload = {"value": 1.0}
headers = {'Content-Type': 'application/json'}
response = requests.request("POST", url, headers=headers, data=json.dumps(payload))
print(response) 
```

where 10003 is the id of the yolov5 plugin at the time of the actual run; the value field in the request indicates the confidence threshold that is expected to be set. For example, the above request would change the confidence level to 1.0, meaning that the target would not be detected in almost any case.

This confidence threshold, as currently set, only takes effect when cpu post-processing is enabled.

> **Note: To enable the dynamic parameter modification feature, you need to refer to [README.md](... /... /... /samples/README.md) to set the ip and port to listen to**.