# sophon-stream

## 1 简介

sophon-stream是基于插件的流处理框架。用户通过串联插件，可以快速构建流处理应用。

- 支持多图
- 单图内支持多模型串联、并联
- 插件支持多线程


主要目录结构和模块说明：

| 目录                     | 模块                                                              | 功能说明              |
| ------------------------|-------------------------------------------------------------------|---------------------| 
| [framework](./framework)| [framework](./framework)                                          | 框架                 |
| [element](./element)    | [yolov5](./element/algorithm/yolov5)                              | yolov5插件           |
|                         | [yolox](./element/algorithm/yolox)                                | yolox插件            |
|                         | [bytetrack](./element/algorithm/bytetrack)                        | bytetrack插件        |
|                         | [decode](./element/multimedia/decode)                             | 解码插件              |
|                         | [encode](./element/multimedia/encode)                             | 编码插件              |
|                         | [osd](./element/multimedia/osd)                                   | 算法结果可视化插件     |
| [samples](./samples)    | [yolov5](./samples/yolov5)                                        | yolov5 demo         |
|                         | [yolox](./samples/yolox)                                          | yolox demo          |
|                         | [bytetrack](./samples/bytetrack)                                  | bytetrack demo      |
|                         | [yolox_bytetrack_osd_encode](./samples/yolox_bytetrack_osd_encode)| 目标跟踪算法结果推流demo|


## 2 快速入门

## 3 FAQ
请参考[sophon-stream常见问题及解答](./docs/FAQ.md)