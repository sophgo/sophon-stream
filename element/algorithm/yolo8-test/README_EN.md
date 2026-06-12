# sophon-stream yolo8-test element

`yolo8-test` is a POC plugin used to validate how to add a new algorithm element to sophon-stream. It reuses the YOLOv8 implementation and gives it independent plugin names, C++ identifiers, CMake target, and runtime registration names.

| Item | Name |
|---|---|
| Plugin directory | `element/algorithm/yolo8-test` |
| C++ namespace | `sophon_stream::element::yolo8_test` |
| C++ class | `Yolo8Test` |
| Element name | `yolo8-test` |
| Group element name | `yolo8-test_group` |
| Build output | `build/lib/libyolo8-test.so` |

The sample configuration reuses models, videos, and class names from `samples/yolov8/data`.
