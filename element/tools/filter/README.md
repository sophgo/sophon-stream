# sophon-stream http_push element

[English](README_EN.md) | 简体中文

sophon-stream filter element是sophon-stream框架中的一个插件，是一个专用作数据筛选得工具。

## 1. 配置参数
sophon-stream http_push插件具有一些可配置的参数，可以根据需求进行设置。以下是一些常用的参数：

```json
{
    "configure": {
        "rules": [
            {
                "channel_id": 0,
                "filters": [
                    {
                        "alert_first_frames": 0,
                        "alert_frame_skip_nums": 6,
                        "areas": [
                            [
                                {
                                    "top": 0,
                                    "left": 0
                                },
                                {
                                    "top": 0,
                                    "left": 100000
                                },
                                {
                                    "top": 1000000,
                                    "left": 1000000
                                },
                                {
                                    "top": 100000,
                                    "left": 0
                                }
                            ]
                        ],
                        "classes": [
                            0
                        ],
                        "times": [
                            {
                                "time_start": "00 00 00",
                                "time_end": "23 59 59"
                            }
                        ],
                        "type": 0
                    }
                ]
            }
        ]
    },
    "shared_object": "../../build/lib/libfilter.so",
    "name": "filter",
    "side": "sophgo",
    "thread_number": 1
}
```

| 参数名        | 类型   | 默认值                               | 说明                            |
| ------------- | ------ | ------------------------------------ | ------------------------------- |
| channel_id            | int | 0                           | 顺序和id与demo的json一致          |
| alert_first_frames            | int | 0                       | 每一路追踪到第几帧开始上报               |
| alert_frame_skip_nums           | int | 1                    | 从第一次上报开始每几帧上报一次          |
| top           | int | 1                    | 多边形顶点的x坐标,需要满足循序，个数为0代表不检测    |
| left           | int | 1                    | 多边形顶点的y坐标,需要满足循序，个数为0代表不检测    |
| classes           | list[int] | []                    | 筛选的类别，为空将会全部筛掉        |
| time_start           | string | 无                    | 开始时间，格式hh mm ss，为空将会全部筛掉  |
| time_end           | int | 无                   | 结束时间，格式hh mm ss,为空将会全部筛掉    |
| type           | int | 0                    | 筛选类型，recognize:0  track:1 classes:other        |
| shared_object | string | "../../../build/lib/libhttp_push.so" | libhttp_push动态库路径          |
| name          | string | "http_push"                          | element名称                     |
| side          | string | "sophgo"                             | 设备类型                        |
| thread_number | int    | 1                                    | 启动线程数                      |


