# sophon-stream yolo8-test element

`yolo8-test` 是用于验证 sophon-stream 新增算法插件流程的 POC 插件。它复用 `yolov8` 插件的实现逻辑，仅将插件目录、C++ 类名、namespace、动态库目标和运行时注册名独立出来。

## 插件命名

| 项目 | 名称 |
|---|---|
| 插件目录 | `element/algorithm/yolo8-test` |
| C++ namespace | `sophon_stream::element::yolo8_test` |
| C++ 主类 | `Yolo8Test` |
| 普通 Element 注册名 | `yolo8-test` |
| Group Element 注册名 | `yolo8-test_group` |
| 编译产物 | `build/lib/libyolo8-test.so` |

## 配置示例

```json
{
  "configure": {
    "model_path": "../yolov8/data/models/BM1684X/yolov8s_int8_1b.bmodel",
    "threshold_conf": 0.5,
    "threshold_nms": 0.5,
    "bgr2rgb": true,
    "mean": [0, 0, 0],
    "std": [255, 255, 255]
  },
  "shared_object": "../../build/lib/libyolo8-test.so",
  "name": "yolo8-test_group",
  "side": "sophgo",
  "thread_number": 4
}
```

该 POC 默认复用 `samples/yolov8/data` 下的模型、视频和类别文件，避免重复拷贝大文件。
