# sophon-stream samples说明

[English](README_EN.md) | 简体中文

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

此时，main函数会前往`yolov5_bytetrack_distributor_resnet_converger`例程目录下寻找配置文件，并根据其配置文件来搭建pipeline。此时，运行起来的pipeline就具有`检测+跟踪+识别`的功能。

或者，为了运行`license_plate_recognition`demo：

```bash
./main --demo_config_path=../license_plate_recognition/config/license_plate_recognition_demo.json
```

此种情况下，main函数则会前往`license_plate_recognition`例程目录下寻找配置文件。按照该配置文件搭建起来的pipeline，则具有`车辆检测+车牌识别`的功能。

这样设计的好处是隔离了用户与底层代码，使用户可以只关注于json文件的配置，而不需要考虑stream框架的运作逻辑。


> 注意
* 关于每个例程的详细信息，请参考每个例程目录下的README.md文件。

## 2. 配置参数
sophon-stream demo具有一些可配置的参数，可以根据需求进行设置。以下是一些常用的参数：

需要注意输入数据channel的设置，以[yolox_bytetrack_osd_encode](yolox_bytetrack_osd_encode/config/yolox_bytetrack_osd_encode_demo.json)为例

```json
  "channels": [
    {
      "channel_id": 2,
      "url": "../data/videos/mot17_01_frcnn.mp4",
      "source_type": "VIDEO",
      "loop_num": 1,
      "skip_element": [5005, 5006],
      "fps": 1,
      "sample_interval": 5,
      "sample_strategy": "KEEP"
    },
    {
      "channel_id": 3,
      "url": "../data/videos/mot17_03_frcnn.mp4",
      "source_type": "VIDEO",
      "loop_num": 1
    },
    {
      "channel_id": 20,
      "url": "../data/videos/mot17_06_frcnn.mp4",
      "source_type": "VIDEO",
      "loop_num": 1
    },
    {
      "channel_id": 30,
      "url": "../data/videos/mot17_08_frcnn.mp4",
      "source_type": "VIDEO",
      "loop_num": 1
    }
  ]
```

|      参数名    |    类型    | 默认值 | 说明 |
|:-------------:| :-------: | :------------------:| :------------------------:|
| channel_id | 整数   | 无 | 输入数据通道编号 |
|   url      | 字符串 | 无 | 输入数据路径，包括本地视频、图片、视频流和base64对应url后缀 |
|source_type | 字符串  | 无  | 输入数据类型，"RSTP"代表RTSP视频流，“RTMP”代表RTMP视频流，“GB28181”代表GB28181视频流，“VIDEO”代表本地视频，“IMG_DIR”代表图片文件夹， “BASE64”代表base64数据 |
|sample_interval | 整数  | 1  |抽帧数，如设置为5，表示每5帧有1帧会被后续处理，即为ObjectMata mFilter字段为false|
|loop_num | 整数  | 1  | 循环次数，仅适用于source_type为"VIDEO"和“IMG_DIR”，值为0时无限循环|
|fps | 浮点数  | 30 | 用于控制视频流的fps，fps=-1表示不控制fps；其它情况下，source_type为"IMG_DIR"或"BASE64"时由设置的值决定，其他source_type从视频流读取fps，设置的值不生效|
|base64_port | 整数  | 12348 | base64对应http端口 |
|skip_element| list | 无 | 设置该路数据是否跳过某些element，目前只对osd和encode生效。不设置时，认为不跳过任何element|
|sample_strategy|字符串|"DROP"|在有抽帧的情况下，设置被抽掉的帧是保留还是直接丢弃。"DROP"表示丢弃，"KEEP"表示保留|
|decode_id|整数|-1|单个decode element的情况不需要填写；多个decode element情况下，标识了某一路由对应id的decode element进行解码|


其中，channel_id为输入视频的通道编号，与[编码器](../element/multimedia/encode/README.md)输出channel_id相对应。例如，输入channel_id为20，使用编码器保存结果为本地视频时，文件名为20.avi。

一个图片文件夹表示一个视频，按frame_id命名，例如
```bash
IMG_DIR/
  ├── ****1.jpg
  ├── ****2.jpg
  ├── ****3.jpg
  ├── ****4.jpg
  ├── ****5.jpg
  ............
  └──******.jpg
```

> **注意**：
>1. 输入RTSP数据流的URL须以`rtsp://`开头
>2. 输入RTMP数据流的URL须以`rtmp://`开头
>3. 假设输入BASE64的URL为`/base64`，则http请求的格式需为「POST」(http://{host_ip}:{base64_port}/base64)，request body的data字段存储base64数据，如{"data": "{base64 string，不含头部(data:image/xxx;base64,)}"}
>4. 输入GB28181数据流的URL须以`gb28181://`开头

需要注意输入数据http_report/http_listen的设置，以[license_plate_recognition](license_plate_recognition/config/license_plate_recognition_demo.json)为例

```json
    "http_report": {
        "ip": "0.0.0.0",
        "port": 10001,
        "path": "/flask_test/"
    },
    "http_listen": {
        "ip": "0.0.0.0",
        "port": 8000,
        "path": "/task/test"
    },
```
|      参数名    |    类型    | 默认值 | 说明 |
|:-------------:| :-------: | :------------------:| :------------------------:|
| ip | 字符串   | http_listen默认为"0.0.0.0"，http_report默认无| 上报/监听的ip地址，report时上报请求到此ip，listen时监听此ip的post请求 |
| port | 整数 | http_listen默认为8000，http_report默认无 | 上报/监听的端口号，report时上报请求到此port，listen时监听此port的post请求 |
|path | 字符串  | http_listen默认为"/task/test"，http_report默认无  | 上报/监听的路由，report时上报请求到此ath，listen时监听此path的post请求 |

> **注意**：
>1. http_report字段必须完整，否则不会进行上报，默认不上报。