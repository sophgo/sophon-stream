# sophon-stream

## 1 简介

sophon-stream是面向算丰开发平台的数据流处理工具。本软件基于插件化的思想，使用C++11开发了一套支持多路数据流并发处理的流水线框架。基于现有的接口，sophon-stream对用户具有易使用、易二次开发的优点，可以大大简化用户配置工程或添加插件的复杂度。sophon-stream基于SophonSDK，可以充分发挥算丰硬件的编解码能力及人工智能算法的推理能力，从而获得较高的性能。

主要目录结构和模块说明：

| 目录                     | 模块                                                              | 功能说明              |
| ------------------------|-------------------------------------------------------------------|---------------------| 
| [framework](./framework)| [framework](./framework)                                          | 框架                 |
| [element](./element)    | [yolov5](./element/algorithm/yolov5)                              | yolov5插件           |
|                         | [yolox](./element/algorithm/yolox)                                | yolox插件            |
|                         | [bytetrack](./element/algorithm/bytetrack)                        | bytetrack插件        |
|                         | [resnet](./element/algorithm/resnet)                              | resnet插件           |
|                         | [decode](./element/multimedia/decode)                             | 解码插件              |
|                         | [encode](./element/multimedia/encode)                             | 编码插件              |
|                         | [osd](./element/multimedia/osd)                                   | 算法结果可视化插件     |
|                         | [distributor](./element/tools/distributor)                        | 数据分发插件          |
|                         | [converger](./element/tools/converger)                            | 数据汇聚插件          |
| [samples](./samples)    | [yolov5](./samples/yolov5)                                        | yolov5 demo         |
|                         | [yolox](./samples/yolox)                                          | yolox demo          |
|                         | [bytetrack](./samples/bytetrack)                                  | bytetrack demo      |
|                         | [resnet](./samples/resnet)                                        | resnet demo      |
|                         | [yolox_bytetrack_osd_encode](./samples/yolox_bytetrack_osd_encode)| 目标检测-跟踪-算法结果推流demo|
|                         | [yolov5_bytetrack_distributor_resnet_converger](./samples/yolov5_bytetrack_distributor_resnet_converger)| 目标检测-跟踪-分发-属性识别demo|

## 2 快速入门
请参考[sophon-stream用户文档](./docs/Sophon_Stream_User_Guide.md)

## 3 FAQ
请参考[sophon-stream常见问题及解答](./docs/FAQ.md)