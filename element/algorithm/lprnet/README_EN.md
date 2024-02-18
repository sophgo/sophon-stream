# sophon-stream lprnet element

English | [简体中文](./README_EN.md)

The sophon-stream lprnet element is a plugin within the sophon-stream framework, serving as a simple, fast, and powerful license plate recognition model. This project has provided a plugin example for yolo recognition and lprnet license plate detection. For details, please refer to [license_plate_recognition](../../../samples/license_plate_recognition/README.md).

## Features

- Support for multiple video streams
- Support for multi-threading

## 2. Configuration Parameters

The sophon-stream lprnet plugin is divided into three parts: preprocessing, inference, and post-processing, each with configurable parameters that can be adjusted according to specific requirements. Taking inference as an example, here are some commonly used parameters:

```json
{
  "configure": {
    "model_path": "../models/BM1684/lprnet_fp32_1b.bmodel",
    "stage": ["infer"]
  },
  "shared_object": "../../../build/lib/liblprnet.so",
  "name": "lprnet",
  "side": "sophgo",
  "thread_number": 4
}
```

|   Parameter Name   |  Type  |                Default Value                |             Description             |
| :-----------------: | :----: | :----------------------------------------: | :---------------------------------: |
|     model_path     | String | "../models/BM1684/lprnet_fp32_1b.bmodel"   |          Path to the lprnet model          |
|        stage        |  List  |                     ["pre"]                 | Flags for the preprocessing, inference, and post-processing stages |
|   shared_object    | String |    "../../../build/lib/liblprnet.so"       |       Path to the liblprnet dynamic library      |
|        name         | String |                   "lprnet"                  |              Element name               |
|        side         | String |                   "sophgo"                  |              Device type               |
|   thread_number    | Integer|                      1                     |             Number of threads to launch              |

> **Note**:

1. For the `stage` parameter, it needs to be set to one or a combination of "pre", "infer", "post", and should be connected in order of preprocessing, inference, and post-processing stages. Allocating these three stages to three elements aims to make full use of resources, improving detection efficiency.
