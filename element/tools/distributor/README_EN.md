# sophon-stream distributor element

English | [简体中文](README.md)

The sophon-stream distributor element is a plugin within the sophon-stream framework designed specifically for data distribution purposes.

## 1. feature
* Must be used in conjunction with the converger element.
* Supports distribution based on categories.
* Supports distribution based on time intervals.
* Supports distribution based on frame intervals.

## 2. Configuration Parameters
Sophon-stream distributor plugin has several configurable parameters that can be adjusted according to specific requirements. Here are some commonly used parameters:

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

| Parameter Name|  name  |        Default value             |            Description                   |
| ---------------- | ------ | -------------------------------------- | -------------------------- |
| default_port     | int    | \                                      | the port sending data to the converger element.  |
| rules            | vector | []                                     | the current distribution logic of the element.  |
| time_interval    | float  | \                                      | the interval between distributions, in seconds.   |
| frame_interval   | int    | \                                      | the interval between distributions in frames.              |
| routes           | vector | []                                     | the distribution paths at the current interval.   |
| classes          | vector | []                                     | a set of categories.                   |
| port             | int    | 1                                      | the distribution port corresponding to the current classes.  |
| class_names_file | string | ""                                     | directory containing names of all classes. |
| shared_object    | string | "../../../build/lib/libdistributor.so" | libdistributor dynamic library path   |
| name             | string | "distributor"                          | element name              |
| side             | string | "sophgo"                               | device type               |
| thread_number    | int    | 1                                      | thread number                 |

> **notes**：
1. Due to the involvement of crop functionality in distribution, it is generally not recommended to set the `time_interval` or `frame_interval` parameters too small, as it might cause blocking.
2. When distributing to the `default_port`, the `interval` parameter is not involved. This port is connected to the converger element and serves to aggregate the branch data.
3. `classes` can be configured for a set of categories. In this case, all categories in this parameter are sent to the corresponding port. For instance, categorizing all animals, such as cat and dog, uniformly based on color.
4. When the `classes` item is not empty, by default, it performs crop and distributes for each category. If it is empty, it is considered to distribute the current large image.
5. Distribution rules depend on business requirements and can be individually configured for time intervals or frame intervals, or a combination of both, forming complex distribution rules.
6. In the design, when users do not fill in the `time_interval` or `frame_interval` parameters, it is considered that each frame is distributed according to the `routes`, which is equivalent to `frame_interval == 1`. However, it should be noted, **as the note 1**, such settings may cause blocking.
7. The distributor element must be used in conjunction with the converger element.

