# sophon-stream distributer element

sophon-stream distributer element是sophon-stream框架中的一个插件，是一个专用作数据分发功能的工具。

## 1. 特性
* 支持按类别分发
* 支持按时间分发

## 2. 配置参数
sophon-stream distributer插件具有一些可配置的参数，可以根据需求进行设置。以下是一些常用的参数：

```json
{
    "configure": {
        "default_port": 0,
        "rules" : [
            {
                "interval": 1, 
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
                "interval": 2, 
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
      "shared_object": "../../../build/lib/libdistributer.so",
      "name": "distributer",
      "side": "sophgo",
      "thread_number": 4
}
```

| 参数名 | 类型 | 默认值 | 说明 |
| ------ | ---- | ---- | -----|
| default_port | int | 0 | 发往数据汇聚element的端口 |
| rules | vector | [] | 当前element的所有分发逻辑 |
| interval | float | 1.0 | 分发的时间间隔 |
| routes | vector | [] | 当前interval下的分发路径 |
| classes | vector | [] | 一组类别 |
| port | int | 1 | 当前classes对应的分发端口 |
| class_names_file | string | "" | 存放所有类别名称的文件目录 |
| shared_object | string | "../../../build/lib/libdistributer.so" | libdistributer动态库路径 |
| name | string | "distributer" | element名称 |
| side | string | "sophgo" | 设备类型 |
| thread_number | int | 1 | 启动线程数 |

> **注意**：
1. 如果分发涉及crop功能，interval参数一般不建议设置太小，可能造成阻塞。
2. 向`default_port`分发时，不涉及interval参数。该端口连接到converger element，起到将分支数据汇聚起来的作用。
3. 若分发规则中不涉及时间间隔，需要将`interval`设置为0。
4. 当`classes`项不为空时，默认对当前类别做crop后分发。若为空，则认为分发当前大图。
5. distributer element支持复杂的分发策略，例如不同的时间间隔、不同的分发路径等。对于一个分发路径，其classes可以是多个类别组成的vector。