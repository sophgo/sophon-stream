# sophon-stream encode element

[English](README_EN.md) | 简体中文

sophon-stream encode element是sophon-stream框架中的一个插件，用于将处理后的图像信息编码为各类视频格式。

## 目录
- [sophon-stream encode element](#sophon-stream-encode-element)
  - [目录](#目录)
  - [1. 特性](#1-特性)
  - [2. 配置参数](#2-配置参数)
  - [3. rtsp使用说明](#3-rtsp使用说明)
  - [4. rtmp使用说明](#4-rtmp使用说明)
  - [5. 输出本地视频文件](#5-输出本地视频文件)
  - [6. 输出本地图片文件夹](#6-输出本地图片文件夹)
  - [7. WebSocket使用说明](#7-websocket使用说明)
  - [8. 推流服务器](#8-推流服务器)

## 1. 特性
* 支持多种输出格式，如RTSP、RTMP、本地视频文件、本地图片文件夹等。
* 支持多种视频编码格式，如H.264、H.265等。
* 支持多种像素格式，如I420、NV12等。
* 支持多路视频流高性能编码，支持硬件加速。
* 提供灵活的配置选项，如编码器参数、视频流端口、线程数等。
* 可以与其他sophon-stream插件无缝集成，提供编码后的数据流。

## 2. 配置参数
sophon-stream编码器插件具有一些可配置的参数，可以根据需求进行设置。以下是一些常用的参数：

```json
{
  "configure": {
    "encode_type": "RTSP",
    "rtsp_port": "8554",
    "rtmp_port": "1935",
    "wss_port": "9000",
    "enc_fmt": "h264_bm",
    "pix_fmt": "I420",
    "ws_enc_type": "IMG_ONLY",
    "fps": 25
  },
  "shared_object": "../../../build/lib/libencode.so",
  "device_id": 0,
  "id": 0,
  "name": "encode",
  "side": "sophgo",
  "thread_number": 4
}
```

|    参数名     |  类型  |              默认值               |                          说明                           |
| :-----------: | :----: | :-------------------------------: | :-----------------------------------------------------: |
|  encode_type  | 字符串 |                无                 | 编码格式，包括 “RTSP”、“RTMP”、“VIDEO”、“IMG_DIR”、"WS" |
|   rtsp_port   | 字符串 |                无                 |                        rtsp 端口                        |
|   rtmp_port   | 字符串 |                无                 |                        rtmp 端口                        |
|   wss_port    | 字符串 |                无                 |                websocket server起始端口                 |
|    enc_fmt    | 字符串 |                无                 |           编码格式，包括 "h264_bm"，“h265_bm”           |
|    pix_fmt    | 字符串 |                无                 |              像素格式，包括 "I420"，"NV12"              |
|  ws_enc_type  | 字符串 |           "IMG_ONLY"              | 当编码格式为WS时生效，设为"IMG_ONLY"时只对图片编码，设为"SERIALIZED"对ObjectMetadata作编码 |
|      fps      |  整数  |                25                 |                  RTSP、RTMP、VIDEO帧率                  |
|      ip       | 字符串 |             "localhost"           |                       流服务器地址                      |
|     width     | 整数   |                -1                 |         编码器输出的宽度，默认和输入图片相同              |
|     height     | 整数   |                -1                 |         编码器输出的高度，默认和输入图片相同              |
| shared_object | 字符串 | "../../../build/lib/libencode.so" |                  libencode 动态库路径                   |
|   device_id   |  整数  |                 0                 |                       tpu 设备号                        |
|      id       |  整数  |                 0                 |                       element id                        |
|     name      | 字符串 |             "encode"              |                      element 名称                       |
|     side      | 字符串 |             "sophgo"              |                        设备类型                         |
| thread_number |  整数  |                 1                 |          启动线程数，需要保证和处理码流数一致           |

> **注意**：
1. 需要保证插件线程数和处理码流数一致
2. encode_type为RTSP时，需保证rtsp_port不为空，encode_type为RTMP时，需保证rtmp_port不为空，encode_type为WS时，需保证wss_port不为空。
3. encode_type为VIDEO和IMG_DIR时，文件保存路径为`./results`

## 3. rtsp使用说明
需要本地启动推流服务器，具体用法见[6. 推流服务器](#8-推流服务器)
在`encode.json`中做出以下设置
```json
"encode_type": "RTSP",
"rtsp_port": "8554"
```

输出视频流URL的格式为：`rtsp://localhost:{rtsp_port}/{channel_id}`

假设rtsp_port为8554，channel_id为0, 此时URL为`rtsp://localhost:8554/0`

## 4. rtmp使用说明
需要本地启动推流服务器，具体用法见[8. 推流服务器](#8-推流服务器)

在`encode.json`中做出以下设置
```json
"encode_type": "RTMP",
"rtmp_port": "1935"
```

输出视频流URL格式为：`rtmp://localhost:{rtmp_port}/{channel_id}`

假设rtmp_port 为1935，channel_id为0, 此时URL为`rtmp://localhost:1935/0`

## 5. 输出本地视频文件
在`encode.json`中做出以下设置
```json
"encode_type": "VIDEO",
```

输出视频文件名为：`{channel_id}.avi`

假设channel_id为0, 此时文件名为`0.avi`

## 6. 输出本地图片文件夹
在`encode.json`中做出以下设置
```json
"encode_type": "IMG_DIR",
```

输出图片文件名为：`./results/{channel_id}/{mFrameId}.jpg`

假设channel_id为0, mFrameId为0，此时文件名为`./results/0/0.jpg`

## 7. WebSocket使用说明

在`encode.json`中做出以下设置
```json
"encode_type": "WS",
"wss_port": "9000"
```

输出websocket URL格式为：`ws://{host_ip}:{wss_port+channel_id}`

host_ip为127.0.0.1, wss_port为9000，channel_id为2，此时URL为`ws://127.0.0.1:9002`

## 8. 推流服务器
可以使用`mediamtx`作为推流服务器，启动步骤如下

首先去[官网](https://github.com/bluenviron/mediamtx/releases)下载对应的软件包然后解压。

对于在边缘设备上部署sophon-stream的场合，我们推荐使用arm64v8版本流服务器。即：

```bash
wget https://github.com/bluenviron/mediamtx/releases/download/v1.2.0/mediamtx_v1.2.0_linux_arm64v8.tar.gz
```

解压缩后打开`mediamtx.yml`配置文件，修改readTimeout与writeTimeout这两个参数，保存后退出
```yml
# timeout of read operations.
readTimeout: 120s
# timeout of write operations.
writeTimeout: 120s
```

然后启动mediamtx
```bash
./mediamtx
```

此时服务器启动成功
```bash
INF MediaMTX v0.23.7
INF [RTSP] listener opened on :8554 (TCP), :8000 (UDP/RTP), :8001 (UDP/RTCP)
INF [RTMP] listener opened on :1935
INF [HLS] listener opened on :8888
INF [WebRTC] listener opened on :8889 (HTTP)
```

mediamtx.yml中rtsp的默认TCP端口是8554，rtmp默认端口是1935，如果修改端口号，插件配置中相应端口配置也要修改成一致。

需要注意的是，mediamtx是一个示例服务器，并不具备高度的可扩展性和功能完整性。如果您需要构建一个稳定和功能丰富的实际RTSP流媒体服务器，可以选择使用成熟的开源或商业解决方案，如Live555、GStreamer、FFmpeg等，这些工具提供了更广泛和全面的RTSP功能支持。

如果您使用该流服务器进行推流，可以参考如下命令：
```bash
ffmpeg -stream_loop -1 -an -re -i <your video> -codec copy -f rtsp -rtsp_transport tcp rtsp://localhost:8554/1
```

> **注意**:
ws依赖boost库，如果make阶段在#include<boost/version.hpp>部分报错，请使用如下命令安装该库：
```bash
sudo apt-get update 
sudo apt-get install libboost-all-dev
```
