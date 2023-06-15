# sophon-stream decode element

sophon-stream decode element是sophon-stream框架中的一个插件，用于图片、视频、RTSP/RTMP视频流解码，以供后续的分析和处理使用。

## 1. 特性
* 支持多种输入格式，如RTSP、RTMP、本地视频与图片文件等。
* 支持RTSP/RTMP视频流断开重连。
* 支持本地视频与图片文件配置循环。
* 支持多路视频流高性能解码，支持硬件加速。
* 提供灵活的配置选项，如解码器参数、设备类型、线程数等。
* 可以与其他sophon-stream插件无缝集成，提供解码后的数据流。

## 2. 配置参数
sophon-stream解码器插件具有一些可配置的参数，可以根据需求进行设置。以下是一些常用的参数：
```json
{
  "configure": {},
  "shared_object": "../../../build/lib/libdecode.so",
  "device_id": 0,
  "id": 0,
  "name": "decode",
  "side": "sophgo",
  "thread_number": 1
}
```

|      参数名    |    类型    | 默认值 | 说明 |
|:-------------:| :-------: | :------------------:| :------------------------:|
|  shared_object |   字符串   |  "../../../build/lib/libdecode.so" | libdecode 动态库路径 |
|  device_id  |    整数       |  0 | tpu 设备号 |
|     id      |    整数       | 0  | element id |
|     name    |    字符串     | "decode" | element 名称 |
|     side    |    字符串     | "sophgo"| 设备类型 |
| thread_number |    整数     | 1| 启动线程数 |


此外，还需要注意decode中输入数据channel的设置，以[yolox_bytetrack_osd_encode](../../../samples/yolox_bytetrack_osd_encode/config/yolox_bytetrack_osd_encode_demo.json)为例

```json
  "channels": [
    {
      "channel_id": 2,
      "url": "../data/videos/mot17_01_frcnn.mp4",
      "source_type": "VIDEO",
      "loop_num": 1
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
|   url      | 字符串 | 无 | 输入数据路径，包括本地视频、图片和视频流 |
|source_type | 字符串  | 无  | 输入数据类型，"RSTP"代表RTSP视频流，“RTMP”代表RTMP视频流，“VIDEO”代表本地视频，“IMG_DIR”代表图片文件夹 |
|loop_num | 整数  | 1  | 循环次数，仅适用于source_type为"VIDEO"和“IMG_DIR”，值为0时无限循环|


其中，channel_id为输入视频的通道编号，与[编码器](../encode/README.md)输出channel_id相对应。例如，输入channel_id为20，使用编码器保存结果为本地视频时，文件名为20.avi。

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
1. 输入RTSP数据流的URL须以`rtsp://`开头
2. 输入RTMP数据流的URL须以`rtmp://`开头
