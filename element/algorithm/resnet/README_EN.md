# sophon-stream resnet element

English | [简体中文](README.md)

sophon-stream resnet element is a plugin in the sophon-stream framework, and it is a simple, fast, and powerful classification model. This project provides a sample routine for this plugin; for details, please refer to [ResNet Demo](../../../samples/resnet/README.md).

## 1. Features
* Support for multiple video streams
* Support for multi-threaded processing

## 2. Configuration Parameters
The sophon-stream resnet plugin has some configurable parameters that can be set according to your needs. Here are some commonly used parameters:

```json
{
  "configure": {
    "model_path": "../data/models/BM1684X/resnet50_int8_4b.bmodel",
    "bgr2rgb": true,
    "mean": [
      0.229,
      0.224,
      0.225
    ],
    "std": [
      0.485,
      0.456,
      0.406
    ],
    "roi": {
      "left": 600,
      "top": 400,
      "width": 800,
      "height": 600
    },
    "task_type": "SingleLabel",
    "class_thresh": [0.5, 0.3, 0.7]
  },
  "shared_object": "../../../build/lib/libresnet.so",
  "name": "resnet",
  "side": "sophgo",
  "thread_number": 1
}
```

| Parameter Name | Type | Default Value | Description |
|:-------------:| :-------: | :------------------:| :------------------------:|
| model_path | String | "../data/models/BM1684X/resnet_car_int8_4b.bmodel" | Path to the resnet model |
| bgr2rgb | Bool | true | Whether to convert the image from BGR to RGB format; the default is BGR |
| mean | Float Array | [0.229,0.224,0.225] | Mean values for image preprocessing, with a length of 3. The calculation is y=(x-mean)/std. If bgr2rgb=true, the order of the array should be R, G, B; otherwise, it should be B, G, R |
| std | Float Array | [0.485,0.456,0.406] | Standard deviations for image preprocessing, with a length of 3. The calculation is the same as above. If bgr2rgb=true, the order of the array should be R, G, B; otherwise, it should be B, G, R |
| roi | Map | None | Preset ROI; when this parameter is configured, only the region defined by the ROI will be processed |
| task_type | String | Work type of resnet. `SingleLabel` means output a label with max score; `FeatureExtract` means output the feature vector; and `MultiLabel` means output multi-labels, which needs `class_thresh` in use. |
| class_thresh | list | None |  |
| shared_object | String | "../../../build/lib/libresnet.so" | Path to the libresnet dynamic library |
| id | Integer | 0 | Element ID |
| device_id | Integer | 0 | TPU device number |
| name | String | "resnet" | Element name |
| side | String | "sophgo" | Device type |
| thread_number | Integer | 1 | Number of threads to start |
