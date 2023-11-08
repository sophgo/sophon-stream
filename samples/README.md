# sophon-stream samples说明

## 1. 简介

本目录下包含sophon-stream提供的参考例程。

本目录结构如下所示：

```bash
./samples/
├── bytetrack                                       # 检测+跟踪例程
├── CMakeLists.txt                                  # cmake文件
├── include                                         # 绘图函数等头文件
├── license_plate_recognition                       # 车辆检测+车牌识别例程
├── openpose                                        # 姿态识别例程
├── README.md                                       # 用户手册
├── resnet                                          # 分类例程
├── retinaface                                      # 人脸检测例程
├── retinaface_distributor_resnet_faiss_converger   # 人脸检测+人脸识别例程
├── src                                             # 唯一的入口函数
├── yolov5                                          # yolov5检测例程
├── yolov5_bytetrack_distributor_resnet_converger   # 检测+跟踪+识别例程
├── yolox                                           # yolox检测例程
└── yolox_bytetrack_osd_encode                      # 检测+跟踪+画图+推流例程
```

本目录包含了多个例程，如人脸检测、车牌识别等。每个例程的目录下都包含配置文件和下载模型、视频等数据的脚本。对于各个例程而言，它们共用同一个入口函数，即`sophon-stream/samples/src/main.cc`。该入口函数主要作用是根据预设的路径，获取配置文件，解析例程的输入数据，并调用统一的底层接口配置所有的element，启动整个pipeline。

例如，为了运行`yolov5_bytetrack_distributor_resnet_converger`demo:

```bash
./main --demo_config_path=../yolov5_bytetrack_distributor_resnet_converger/config/yolov5_bytetrack_distributor_resnet_converger_demo.json
```

时，main函数会前往`yolov5_bytetrack_distributor_resnet_converger`例程目录下寻找配置文件，并根据其配置文件来搭建pipeline。此时，运行起来的pipeline就具有`检测+跟踪+识别`的功能。

或者，为了运行`license_plate_recognition`demo：

```bash
./main --demo_config_path=../license_plate_recognition/config/license_plate_recognition_demo.json
```

此种情况下，main函数则会前往`license_plate_recognition`例程目录下寻找配置文件。按照该配置文件搭建起来的pipeline，则具有`车辆检测+车牌识别`的功能。

这样设计的好处是隔离了用户与底层代码，使用户可以只关注于json文件的配置，而不需要考虑stream框架的运作逻辑。


> 注意
* 关于每个例程的详细信息，请参考每个例程目录下的README.md文件。