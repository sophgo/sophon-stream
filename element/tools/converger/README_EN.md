# sophon-stream converger element

English | [简体中文](README.md)

sophon-stream converger element is a plugin within the sophon-stream framework, designed as a specialized tool for data convergence purposes

## 1. feature
* Must be used in conjunction with the distributor element.
* Ensures that the output ObjectMetadata maintains the correct chronological order.

## 2. Configuration parameters
sophon-stream converger plugin has several configurable parameters that can be adjusted according to specific requirements. Here are some commonly used parameters:
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

| Parameter Name|  name  |        Default value             |            Description                   |
| ------------- | ------ | ------------------------------------ | ------------------------------- |
| default_port  | int    | \                                    | Port for receiving data from the distributor element |
| shared_object | string | "../../../build/lib/libconverger.so" | libconverger dynamic library path         |
| name          | string | "converger"                          | element name                     |
| side          | string | "sophgo"                             | device type                      |
| thread_number | int    | 1                                    | Thread number                      |

> **notes**
1. Once the converger element receives `ObjectMetadata` from the `default_port`, it waits for all its branches to finish updating before transmitting to subsequent elements.
2. Before sending, it sequentially stores all data; during transmission, it sends all completed updated data in sequence.
3. The converger element must be used in conjunction with the distributor element.
