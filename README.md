# sophon-stream

[English](README_EN.md) | 简体中文

## 1 简介

sophon-stream是面向算丰开发平台的数据流处理工具。本软件基于插件化的思想，使用C++17开发了一套支持多路数据流并发处理的流水线框架。基于现有的接口，sophon-stream对用户具有易使用、易二次开发的优点，可以大大简化用户配置工程或添加插件的复杂度。sophon-stream基于SophonSDK，可以充分发挥算丰硬件的编解码能力及深度学习算法的推理能力，从而获得较高的性能。

目前，本仓库已开源到github：https://github.com/sophgo/sophon-stream

教学视频已发布到Bilibili：https://www.bilibili.com/video/BV1ZpvDeXEQw

欢迎star、issue、pr！

主要目录结构和模块说明：

| 目录      | 模块                                             | 功能说明  |
| ------------------------|-------------------------------------------------------------------|---------------------| 
| [framework](./framework)| [framework](./framework)                                          | 框架                        |
| [element](./element)    | [yolov5](./element/algorithm/yolov5)                              | yolov5插件             |
|                         | [yolov7](./element/algorithm/yolov7)                              | yolov7插件             |
|                         | [yolov8](./element/algorithm/yolov8)                              | yolov8插件，支持检测、姿态、分类 |
|                         | [yolox](./element/algorithm/yolox)                                | yolox插件               |
|                         | [bytetrack](./element/algorithm/bytetrack)                        | bytetrack插件       |
|                         | [resnet](./element/algorithm/resnet)                              | resnet插件，支持分类、抽取特征  |
|                         | [openpose](./element/algorithm/openpose)                          | openpose插件       |
|                         | [retinaface](./element/algorithm/retinaface)                      | retinaface插件     |
|                         | [lprnet](./element/algorithm/lprnet)                              | lprnet插件            |
|                         | [decode](./element/multimedia/decode)                             | 解码插件               |
|                         | [encode](./element/multimedia/encode)                             | 编码插件               |
|                         | [osd](./element/multimedia/osd)                                   | 算法结果可视化插件       |
|                         | [distributor](./element/tools/distributor)                        | 数据分发插件       |
|                         | [converger](./element/tools/converger)                            | 数据汇聚插件       |
|                         | [faiss](./element/tools/faiss)                                    | faiss数据库插件         |
|                         | [blank](./element/tools/blank)                                    | 空白插件                |
| [samples](./samples)    | [yolov5](./samples/yolov5)                                        | yolov5 demo                             |
|                         | [yolov7](./samples/yolov7)                                        | yolov7 demo                            |
|                         | [yolov8](./samples/yolov8/)                                       | yolov8 demo                             |
|                         | [yolov8_obb](./samples/yolov8_obb/)                               | yolov8 obb demo                         |
|                         | [yolox](./samples/yolox)                                          | yolox demo                              |
|                         | [bytetrack](./samples/bytetrack)                                  | bytetrack demo                          |
|                         | [resnet](./samples/resnet)                                        | resnet demo                             |
|                         | [openpose](./samples/openpose)                                    | openpose demo                           |
|                         | [retinaface](./samples/retinaface)                                | retinaface demo                         |
|                         | [yolox_bytetrack_osd_encode](./samples/yolox_bytetrack_osd_encode)| 目标检测-跟踪-算法结果推流demo |
|                         | [yolov5_bytetrack_distributor_resnet_converger](./samples/yolov5_bytetrack_distributor_resnet_converger)| 目标检测-跟踪-分发-属性识别demo |
|                         | [retinaface_distributor_resnet_faiss_converger](./samples/retinaface_distributor_resnet_faiss_converger)| 人脸检测-分发-人脸识别demo |
|                         | [license_plate_recognition](./samples/license_plate_recognition/) | 车牌检测-车牌识别demo |
|                         | [ppocr](./samples/ppocr/)                                         | PPOCR demo |
|                         | [yolov5_fastpose_posec3d](./samples/yolov5_fastpose_posec3d/)     | 姿态识别-行为识别demo |
|                         | [bird_dwa_blend_encode](./samples/bird_dwa_blend_encode/)         | 鸟瞰拼接demo |
|                         | [dwa_blend_encode](./samples/dwa_blend_encode/)                   | 鱼眼拼接demo |
|                         | [dwa_dpu_encode](./samples/dwa_dpu_encode/)                       | 双目深度估计demo |
|                         | [gdwa_blend_encode](./samples/gdwa_blend_encode/)                 | 广角拼接demo |
|                         | [license_area_intrusion](./samples/license_area_intrusion/)       | 区域入侵demo |
|                         | [multi_graph](./samples/multi_graph/)                             | 多graph功能demo |
|                         | [structured_recognition](./samples/structured_recognition/)       | 单路码流配置不同算法demo |
|                         | [tripware](./samples/tripwire/)                                   | 越线检测demo |
|                         | [yolox_bytetrack_osd_qt](./samples/yolox_bytetrack_osd_qt/)       | 目标检测-跟踪-绘图-HDMI显示demo |

## 2 快速入门
请参考[sophon-stream用户文档](./docs/Sophon_Stream_User_Guide.md)

## 3 FAQ
请参考[sophon-stream常见问题及解答](./docs/FAQ.md)
