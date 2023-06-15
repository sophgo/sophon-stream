# sophon-stream bytetrack element

sophon-stream bytetrack element是sophon-stream框架中的一个插件，是一个简单、快速、强大的多目标跟踪器，且不依赖特征提取模型。本项目已提供此插件例程，详情请参见[ByteTrack Demo](../../../samples/bytetrack/README.md)

## 1. 特性
* 支持检测模块和跟踪模块解耦，可适配各种检测器
* 支持多路视频流
* 支持多线程处理

## 2. 配置参数
sophon-stream bytetrack插件具有一些可配置的参数，可以根据需求进行设置。以下是一些常用的参数：

```json
{
    "configure": {
        "track_thresh": 0.5,
        "high_thresh": 0.6,
        "match_thresh": 0.7,
        "frame_rate": 30,
        "track_buffer": 30
    },
    "shared_object": "../../../build/lib/libbytetrack.so",
    "device_id": 0,
    "id": 0,
    "name": "bytetrack",
    "side": "sophgo",
    "thread_number": 2
}
```

|      参数名    |    类型    | 默认值 | 说明 |
|:-------------:| :-------: | :------------------:| :------------------------:|
|  track_thresh  |   浮点数   | 0.5 | 目标跟踪检测阈值 |
|  high_thresh   |   浮点数   | 0.6 | 目标框重新匹配阈值 |
|  match_thresh  |   浮点数   | 0.7 | 目标跟踪匹配阈值 |
|  frame_rate    |   浮点数   | 30  | 帧率 |
|  track_buffer  |   整数    |  30 | 目标跟踪缓存 |
|  shared_object |   字符串   |  "../../../build/lib/libbytetrack.so"  | libbytetrack 动态库路径 |
|  device_id  |    整数       |  0 | tpu 设备号 |
|     id      |    整数       | 0  | element id |
|     name    |    字符串     | "bytetrack" | element 名称 |
|     side    |    字符串     | "sophgo"| 设备类型 |
| thread_number |    整数     | 2 | 启动线程数，需要保证和处理码流数一致 |

> **注意**：
需要保证插件线程数和处理码流数一致
