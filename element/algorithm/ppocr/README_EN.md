# sophon-stream ppocr element

English | [简体中文](README.md)

sophon-stream ppocr element is an element in sophon-stream. It is a simple, fast and powerful text detection and recognition model. This plug-in routine is available in this project, see[PPOCR Demo](../../../samples/ppocr/README.md)

## feature

- Support for multiple video streams
- Support for multi-threaded processing

## 2. Configuration Settings

The Sophon-Stream ppocr plugin has several configurable parameters that can be adjusted according to specific requirements. Here are some commonly used parameters:

detection part:

```json
{
    "configure": {
        "model_path": "../ppocr/data/models/BM1684X/ch_PP-OCRv3_det_fp16_1b.bmodel",
        "bgr2rgb": false,
        "mean": [
            123.675,
            116.28,
            103.53
        ],
        "std": [
            58.395,
            57.12,
            57.375
        ]
    },
    "shared_object": "../../build/lib/libppocr_det.so",
    "name": "ppocr_det_group",
    "side": "sophgo",
    "thread_number": 4
}
```

|    Parameter Name         |  Type  |                  Default Value                                                    |               Description                |
| :--------------: | :----: | :------------------------------------------------------------------------: | :------------------------------: |
|  model_path      | string | "../ppocr/data/models/BM1684X/ch_PP-OCRv3_det_fp16_1b.bmodel"             |         detection model path      |
| bgr2rgb          | bool |      false                                                                   |            The images decoded by the decoder are in the default BGR format. whether a need to convert the images to the RGB format                  |
|    mean         |  float[]  |                    \                                                      |       The image preprocessing requires mean values in an array of length 3. The formula used for calculation is y=(x-mean)/std . When bgr2rgb is set to true, the array should be in RGB order; otherwise, it should be in BGR order.      |
|    std           |  float[]  |                    \                                                     |       The image preprocessing involves variance values in an array of length 3. The calculation method remains the same. When bgr2rgb is set to true, the array should be in RGB order; otherwise, it should be in BGR order.      |
|  shared_object   | string |    "../../build/lib/libppocr_det.so"                                       |       libppocr_det dynamic library path      |
|     name         | string |                 "ppocr_det_group"                                            |           element name            |
|     side         | string |                 "sophgo"                                                   |             device type             |
| thread_number    |  int  |                    1                                                       |            Number of the thread            |

recognition part:

```json
{
  "configure": {
    "model_path": "../ppocr/data/models/BM1684X/ch_PP-OCRv3_rec_fp16_1b_320.bmodel",
    "beam_search": false,
    "beam_width": 3,
    "class_names_file": "../ppocr/data/datasets/ppocr_keys_v1.txt"
  },
  "shared_object": "../../build/lib/libppocr_rec.so",
  "name": "ppocr_rec_group",
  "side": "sophgo",
  "thread_number": 4
}

```

|    Parameter Name         |  Type  |                  Default Value                                                    |               Description               |
| :--------------: | :----: | :------------------------------------------------------------------------: | :------------------------------: |
|  model_path      | string | ".../ppocr/data/models/BM1684X/ch_PP-OCRv3_rec_fp16_1b_320.bmodel"         |         recognition model path    |
| beam_search     | bool |                                     false                                    |            bean_search          |
| beam_width      | int |                                         3                                      |            search width          |
| class_names_file | string |      "../ppocr/data/datasets/ppocr_keys_v1.txt"                              |            class names file      |
|  shared_object   | string |    "../../build/lib/libppocr_rec.so"                                         |       libppocr_rec dynamic library path        |
|     name         | string |                 "ppocr_rec_group"                                            |           element name            |
|     side         | string |                 "sophgo"                                                   |             device type             |
| thread_number    |  int  |                    1                                                       |            Number of the thread            |


