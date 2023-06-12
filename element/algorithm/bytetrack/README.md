1.bytetrack目标跟踪算法插件
需要保证插件线程数和处理码流数相等

2.配置
{
    "configure": {
        "track_thresh": 0.6,
        "high_thresh": 0.7,
        "match_thresh": 0.8,
        "frame_rate": 30,
        "track_buffer": 30
    },
    "shared_object": "../../../element/algorithm/bytetrack/build/libbytetrack.so",
    "device_id": 0,
    "id": 0,
    "name": "bytetrack",
    "side": "sophgo",
    "thread_number": 2
}