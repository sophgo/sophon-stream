# sophon-stream http_push element

English | [简体中文](README.md)

Sophon-stream `http_push` element is a plugin within the Sophon-stream framework. It serves as a specialized tool for ObjectMetadata serialization, base64 encoding, and transmission, supporting both http and https.

## 1. Configuration Parameters
Sophon-stream http_push plugin comes with several configurable parameters that can be adjusted according to requirements. Here are some commonly used parameters:

```json
{
    "configure": {
        "ip": "0.0.0.0",
        "port" : 8000,
        "path" : "/stream/test"
    },
    "shared_object": "../../../build/lib/libhttp_push.so",
    "name": "http_push",
    "side": "sophgo",
    "thread_number": 1
}
```

| Parameter Name|  name  |        Default value                       |            Description           |
| ------------- | ------ | ------------------------------------ | ------------------------------- |
| scheme        | string | "http"                               | use "https" for https protocol |
| ip            | string | "0.0.0.0"                            | `httplib::Client` ip           |
| port            | int | 8000                                  | The port for `httplib::Client`      |
| cert            | string |                                   | The client_cert for `httplib::Client`     |
| key            | string |                                   | The client_key for `httplib::Client`     |
| cacert            | string |                                   | The ca_cert_path for `httplib::Client`     |
| verify            | bool |                                   | Whether enable_server_certificate_verification     |
| path            | string | "/stream/test"                                | The path of http request      |
| shared_object | string | "../../../build/lib/libhttp_push.so" | libhttp_push dynamic library path      |
| name          | string | "http_push"                          | element name                     |
| side          | string | "sophgo"                             | device type                       |
| thread_number | int    | 1                                    | thread num                      |

> **notes**
1. When using the `http_push` element, it's important to ensure that the number of threads started matches the number of input stream routes.
