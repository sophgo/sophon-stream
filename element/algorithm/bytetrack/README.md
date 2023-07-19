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
        "min_box_area": 10,
        "frame_rate": 30,
        "track_buffer": 30
    },
    "shared_object": "../../../build/lib/libbytetrack.so",
    "device_id": 0,
    "id": 0,
    "name": "bytetrack",
    "side": "sophgo",
    "thread_number": 4
}
```

|      参数名    |    类型    | 默认值 | 说明 |
|:-------------:| :-------: | :------------------:| :------------------------:|
|  track_thresh  |   浮点数   | 0.5 | 目标跟踪检测阈值，与目标检测阈值相关联。如果目标检测阈值偏低，可以适当调低此参数 |
|  high_thresh   |   浮点数   | 0.6 | 在暂未匹配的检测目标中初始化新轨迹的阈值，不做重点调整 |
|  match_thresh  |   浮点数   | 0.7 | 目标跟踪匹配阈值，用于判断检测框的关联性。如果同一目标检测对象容易被匹配为不同目标跟踪的对象，可以适当调高此参数 |
|  frame_rate    |   浮点数   | 30  | 视频帧率，影响跟踪目标的最大消失时间，超过此时间的目标将被移除，计算方式为(max_time_lost = frame_rate / 30.0 * track_buffer) |
|  min_box_area  |   整数    |  10 | 过滤掉h*w小于min_box_area的跟踪框 |
|  track_buffer  |   整数    |  30 | 目标跟踪缓存，与最大消失时间关联 |
|  shared_object |   字符串   |  "../../../build/lib/libbytetrack.so"  | libbytetrack 动态库路径 |
|  device_id  |    整数       |  0 | tpu 设备号 |
|     id      |    整数       | 0  | element id |
|     name    |    字符串     | "bytetrack" | element 名称 |
|     side    |    字符串     | "sophgo"| 设备类型 |
| thread_number |    整数     | 无 | 启动线程数，需要保证和处理码流数一致 |

> **注意**：
需要保证插件线程数和处理码流数一致
