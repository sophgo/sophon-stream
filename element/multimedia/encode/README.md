1.编码插件,支持编码成rtsp/rtmp/本地视频文件
需要保证插件线程数和处理码流数一致

2.{
  "configure": {
    "rtsp_port": 8554
  },
  "shared_object": "../../../build/lib/libencode.so",
  "device_id": 0,
  "id": 0,
  "name": "encode",
  "side": "sophgo",
  "thread_number": 4
}
3.rtsp使用说明
4.rtmp使用说明