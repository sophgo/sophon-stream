# sophon-stream converger element

[English](README_EN.md) | 简体中文

sophon-stream converger element是sophon-stream框架中的一个插件，是一个专用作数据汇聚功能的工具。

## 1. 特性
* 必须与distributor element配合使用
* 保证输出的ObjectMetadata具有正确的时间顺序

## 2. 配置参数
sophon-stream converger插件具有一些可配置的参数，可以根据需求进行设置。以下是一些常用的参数：

```json
{
    "configure": {
        "default_port": 0
    },
    "shared_object": "../../../build/lib/libconverger.so",
    "name": "converger",
    "side": "sophgo",
    "thread_number": 1
}
```

| 参数名        | 类型   | 默认值                               | 说明                            |
| ------------- | ------ | ------------------------------------ | ------------------------------- |
| default_port  | int    | 无                                   | 从数据分发element接收数据的端口 |
| shared_object | string | "../../../build/lib/libconverger.so" | libconverger动态库路径          |
| name          | string | "converger"                          | element名称                     |
| side          | string | "sophgo"                             | 设备类型                        |
| thread_number | int    | 1                                    | 启动线程数                      |

> **注意**
1. converger element从`default_port`接收到ObjectMetadata之后，会等待其所有的分支都更新完成，才会向后续element发送。
2. 发送前，将所有数据依序保存；发送时，将所有已经完成更新的数据依序发送。
3. converger element必须搭配distributor element使用。