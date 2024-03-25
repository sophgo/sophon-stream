# sophon-stream decode element

English | [简体中文](README.md)

The sophon-stream decode element is a plugin within the sophon-stream framework, used for decoding images, videos, RTSP/RTMP video streams for subsequent analysis and processing purposes.

## 1. feature
* Supports various input formats, including RTSP, RTMP, local videos, image files, BASE64, etc.
* Supports reconnection for interrupted RTSP/RTMP video streams.
* Allows configuration for looping local videos and image files.
* High-performance decoding for multiple video streams with hardware acceleration.
* Offers flexible configuration options such as decoder parameters, device types, thread counts, etc.
* Seamlessly integrates with other sophon-stream plugins, providing decoded data streams after processing.

## 2. Configuration parameters
sophon-stream decoder plugin has several configurable parameters that can be adjusted according to specific requirements. Here are some commonly used parameters:
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

|      Parameter Name    |    Type    | Default Value | Description |
|:-------------:| :-------: | :------------------:| :------------------------:|
|  shared_object |   string   |  "../../../build/lib/libdecode.so" | libdecode dynamic library path |
|  device_id  |    int       |  0 | tpu device id |
|     id      |    int       | 0  | element id |
|     name    |    string     | "decode" | element name |
|     side    |    string     | "sophgo"| device type |
| thread_number |    int     | 1| thread number |



Additionally, attention should be paid to the setting of the input data channels in the decode module. 
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
      "sample_strategy": "KEEP",
      "roi":{
        "left": 0,
        "top": 0,
        "width": 800,
        "height": 600
      }
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

|      Parameter Name    |    Type    | Default Value | Description |
|:-------------:| :-------: | :------------------:| :------------------------:|
| channel_id | int   | \ | Input data channel number |
|   url      | string | \ | Input data path, including local videos, images, video streams, and base64-encoded URLs. |
|source_type | string  | \  | Input data types: "RTSP" represents an RTSP video stream, “RTMP” represents an RTMP video stream, “VIDEO” represents local videos, “IMG_DIR” represents image folders, and “BASE64” represents base64-encoded data. |
|sample_interval | int  | 1  |Frame extraction rate. Setting it to 5 implies that for every 5 frames, 1 frame will be processed subsequently, which means the ObjectMata mFilter field is set to false.|
|loop_num | int  | 1  | Loop count. Only applicable when the source_type is set to "VIDEO" and "IMG_DIR". A value of 0 indicates an infinite loop.|
|fps | float  | 30 | Used to control the frames per second (fps) of the video stream. Fps=-1 means no control over fps. In other cases, when source_type is set to "IMG_DIR" or "BASE64", it's determined by the set value. For other source_types, fps is read from the video stream, and the set value does not take effect.|
|base64_port | int  | 12348 | Base64 corresponds to the HTTP port |
|skip_element| list | \ | Set whether to skip certain elements for this data stream. Currently, this only applies to OSD and Encode. When not specified, it's assumed that no elements are to be skipped.|
|sample_strategy|string|"DROP"|When frames are being filtered, set whether the filtered frames are to be kept or discarded. "DROP" indicates discarding the frames, while "KEEP" indicates retaining them.|
|roi| dict| \ | When roi is set, the frame from decoder will be cropped according to the roi range, otherwise passing the original frame.| 


Where `channel_id` stands for the channel number of the input video, corresponding to the `channel_id` output by the [encoder](../encode/README.md). For instance, if the input `channel_id` is 20 and the encoder is used to save the results as a local video, the file name will be `20.avi`.

A picture folder can represent a video, named according to the frame_id, for example:
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

> **notes**：
>1. The URL for inputting RTSP data stream must begin with `rtsp://`.
>2. The URL for inputting RTMP data stream must begin with `rtmp://`.
>3. If the input BASE64 URL is `/base64`, the HTTP request format should be a POST request to "http://{host_ip}:{base64_port}/base64". The request body's data field stores the base64 data, such as {"data": "{base64 string, excluding the header (data:image/xxx;base64,)}"}.
>4. The URL for inputting GB28181 data stream must start with `gb28181://`.

