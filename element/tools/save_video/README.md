# sophon-stream save_video element

[English](README_EN.md) | 简体中文

sophon-stream `save_video` element 是一个功能强大的插件，专门用于实现事件触发的视频录制和报警上报。当检测到指定的目标时，它能够自动录制一段视频、截取快照，并通过HTTP POST请求将包含视频和图片访问URL的报警信息发送到指定的服务器。

## 1. 功能特性

- **事件触发录制**：仅在检测到感兴趣的目标时启动录制，有效节省存储空间。
- **灵活的触发条件**：可以配置特定类别（`trigger_classes`）和连续帧数（`min_trigger_frames`）来精确控制录制触发，减少误报。
- **自动报警上报**：录制结束后，自动将事件信息（包括视频和快照的URL）通过HTTP POST请求发送到配置的服务器。
- **存储空间管理**：支持按天数（`retention_days`）和总容量（`retention_max_gb`）两种策略自动清理旧的录像文件，防止硬盘占满。
- **高度可定制**：从服务器地址、存储路径到报警内容均可通过JSON配置进行定制。

## 2. 配置参数

`save_video` 插件的参数均在 `configure` 字段中进行配置。

### 2.1. JSON配置示例

```json
{
  "configure": {
    "server_url": "http://182.92.230.1:9181/warning-agreement/upload",
    "save_dir": "/data/alarms",
    "base_file_url": "http://127.0.0.1/static", 
    "record_seconds": 10,
    "retention_days": 1,
    "retention_max_gb": 1,
    "cleanup_interval_seconds": 300,
    "deviceId": "1287",
    "deviceIp": "127.0.0.1",
    "safetyId": "1",
    "safetyName": "安全员1",
    "warning": "way",
    "video_url_field": "safetyUrl",
    "trigger_classes": [0,1,2],
    "min_trigger_frames": {
      "0": 20,
      "1": 5,
      "2": 1
    },
    "type": {
      "0": 10,
      "1": 2,
      "2": 6,
    },
  },
  "shared_object": "../../build/lib/libsave_video.so",
  "name": "save_video",
  "side": "sophgo",
  "thread_number": 1
}

```

### 2.2. 参数说明

| 参数名                       | 类型          | 默认值            | 说明                                                                                                                                                       |
| ---------------------------- | ------------- | ----------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------- |
| **录制与存储**         |               |                   |                                                                                                                                                            |
| `save_dir`                 | string        | (无)              | **(必需)** 视频和快照文件的根存储目录。                                                                                                              |
| `record_seconds`           | int           | 10                | 触发事件后，继续录制的时长（秒）。                                                                                                                         |
| `trigger_classes`          | array of int  | `[]` (空数组)   | 触发录制的目标类别ID列表。如果为空数组，则任何检测到的目标都会触发录制。                                                                                   |
| `min_trigger_frames`       | int 或 object | 1                 | 触发录制所需的最少连续帧数。可设为全局整数，或按类别设置的对象，如 `{"0": 5, "1": 3}`。                                                                  |
| **报警上报**           |               |                   |                                                                                                                                                            |
| `server_url`               | string        | (无)              | **(必需)** 接收报警信息的HTTP服务器完整URL。                                                                                                         |
| `base_file_url`            | string        | `""` (空字符串) | 用于构建报警信息中文件访问URL的基地址。最终URL格式为 `{base_file_url}/{年-月-日}/{文件名}`。如果为空，则上报的URL为文件的绝对路径。                      |
| `deviceId`                 | string        | `""`            | 上报JSON中的 `deviceId`字段值。                                                                                                                          |
| `deviceIp`                 | string        | `""`            | 上报JSON中的 `deviceIp`字段值。                                                                                                                          |
| `safetyId`                 | string        | `""`            | 上报JSON中的 `safetyId`字段值。                                                                                                                          |
| `safetyName`               | string        | `""`            | 上报JSON中的 `safetyName`字段值。                                                                                                                        |
| `warning`                  | string        | `""`            | 上报JSON中的 `warning`字段值。                                                                                                                           |
| `type`                     | int 或 object | (使用class_id)    | 上报JSON中的 `type`字段值。可设为固定整数，或按类别映射的对象，如 `{"0": 101, "1": 102}`。若不配置，则直接使用检测到的目标 `class_id`作为 `type`。 |
| `video_url_field`          | string        | `"safetyUrl"`   | 上报JSON中视频URL所用的字段名。                                                                                                                            |
| **存储清理**           |               |                   |                                                                                                                                                            |
| `retention_days`           | int           | 0                 | 文件保留的最长天数。超过此天数的文件将被自动删除。0表示不启用此策略。                                                                                      |
| `retention_max_gb`         | double        | 0.0               | 存储目录允许占用的最大磁盘空间（GB）。超过此限制将从最旧的文件开始删除。0表示不启用此策略。                                                                |
| `cleanup_interval_seconds` | int           | 300               | 自动清理任务的运行间隔（秒）。                                                                                                                             |
| **基础配置**           |               |                   |                                                                                                                                                            |
| `shared_object`            | string        | (无)              | **(必需)** `libsave_video.so` 动态库的路径。                                                                                                       |
| `name`                     | string        | `"save_video"`  | element的名称。                                                                                                                                            |
| `side`                     | string        | `"sophgo"`      | 设备类型。                                                                                                                                                 |
| `thread_number`            | int           | 1                 | 启动的线程数。                                                                                                                                             |

> **注意**
>
> 1. `save_video` element 通常作为数据流的末端，需要保证其 `thread_number` 与输入数据流的路数一致，以确保每个通道的数据都能被独立处理。
> 2. `save_dir` 和 `server_url` 是必需参数，必须正确配置。
> 3. 存储清理功能在一个独立的后台线程中运行，不会阻塞主处理流程。
