# sophon-stream http_push element

English | [简体中文](README.md)

Sophon-stream `http_push` element is a plugin within the Sophon-stream framework. It serves as a specialized tool for ObjectMetadata serialization, base64 encoding, and transmission.

## 1. Configuration Parameters
Sophon-stream http_push plugin comes with several configurable parameters that can be adjusted according to requirements. Here are some commonly used parameters:

```json
{
    "configure": {
        "ip": "0.0.0.0",
        "port" : 8000
    },
    "shared_object": "../../../build/lib/libhttp_push.so",
    "name": "http_push",
    "side": "sophgo",
    "thread_number": 1
}
```

| Parameter Name|  name  |        Default value                       |            Description           |
| ------------- | ------ | ------------------------------------ | ------------------------------- |
| ip            | string | "0.0.0.0"                            | `httplib::Client` ip           |
| port            | int | 8000                                  | The port for `httplib::Client`, determined by the sum of 'port' and channel_id'.      |
| shared_object | string | "../../../build/lib/libhttp_push.so" | libhttp_push dynamic library path      |
| name          | string | "http_push"                          | element name                     |
| side          | string | "sophgo"                             | device type                       |
| thread_number | int    | 1                                    | thread num                      |

> **notes**
1. When using the `http_push` element, it's important to ensure that the number of threads started matches the number of input stream routes.