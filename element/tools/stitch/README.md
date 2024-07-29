# sophon-stream stitch element


sophon-stream stitch element是sophon-stream框架中的一个插件，是一个用于拼接的插件。

## 1. 特性
暂时只能配合dpu使用。

## 2. 配置参数
sophon-stream stitch插件具有一些可配置的参数，可以根据需求进行设置。以下是一些常用的参数：

```json
{
  "configure": {
    "stitch_mode": "HORIZONTAL"
  },
  "shared_object": "../../build/lib/libstitch.so",
  "name": "stitch",
  "side": "sophgo",
  "thread_number": 1
}
```


| 参数名      | 类型   | 默认值 | 说明                                         |
| ----------- | ------ | ------ | -------------------------------------------- |
| stitch_mode | string | 无     | 设置图像的拼接模型，可选HORIZONTAL和VERTICAL |


## 3. 配置示例
## 3.1 stitch_demo
```json
{
  "channels": [
    {
      "channel_id": 2,
      "url": "../stitch/data/test_car_person_1080P.mp4",
      "source_type": "VIDEO",
      "sample_interval": 1,
      "loop_num": 5,
      "fps": -1,
      "decode_id": 5000
    },
    {
      "channel_id": 3,
      "url": "../stitch/data/test_car_person_1080P.mp4",
      "source_type": "VIDEO",
      "sample_interval": 1,
      "loop_num": 5,
      "fps": -1,
      "decode_id": 5001
    }
  ],

  "engine_config_path": "../stitch/config/engine.json"
}
```

### 3.2 decode
```json
{
  "configure": {},
  "shared_object": "../../build/lib/libdecode.so",
  "name": "decode",
  "side": "sophgo",
  "thread_number": 1
}
```
### 3.3 resize
使用resize的原因是因为拼接后的图像较大，可以适当缩放一下再进行下一步。
```json
{
  "configure": {
    "dst_w":1920,
    "dst_h":1080,
    "crop_w": 3840,
    "crop_h": 1080,
    "crop_top": 0,
    "crop_left": 0
  },
  "shared_object": "../../build/lib/libresize.so",
  "name": "resize",
  "side": "sophgo",
  "thread_number": 1
}
```

### 3.4 engine
```json
[
    {
        "graph_id": 0,
        "device_id": 0,
        "graph_name": "stitch",
        "elements": [
            {
                "element_id": 5000,
                "element_config": "../stitch/config/decode.json",
                "ports": {
                    "input": [
                        {
                            "port_id": 0,
                            "is_sink": false,
                            "is_src": true
                        }
                    ]
                }
            },
            {
                "element_id": 5001,
                "element_config": "../stitch/config/decode.json",
                "ports": {
                    "input": [
                        {
                            "port_id": 0,
                            "is_sink": false,
                            "is_src": true
                        }
                    ]
                }
            },
            {
                "element_id": 5002,
                "element_config": "../stitch/config/stitch.json"
            },
            {
                "element_id": 5003,
                "element_config": "../stitch/config/resize.json",
                "ports": {
                    "output": [
                        {
                            "port_id": 0,
                            "is_sink": true,
                            "is_src": false
                        }
                    ]
                }
            }
        ],
        "connections": [
            {
                "src_element_id": 5000,
                "src_port": 0,
                "dst_element_id": 5002,
                "dst_port": 0
            },
            {
                "src_element_id": 5001,
                "src_port": 1,
                "dst_element_id": 5002,
                "dst_port": 0
            },
            {
                "src_element_id": 5002,
                "src_port": 0,
                "dst_element_id": 5003,
                "dst_port": 0
            }
        ]
    }
]
```