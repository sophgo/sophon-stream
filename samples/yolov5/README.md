# yolov5

## 1 概述

本例程基于[​YOLOv5官方开源仓库](https://github.com/ultralytics/yolov5)v6.1版本的模型和算法进行移植，使之适配sophon-stream开源框架，可以在SOPHON BM1684、BM1684X设备上对多路视频流进行实时处理。

## 2 编译

请参考[sophon-stream编译](../../docs/how_to_make.md)

## 3 运行

### 3.1 使用例程

例程位于[sophon-stream yolov5例程](./src/usecase_yolov5.cc)，其中包括解析配置文件、构建graph、为sink port设置数据处理函数等内容。

### 3.2 配置文件

配置文件位于[sophon-stream yolov5配置文件](./config/)，其中包括