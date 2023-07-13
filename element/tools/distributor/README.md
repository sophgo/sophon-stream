# sophon-stream distributor element

sophon-stream distributor element是sophon-stream框架中的一个插件，是一个专用作数据分发功能的工具。

## 1. 特性
* 必须与converger element配合使用
* 支持按类别分发
* 支持按时间间隔分发
* 支持按帧间隔分发

## 2. 配置参数
sophon-stream distributor插件具有一些可配置的参数，可以根据需求进行设置。以下是一些常用的参数：

```json
{
    "configure": {
        "default_port": 0,
        "rules" : [
            {
                "time_interval": 1,
                "routes": [
                    {
                        "classes":["car"],
                        "port": 1
                    },
                    {
                        "classes":["person"],
                        "port" : 2
                    }
                ]
            },
            {
                "frame_interval": 10,
                "routes": [
                    {
                        "classes":["cat"],
                        "port": 3
                    },
                    {
                        "classes":[],
                        "port" : 4
                    }
                ]
            }
        ],
        "class_names_file" : "../data/coco.names"
      },
      "shared_object": "../../../build/lib/libdistributor.so",
      "name": "distributor",
      "side": "sophgo",
      "thread_number": 1
}
```

| 参数名           | 类型   | 默认值                                 | 说明                       |
| ---------------- | ------ | -------------------------------------- | -------------------------- |
| default_port     | int    | 无                                     | 发往数据汇聚element的端口  |
| rules            | vector | []                                     | 当前element的所有分发逻辑  |
| time_interval    | float  | 无                                     | 分发的时间间隔，单位：秒   |
| frame_interval   | int    | 无                                     | 分发的帧间隔               |
| routes           | vector | []                                     | 当前interval下的分发路径   |
| classes          | vector | []                                     | 一组类别                   |
| port             | int    | 1                                      | 当前classes对应的分发端口  |
| class_names_file | string | ""                                     | 存放所有类别名称的文件目录 |
| shared_object    | string | "../../../build/lib/libdistributor.so" | libdistributor动态库路径   |
| name             | string | "distributor"                          | element名称                |
| side             | string | "sophgo"                               | 设备类型                   |
| thread_number    | int    | 1                                      | 启动线程数                 |

> **注意**：
1. 由于分发涉及crop功能，`time_interval`或`frame_interval`参数一般不建议设置太小，否则可能造成阻塞。
2. 向`default_port`分发时，不涉及`interval`参数。该端口连接到converger element，起到将分支数据汇聚起来的作用。
3. `classes`可以配置一组类别，此种情况下，该参数中的所有类别都发送往对应的端口。例如对所有动物，如`cat`和`dog`，统一做颜色分类。
4. 当`classes`项不为空时，默认对每个类别做crop后分发。若为空，则认为分发当前大图。
5. 分发规则视业务需求而定，可以单独配置时间间隔、也可以单独配置帧间隔，亦可二者结合，形成复杂的分发规则。
6. 设计上，当用户不填写`time_interval`或`frame_interval`参数时，会视为对每一帧都按照`routes`进行分发，即相当于`frame_interval == 1`的情况。但需要注意，同【注意事项1】，如此设置可能会造成阻塞。
7. distributor element必须搭配converger element使用。