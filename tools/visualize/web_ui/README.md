# Web UI 使用方法

## 1 启动 Web UI
输入下列代码启动程序
```bash
cd build
python3 -m http.server 3000
```

在浏览器中输入下列网址来打开页面
```
http:/{host-ip}:3000/index.html
```

>注意：
>1.前后端服务默认运行在同一台机器，host-ip为服务器地址，3000为浏览器端口，如有占用可更换
>2.此方法可无需npm启动web_ui
>3.如果显示视频画面需要配置encode.json中的"encode_type"为"WS"

## 2 使用react启动
如果需要开发调试，需要安装npm
```bash
sudo apt install npm
```

输入以下命令启动程序
```bash
npm install --legacy-peer-deps
npm start
```

注意：使用react启动本项目依赖node(>v14.17.0)和npm(>v6.14.13)工具，react默认使用3000端口

## 3 Pipeline Json配置说明
### 3.1 yolov5参数配置

yolov5是sophon-stream中一种视频目标检测应用。yolov5例程算法的前处理、推理、后处理分别在三个element上进行运算，element内部可以开启多个线程，保证了一定的检测效率。

yolov5 demo中各部分参数位于 [config](../../../samples/yolo5/config/) 目录，结构如下所示：

```bash
./config/
├── decode.json                 # 解码配置
├── engine.json                 # sophon-stream graph配置
├── yolov5_demo.json            # yolov5 demo配置
├── yolov5_infer.json           # yolov5 推理配置
├── yolov5_post.json            # yolov5 后处理配置
└── yolov5_pre.json             # yolov5 前处理配置
```
更详细的介绍，请参见 [yolov5的README.md](../../../samples/yolov5/README.md)。

### 3.2 yolox参数配置
yolox是sophon-stream中一种视频目标检测应用。yolox由旷视提出，是基于YOLO系列的改进.
在yolox例程中，yolox算法的前处理、推理、后处理分别在三个element上进行运算，element内部可以开启多个线程，保证了一定的检测效率。

yolox demo中各部分参数位于 [config](../../../samples/yolox/config/) 目录，结构如下所示：

```bash
./config
├── decode.json             # 解码配置
├── engine.json             # sophon-stream graph配置
├── yolox_demo.json         # yolox demo配置
├── yolox_infer.json        # yolox 推理配置
├── yolox_post.json         # yolox 后处理配置
└── yolox_pre.json          # yolox 前处理配置
```

更详细的介绍，请参见 [yolox的README.md](../../../samples/yolox/README.md)。

### 3.3 resnet参数配置
resnet是sophon-stream中的一种视频目标分类应用。深度残差网络（Deep residual network, ResNet）是由于Kaiming He等在2015提出的深度神经网络结构，它利用残差学习来解决深度神经网络训练退化的问题。

resnet demo中各部分参数位于 [config](../../../samples/resnet/config/) 目录，结构如下所示：

```bash
./config
├── decode.json             # 解码配置
├── engine.json             # sophon-stream graph配置
├── resnet_demo.json        # resnet demo配置
├── resnet_roi.json         # resnet roi配置
└── resnet_classify.json    # resnet 插件配置
```

更详细的介绍，请参见 [resnet的README.md](../../../samples/resnet/README.md)。

### 3.4 bytetrack参数配置
bytetrack是sophon-stream中一种视频目标跟踪应用。ByteTrack是一个简单、快速、强大的多目标跟踪器，且不依赖特征提取模型。

bytetrack demo中各部分参数位于 [config](../../../samples/bytetrack/config/) 目录，结构如下所示：

```bash
./config
   ├── bytetrack_demo.json       # bytetrack demo 配置
   ├── bytetrack.json            # bytetrack目标跟踪器参数配置
   ├── decoder.json              # 解码配置
   ├── engine.json               # sophon-stream graph配置
   ├── infer.json                # 目标检测器推理配置
   ├── post.json                 # 目标检测器后处理配置
   └── pre.json                  # 目标检测器前处理配置
```

更详细的介绍，请参见 [bytetrack的README.md](../../../samples/bytetrack/README.md)。

### 3.5 yolov5_bytetrack_distributor_resnet_converger参数配置
yolov5_bytetrack_distributor_resnet_converger是sophon-stream中一种包含了多算法和按类别发往不同分支的复杂应用。

yolov5_bytetrack_distributor_resnet_converger demo中各部分参数位于 [./config](../../../samples/yolov5_bytetrack_distributor_resnet_converger/config/)目录，结构如下所示：

```bash
./config
├── bytetrack.json                                                    # bytetrack跟踪算法配置
├── converger.json                                                    # 汇聚element配置
├── decode.json                                                       # 解码配置
├── distributor_class.json                                            # 每帧按类别分发
├── distributor_frame_class.json                                      # 跳帧按类别分发
├── distributor_frame.json                                            # 跳帧分发full frame
├── distributor_time_class.json                                       # 间隔时间按类别分发（默认）
├── distributor_time.json                                             # 间隔时间分发full frame
├── engine.json                                                       # graph配置
├── resnet_car.json                                                   # resnet 车辆颜色分类
├── resnet_person.json                                                # resnet 行人性别分类
├── yolov5_bytetrack_distributor_resnet_converger_demo.json           # demo配置
├── yolov5_infer.json                                                 # yolov5 推理配置
├── yolov5_post.json                                                  # yolov5 后处理配置
└── yolov5_pre.json                                                   # yolov5 前处理配置
```

更详细的介绍，请参见 [yolov5_bytetrack_distributor_resnet_converger的README.md](../../../samples/yolov5_bytetrack_distributor_resnet_converger/README.md)。

### 3.6 yolox_bytetrack_osd_encode参数配置
yolox_bytetrack_osd_encode是sophon-stream中一种视频目标检测跟踪应用，最后将算法结果推流输出。

yolox_bytetrack_osd_encode demo中各部分参数位于 [./config](../../../samples/yolox_bytetrack_osd_encode/config/)目录，结构如下所示：

```bash
./config
├── bytetrack.json                                                    # bytetrack跟踪算法配置
├── decode.json                                                       # 解码配置
├── encode.json                                                       # 编码配置
├── engine.json                                                       # graph配置
├── osd.json                                                          # 对具体某个element的配置细节
├── ws.json                                                           # websocket配置
├── yolox_bytetrack_osd_encode_demo.json                              # demo配置
├── yolox_infer.json                                                  # yolox 推理配置
├── yolox_post.json                                                   # yolox 后处理配置
├── yolox_pre.json                                                    # yolox 前处理配置
```

更详细的介绍，请参见 [yolox_bytetrack_osd_encode的README.md](../../../samples/yolox_bytetrack_osd_encode/README.md)。
