1.解码插件，支持本地视频/本地图片文件夹/rtsp/rtmp
"本地视频/本地图片文件夹"支持配置循环
"rtsp/rtmp"支持断开重连

2.配置
{
  "configure": {},
  "shared_object": "../../../element/multimedia/decode/build/libdecode.so",
  "device_id": 0,
  "id": 0,
  "name": "decode",
  "side": "sophgo",
  "thread_number": 1
}