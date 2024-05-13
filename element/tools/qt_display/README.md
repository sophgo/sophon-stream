# sophon-stream qt_display element

[English](README_EN.md) | 简体中文

sophon-stream qt_display element是sophon-stream框架中的一个插件，是一个用于qt显示的工具。

## 1. 配置参数
sophon-stream qt_display插件具有一些可配置的参数，可以根据需求进行设置。以下是一些常用的参数：

```json
{
  "configure": {
      "width": 1920,
      "height": 1080,
      "rows": 2,
      "cols": 3
  },
  "shared_object": "../../build/lib/libqt_display.so",
  "name": "qt_display",
  "side": "sophgo",
  "thread_number": 4
}
```

| 参数名        | 类型   | 默认值                               | 说明                            |
| ------------- | ------ | ------------------------------------ | ------------------------------- |
| width         | int    | 1920                                  | 显示屏幕的宽            |
| height        | int    | 1080                                 | 显示屏幕的高            |
| rows          | int    | 2                                    | 每行显示路数的个数            |
| cols          | int    | 3                                    | 每列显示路数的个数            |
| shared_object | string | "../../../build/lib/libqt_display.so" | libqt_display动态库路径          |
| name          | string | "qt_display"                          | element名称                     |
| side          | string | "sophgo"                             | 设备类型                        |
| thread_number | int    | 4                                    | 启动线程数                      |

> **注意**
1. libqt_display element 使用时qt配置的显示路数(rows*cols)应该大于等于总路数
2. thread_number 应该与总路数一致