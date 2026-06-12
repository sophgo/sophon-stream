# yolo8-test Demo

该 demo 用于验证 `yolo8-test` 新插件是否能在 sophon-stream 中完成动态库加载、ElementFactory 注册、Graph 初始化和流水线运行。

## 目录

```text
samples/yolo8-test/
├── config/
│   ├── decode.json
│   ├── engine_group.json
│   ├── yolo8-test_demo.json
│   ├── yolo8-test_group.json
│   └── yolo8-test_classthresh_roi_example.json
└── data/
    └── coco.names
```

## 复用关系

该 POC 只验证新增插件流程，因此配置复用原 `samples/yolov8/data` 中的模型和视频：

```json
"model_path": "../yolov8/data/models/BM1684X/yolov8s_int8_1b.bmodel"
```

```json
"url": "../yolov8/data/videos/test_car_person_1080P.avi"
```

运行时使用新的插件动态库：

```json
"shared_object": "../../build/lib/libyolo8-test.so",
"name": "yolo8-test_group"
```

## 运行

编译后在 `samples/build` 下执行：

```bash
./main --demo_config_path=../yolo8-test/config/yolo8-test_demo.json
```

如果开启 `download_image=true`，请确保 `class_names` 指向的类别文件存在。
