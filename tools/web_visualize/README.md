# Web Visualize Tool
本工具可以实现接受post请求，并在web端显示请求中的rtsp流，最大支持16路视频显示

## 环境准备
本工具依赖环境 python (3.8.9) opencv-python (4.7.0.72), Flask (2.3.2) websockets(11.0.3)。
另外，本工具依赖8000，8765-8780端口，请保证端口未被占用或更换端口。

## 运行
```shell
python3 app.py
```
运行后可登陆 http://127.0.0.1:8000/ 访问前端界面

发送post请求至 http://127.0.0.1:8000/pushstream 可实现前端视频显示

## 接口定义
url: https://ip:port/pushstream

method: post

请求头: content-type: application/json;charset=UTF-8

请求json: {"url": "rtsp url"}

响应json:

Success respond: Code: 200 OK

Error respond: Code: 500 Internal Server Error
{ "error": "can not open rtsp/out of Road" }