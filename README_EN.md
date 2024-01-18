# sophon-stream

English | [简体中文](README.md)

## 1 Introduction

sophon-stream is a data stream processing tool designed for the Sophon development platform. This software is built on a modular concept and has been developed using C++11 to create a pipeline framework that supports concurrent processing of multiple data streams. Leveraging existing interfaces, sophon-stream offers users the advantages of ease of use and ease of secondary development, greatly simplifying the complexity of configuring projects or adding plugins. Sophon-stream is based on SophonSDK, allowing it to fully harness the hardware's encoding and decoding capabilities as well as the inference capabilities of artificial intelligence algorithms, thereby achieving higher performance.

Main Directory Structure and Module Descriptions:

| Catalogs      | Modules                                                     | Description   |
| ------------------------|-------------------------------------------------------------------|---------------------| 
| [framework](./framework)| [framework](./framework)                                          | framework                       |
| [element](./element)    | [yolov5](./element/algorithm/yolov5)                              | yolov5 plugin              |
|                         | [yolov7](./element/algorithm/yolov7)                              | yolov7 plugin          |
|                         | [yolox](./element/algorithm/yolox)                                | yolox plugin                |
|                         | [bytetrack](./element/algorithm/bytetrack)                        | bytetrack plugin        |
|                         | [resnet](./element/algorithm/resnet)                              | resnet plugin              |
|                         | [openpose](./element/algorithm/openpose)                          | openpose plugin          |
|                         | [retinaface](./element/algorithm/retinaface)                      | retinaface plugin      |
|                         | [lprnet](./element/algorithm/lprnet)                              | lprnet plugin              |
|                         | [decode](./element/multimedia/decode)                             | decode plugin                |
|                         | [encode](./element/multimedia/encode)                             | encode plugin                |
|                         | [osd](./element/multimedia/osd)                                   | osd plugin          |
|                         | [distributor](./element/tools/distributor)                        | distributor plugin        |
|                         | [converger](./element/tools/converger)                            | converger plugin          |
|                         | [faiss](./element/tools/faiss)                                    | faiss plugin          |
|                         | [blank](./element/tools/blank)                                    | blank plugin                 |
| [samples](./samples)    | [yolov5](./samples/yolov5)                                        | yolov5 demo                             |
|                         | [yolov7](./samples/yolov7)                                        | yolov7 demo                            |
|                         | [yolox](./samples/yolox)                                          | yolox demo                              |
|                         | [bytetrack](./samples/bytetrack)                                  | bytetrack demo                          |
|                         | [resnet](./samples/resnet)                                        | resnet demo                             |
|                         | [openpose](./samples/openpose)                                    | openpose demo                           |
|                         | [retinaface](./samples/retinaface)                                | retinaface demo                         |
|                         | [yolox_bytetrack_osd_encode](./samples/yolox_bytetrack_osd_encode)| detect-track-encode demo|
|                         | [yolov5_bytetrack_distributor_resnet_converger](./samples/yolov5_bytetrack_distributor_resnet_converger)| detect-track-distribute-recognize demo|
|                         | [retinaface_distributor_resnet_faiss_converger](./samples/retinaface_distributor_resnet_faiss_converge)| face detect-distribute-face recognize demo|
|                         | [license_plate_recognition](./samples/license_plate_recognition/) | car detect-license plate recognize demo |
|                         | [ppocr](./samples/ppocr/)                                         | PPOCR demo |
|                         | [yolov5_fastpose_posec3d](./samples/yolov5_fastpose_posec3d/)     | gesture recognition - behavior recognition demo |

## 2 Quick Start

Please refer to the [sophon-stream user guide](./docs/Sophon_Stream_User_Guide_EN.md)

## 3 FAQ

Please refer to the [sophon-stream FAQ](./docs/FAQ_EN.md)