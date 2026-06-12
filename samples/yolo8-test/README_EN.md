# yolo8-test Demo

This demo validates that the new `yolo8-test` plugin can be loaded, registered, initialized in a graph, and executed by sophon-stream.

The POC reuses model and video assets from `samples/yolov8/data`, while loading the new plugin library:

```json
"shared_object": "../../build/lib/libyolo8-test.so",
"name": "yolo8-test_group"
```

Run after building:

```bash
./main --demo_config_path=../yolo8-test/config/yolo8-test_demo.json
```
