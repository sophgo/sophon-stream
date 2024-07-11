# sophon-stream http_push element

[English](README_EN.md) | 简体中文

sophon-stream http_push element是sophon-stream框架中的一个插件，是一个专用作ObjectMetadata序列化、base64编码和发送的工具，支持http和https协议。

## 1. 配置参数
sophon-stream http_push插件具有一些可配置的参数，可以根据需求进行设置。以下是一些常用的参数：

```json
{
    "configure": {
        "ip": "0.0.0.0",
        "port" : 8000,
        "path": "/stream/test"
    },
    "shared_object": "../../../build/lib/libhttp_push.so",
    "name": "http_push",
    "side": "sophgo",
    "thread_number": 1
}
```

| 参数名        | 类型   | 默认值                               | 说明                            |
| ------------- | ------ | ------------------------------------ | ------------------------------- |
| scheme        | string | "http"                            | 传输协议，用https时需填写https     |
| ip            | string | "0.0.0.0"                            | httplib::Client的ip            |
| port            | int | 8000                            | httplib::Client的端口            |
| cert            | string |                             | 服务器的证书文件路径，发送https请求时使用           |
| key            | string |                             | 服务器的密钥文件路径，发送https请求时使用           |
| cacert            | string |                             | 验证服务器证书的ca证书路径，发送https请求时使用           |
| veriry            | bool |                             | 是否验证证书，是填写true，否填写false           |
| path            | string | "/stream/test"                     | http请求的path            |
| shared_object | string | "../../../build/lib/libhttp_push.so" | libhttp_push动态库路径          |
| name          | string | "http_push"                          | element名称                     |
| side          | string | "sophgo"                             | 设备类型                        |
| thread_number | int    | 1                                    | 启动线程数                      |

> **注意**
1. http_push element 使用时需要保证启动线程数与输入码流路数一致
