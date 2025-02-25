# sophon-stream encode element

English | [简体中文](README.md)

sophon-stream encode element is a plugin within the sophon-stream framework used to encode processed image information into various video formats.

## Content
- [sophon-stream encode element](#sophon-stream-encode-element)
  - [Content](#content)
  - [1. feature](#1-feature)
  - [2. Configuration parameters](#2-configuration-parameters)
  - [3. RTSP Usage Instructions](#3-rtsp-usage-instructions)
  - [4. RTMP Usage Instructions](#4-rtmp-usage-instructions)
  - [5. Output Local Video File](#5-output-local-video-file)
  - [6. Output local image folder](#6-output-local-image-folder)
  - [7. WebSocket Usage Instructions](#7-websocket-usage-instructions)
  - [8. Streaming Server](#8-streaming-server)

## 1. feature
* Supports various output formats such as RTSP, RTMP, local video files, local image folders, etc.
* Supports multiple video encoding formats like H.264, H.265, etc.
* Supports various pixel formats such as I420, NV12, etc.
* Enables high-performance encoding for multiple video streams with hardware acceleration.
* Provides flexible configuration options like encoder parameters, video stream ports, thread count, etc.
* Seamlessly integrates with other sophon-stream plugins to provide encoded data streams.


## 2. Configuration parameters
sophon-stream encoder plugin has several configurable parameters that can be adjusted according to specific requirements. Here are some commonly used parameters:

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
    "wss_backend": "WEBSOCKETPP",
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

| Parameter Name|  name  |        Default value             |                        Description                       |
| :-----------: | :----: | :-------------------------------: | :-----------------------------------------------------: |
|  encode_type  | string |                \                 | output format，include "RTSP","RTMP","VIDEO","IMG_DIR","WS" |
|   rtsp_port   | string |                \                 |                        rtsp port                        |
|   rtmp_port   | string |                \                 |                        rtmp port                        |
|   wss_port    | string |                \                 |                WebSocket server starting port           |
|    enc_fmt    | string |                \                 |       encode format，include "h264_bm"，"h265_bm"       |
|    pix_fmt    | string |                \                 |             pixel format，include "I420"，"NV12"        |
|  ws_enc_type  | string |           "IMG_ONLY"             |Take effect when the encoding format is WS. Setting to "IMG_ONLY" means only encoding pictures. Setting to "SERIALIZED" means encoding ObjectMetadata.|
| wss_backend   | string |          "WEBSOCKETPP"            | websocket server type, supports "WEBSOCKETPP" and "BOOST"      |
|      fps      |  int  |                25                 |                  RTSP,RTMP,VIDEO frame rate             |
|      ip       | string |             "localhost"           |                       ip of stream server              |
|      prefix   | string |                ""                 |          the prefix of output_path's last name                      |
|     width     | int    |               -1                 |           width of encoder output, default to img.width  |
|     height     | int    |               -1                 |           width of encoder output, default to img.height  |
| shared_object | string | "../../../build/lib/libencode.so" |                  libencode dynamic library path        |
|   device_id   |  int  |                 0                 |                       tpu device id                     |
|      id       |  int  |                 0                 |                       element id                        |
|     name      | string |             "encode"              |                      element name                      |
|     side      | string |             "sophgo"              |                      device type                       |
| thread_number |  int  |                 1           |  Thread number, it should be consistent with the number of streams being processed. |

> **notes**：
1. It is necessary to ensure that the number of plugin threads matches the number of processed streams.
2. When encode_type is set to RTSP, ensure that rtsp_port is not empty. For encode_type as RTMP, ensure that rtmp_port is not empty. For encode_type as WS, ensure that wss_port is not empty.
3. For encode_type set as VIDEO and IMG_DIR, the file saving path is "./results".


## 3. RTSP Usage Instructions
It is required to locally start the streaming server. For specific usage, refer to [8.Streaming Server](#8-Streaming-Server).
Make the following settings in the `encode.json` file
```json
"encode_type": "RTSP",
"rtsp_port": "8554"
```

The format of the output video stream URL is: `rtsp://localhost:{rtsp_port}/{graph_id}_{channel_id}`

If the rtsp_port is 8554, the channel_id and graph_id is 0, the URL is:`rtsp://localhost:8554/0_0`

## 4. RTMP Usage Instructions
It is required to locally start the streaming server. For specific usage, refer to [8.Streaming Server](#8-Streaming-Server).
Make the following settings in the `encode.json` file
```json
"encode_type": "RTMP",
"rtmp_port": "1935"
```

The format of the output video stream URL is: `rtmp://localhost:{rtmp_port}/{channel_id}`

If the rtsp_port is 1935 and the channel_id is 0, the URL is:`rtmp://localhost:1935/0`

## 5. Output Local Video File
Make the following settings in the `encode.json` file
```json
"encode_type": "VIDEO",
```

Output Video File Name is:`{channel_id}.avi`

If the channel_id is 0, then the file name would be`0.avi`

## 6. Output local image folder
Make the following settings in the `encode.json` file
```json
"encode_type": "IMG_DIR",
```

The output image file name would be：`./results/{channel_id}/{mFrameId}.jpg`

If the channel_id is 0 and mFrameId is 0, the file name is`0_0.jpg`

## 7. WebSocket Usage Instructions
Make the following settings in the `encode.json` file
```json
"encode_type": "WS",
"wss_port": "9000"
```

Output websocket URL format is: `ws://{host_ip}:{wss_port+channel_id}`

When `host_ip` is 127.0.0.1, `wss_port` is 9000 and `channel_id` is 2, the URL should be`ws://127.0.0.1:9002`.

## 8. Streaming Server
`mediamtx` as a streaming server can be started using the following steps:

First, download the corresponding package from the [official website](https://github.com/bluenviron/mediamtx/releases) and then extract it.

For deployment on edge devices with sophon-stream, we recommend using the arm64v8 version of the streaming server. 

```bash
wget https://github.com/bluenviron/mediamtx/releases/download/v1.2.0/mediamtx_v1.2.0_linux_arm64v8.tar.gz
```

After extracting, open the `mediamtx.yml` configuration file and modify the readTimeout and writeTimeout parameters. Save the changes and exit.
```yml
# timeout of read operations.
readTimeout: 120s
# timeout of write operations.
writeTimeout: 120s
```

Then run `mediamtx`
```bash
./mediamtx
```

Streaming Server would be started successfully
```bash
INF MediaMTX v0.23.7
INF [RTSP] listener opened on :8554 (TCP), :8000 (UDP/RTP), :8001 (UDP/RTCP)
INF [RTMP] listener opened on :1935
INF [HLS] listener opened on :8888
INF [WebRTC] listener opened on :8889 (HTTP)
```


The default TCP port for RTSP in `mediamtx.yml` is 8554, and the default port for RTMP is 1935. If you modify the port numbers, ensure that the corresponding port configurations in the plugin settings are also updated to match.

Please note that mediamtx is an example server and may not offer extensive scalability or comprehensive functionality. If you require a stable and feature-rich RTSP streaming media server, consider using mature open-source or commercial solutions such as Live555, GStreamer, FFmpeg, etc. These tools provide broader and more comprehensive support for RTSP functionalities.

If you are using this streaming server for broadcasting, you can refer to the following commands:
```bash
ffmpeg -stream_loop -1 -an -re -i <your video> -codec copy -f rtsp -rtsp_transport tcp rtsp://localhost:8554/1
```

> **notes**:
ws depends on the `boost` library. If you encounter an error during the `make` phase at '#include <boost/version.hpp>', please use the following command to install this library
```bash
sudo apt-get update 
sudo apt-get install libboost-all-dev
```
