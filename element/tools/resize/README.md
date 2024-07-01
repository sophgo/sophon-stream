# sophon-stream resize element


sophon-stream resize element是sophon-stream框架中的一个插件，是一个用于尺寸变换的插件。

## 1. 特性
目前插件可以支持按比例缩放，若ratio为0或1则采用宽高进行缩放，否则直接采用比例因子进行缩放。

## 2. 配置参数
sophon-stream resize插件具有一些可配置的参数，可以根据需求进行设置。以下是一些常用的参数：

```json
{
    "configure": {
      "ratio":0.4,
      "dst_h":1080,
      "dst_w":1920
    },
    "shared_object": "../../build/lib/libresize.so",
    "name": "resize",
    "side": "sophgo",
    "thread_number": 2
  }

```

| 参数名        | 类型   | 默认值                         | 说明             |
| ------------- | ------ | ------------------------------| ---------------- |
| dst_h         | int    | 无                            | 输出图像的高度信息 |
| dst_w         | int    | 无                            | 输出图像的宽度信息 |
| crop_top      | int    | 无                            | 对输入图像进行裁剪操作时，从哪一行开始进行裁剪的位置信息 |
| crop_left     | int    | 无                            | 对输入图像进行裁剪操作时，从哪一列开始进行裁剪的位置信息 |
| crop_h        | int    | 无                            | 对输入图像进行裁剪操作时，裁剪出图像的高度信息 |
| crop_w        | int    | 无                            | 对输入图像进行裁剪操作时，裁剪出图像的宽度信息 |
| shared_object | string | "../../../build/lib/libresize.so" | libresize动态库路径 |
| name          | string | "resize"                       | element名称      |
| side          | string | "sophgo"                       | 设备类型         |
| thread_number | int    | 1                              | 启动线程数       |

