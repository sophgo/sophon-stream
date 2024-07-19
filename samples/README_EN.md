# sophon-stream samples instruction

English | [简体中文](README.md)

## 1. Introduction

This directory contains reference demo provided by sophon-stream.

The directory structure is as follows:

```bash
./samples/
├── bytetrack                                       # detection and tracking demo
├── CMakeLists.txt                                  # cmake file
├── include                                         # header files for plotting functions, etc
├── license_plate_recognition                       # vehicle detection and license plate recognition demo
├── openpose                                        # pose estimation demo
├── README.md                                       # user manual
├── README_EN.md                                    # user manual in English
├── resnet                                          # classification demo
├── retinaface                                      # face detection demo
├── retinaface_distributor_resnet_faiss_converger   # face detection and recognition demo
├── src                                             # the only main function
├── yolov5                                          # yolov5 detection demo
├── yolov5_bytetrack_distributor_resnet_converger   # detection, tracking and recognition demo
├── yolox                                           # yolox detection demo
└── yolox_bytetrack_osd_encode                      # detection, tracking, drawing and streaming demo
```

This directory contains multiple routines such as face detection, license plate recognition, and more. Each routine's directory includes configuration files and scripts for downloading models, videos, and other data. For all these routines, they share the same main function, namely `sophon-stream/samples/src/main.cc`. This main function primarily functions to retrieve configuration files based on preset paths, parse input data for the routine, and invoke a unified lower-level interface to configure all elements. It initiates the entire pipeline.

For example，you can run `yolov5_bytetrack_distributor_resnet_converger` routine as follow :

```bash
./main --demo_config_path=../yolov5_bytetrack_distributor_resnet_converger/config/yolov5_bytetrack_distributor_resnet_converger_demo.json
```

Then, the main function navigates to the directory of the `yolov5_bytetrack_distributor_resnet_converger` routine to search for configuration files. It constructs a pipeline based on the configurations specified in these files. Consequently, the running pipeline possesses the functionality of 'detection + tracking + recognition'.

For others，you can run `license_plate_recognition` routine as follow：

```bash
./main --demo_config_path=../license_plate_recognition/config/license_plate_recognition_demo.json
```

Then, the main function will then navigate to the directory of the `license_plate_recognition` routine to search for configuration files. Following the specifications within these configuration files, it constructs a pipeline that incorporates 'vehicle detection + license plate recognition' functionalities.

The benefit of this design is the segregation between users and the underlying code, allowing users to focus solely on configuring JSON files without needing to consider the operational logic of the stream framework.

> Notes
* For detailed information about each routine, please refer to the `README_EN.md` file located within each routine's directory.

## 2. Configuration
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
|decode_id|int|-1|In the case of a single decode element, it is not necessary to fill in the form; in the case of multiple decode elements, it identifies that a certain way is decoded by the decode element with the corresponding id.|
|roi| dict| \ | When roi is set, the frame from decoder will be cropped according to the roi range, otherwise passing the original frame.| 
|graph_id| int | 0 | Graph Id that the current stream belongs |


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

Need attend to the http_report/http_listen setting of input data.For example,[license_plate_recognition](license_plate_recognition/config/license_plate_recognition_demo.json)

```json
    "http_report": {
        "ip": "0.0.0.0",
        "port": 10001,
        "path": "/flask_test/"
    },
    "http_listen": {
        "ip": "0.0.0.0",
        "port": 8000,
        "path": "/task/test"
    },
```

 |Parameter Name | Type | Default Value | Description|
 |:-------------:| :-------------: | :------------------:| :---------------------------------------------------:|
|IP | String | HTTP_ The default listen is "0.0.0.0", with HTTP_ Report defaults to no | IP address for reporting/listening. When reporting, report requests to this IP address, and when listening, listen for post requests from this IP address |
|Port | integer | http_ The default listen is 8000, HTTP_ Report defaults to no | port number for reporting/listening. When reporting, report requests to this port, and when listening, listen for post requests from this port |
|Path | string | http_listen defaults to "/task/test", http_report defaults to no | route for reporting/listening. When reporting, report requests to this path, and when listening, listen for post requests to this path |


> **注意**：
>1.  The http_report field must be complete, otherwise it will not be reported and will not be reported by default.