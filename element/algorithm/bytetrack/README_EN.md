# sophon-stream bytetrack element

English | [简体中文](README.md)

sophon-stream bytetrack element is a plugin in the sophon-stream framework. It is a simple, fast, and powerful multi-object tracker that does not rely on feature extraction models. This project provides a sample routine for this plugin; for details, refer to [ByteTrack Demo](../../../samples/bytetrack/README.md).

## 1. Features
* Decoupling of detection and tracking modules, adaptable to various detectors
* Support for multiple video streams
* Support for multi-threaded processing

## 2. Configuration Parameters
The sophon-stream bytetrack plugin has some configurable parameters that can be set according to your needs. Here are some commonly used parameters:

```json
{
    "configure": {
        "track_thresh": 0.5,
        "high_thresh": 0.6,
        "match_thresh": 0.7,
        "min_box_area": 10,
        "frame_rate": 30,
        "track_buffer": 30,
        "correct_box": true,
        "agnostic": true
    },
    "shared_object": "../../../build/lib/libbytetrack.so",
    "device_id": 0,
    "id": 0,
    "name": "bytetrack",
    "side": "sophgo",
    "thread_number": 4
}
```

| Parameter Name | Type | Default Value | Description |
|:-------------:| :-------: | :------------------:| :------------------------:|
| track_thresh | Float | 0.5 | Detection threshold for target tracking, associated with the target detection threshold. If the target detection threshold is too low, this parameter can be adjusted downwards. |
| high_thresh | Float | 0.6 | Threshold to initialize new trajectories in undetected targets without significant adjustments. |
| match_thresh | Float | 0.7 | Matching threshold for target tracking, used to determine the correlation of detection boxes. If the same target detection object is easily matched to different target tracking objects, this parameter can be adjusted upwards. |
| frame_rate | Float | 30 | Video frame rate, affecting the maximum disappearance time of tracked targets. Targets exceeding this time will be removed (calculation: max_time_lost = frame_rate / 30.0 * track_buffer). |
| min_box_area | Integer | 10 | Filter out tracking boxes with an area smaller than h*w. |
| track_buffer | Integer | 30 | Target tracking buffer, related to the maximum disappearance time. |
|  correct_box |   Bool  | true | Whether to use Kalman filtering to correct the tracking box, and use the original target detection box when the value is false |
|    agnostic  |   Bool  | true | Whether to perform uncategorized tracking? When the value is false, boxes of different categories will be offset by different offsets, and then calculate iou. The offset is the class id multiplied by 7000|
| shared_object | String | "../../../build/lib/libbytetrack.so" | Path to the *libbytetrack* dynamic library. |
| device_id | Integer | 0 | TPU device number. |
| id | Integer | 0 | Element ID. |
| name | String | "bytetrack" | Element name. |
| side | String | "sophgo" | Device type. |
| thread_number | Integer | None | Number of threads to start; ensure consistency with the number of processed streams. |

> **Note**:
Ensure that the number of plugin threads is consistent with the number of processed streams.