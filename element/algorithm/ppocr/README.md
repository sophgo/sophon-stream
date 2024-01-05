# sophon-stream ppocr element

[English](README_EN.md) | 简体中文

sophon-stream ppocr element 是 sophon-stream 框架中的一个插件，是一个简单、快速、强大的文字检测识别模型。本项目已提供此插件例程，详情请参见 [PPOCR Demo](../../../samples/ppocr/README.md)

## 特性

- 支持多路视频流
- 支持多线程处理

## 2. 配置参数

sophon-stream ppocr 插件具有一些可配置的参数，可以根据需求进行设置。以下是一些常用的参数：

检测部分：
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

|    参数名         |  类型  |                  默认值                                                    |               说明                |
| :--------------: | :----: | :------------------------------------------------------------------------: | :------------------------------: |
|  model_path      | 字符串 | "../ppocr/data/models/BM1684X/ch_PP-OCRv3_det_fp16_1b.bmodel"             |         检测模型路径               |
| bgr2rgb          | bool |      false                                                                   |            解码器解出来的图像默认是bgr格式，是否需要将图像转换成rgb格式                  |
|    mean         |  浮点数组  |                    无                                                      |       图像前处理均值，长度为3；计算方式为: y=(x-mean)/std；若bgr2rgb=true，数组中数组顺序需为r、g、b，否则需为b、g、r      |
|    std           |  浮点数组  |                    无                                                      |       图像前处理方差，长度为3；计算方式同上；若bgr2rgb=true数组中数组顺序需为r、g、b，否则需为b、g、r      |
|  shared_object   | 字符串 |    "../../build/lib/libppocr_det.so"                                       |       libppocr_det 动态库路径      |
|     name         | 字符串 |                 "ppocr_det_group"                                            |           element 名称            |
|     side         | 字符串 |                 "sophgo"                                                   |             设备类型             |
| thread_number    |  整数  |                    1                                                       |            启动线程数            |

识别部分：
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

|    参数名         |  类型  |                  默认值                                                    |               说明                |
| :--------------: | :----: | :------------------------------------------------------------------------: | :------------------------------: |
|  model_path      | 字符串 | ".../ppocr/data/models/BM1684X/ch_PP-OCRv3_rec_fp16_1b_320.bmodel"         |         识别模型路径          |
| beam_search     | bool |                                     false                                    |            bean_search          |
| beam_width      | 整数 |                                         3                                      |            search宽度          |
| class_names_file | 字符串 |      "../ppocr/data/datasets/ppocr_keys_v1.txt"                              |            类别名文件          |
|  shared_object   | 字符串 |    "../../build/lib/libppocr_rec.so"                                         |       libppocr_rec 动态库路径        |
|     name         | 字符串 |                 "ppocr_rec_group"                                            |           element 名称            |
|     side         | 字符串 |                 "sophgo"                                                   |             设备类型             |
| thread_number    |  整数  |                    1                                                       |            启动线程数            |


