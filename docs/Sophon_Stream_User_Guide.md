# 算能 sophon-stream 用户手册

## 1. 快速入门

### 1.1 安装和配置环境

#### 1.1.1 x86/arm PCIe平台

如果您在x86/arm平台安装了PCIe加速卡（如SC系列加速卡），可以直接使用它作为开发环境和运行环境。您需要安装libsophon、sophon-opencv和sophon-ffmpeg，具体步骤可参考[x86-pcie平台的开发和运行环境搭建](EnvironmentInstallGuide.md#3-x86-pcie平台的开发和运行环境搭建)或[arm-pcie平台的开发和运行环境搭建](EnvironmentInstallGuide.md#5-arm-pcie平台的开发和运行环境搭建)。

### 1.1.2 SoC平台

如果您使用SoC平台（如SE、SM系列边缘设备），刷机后在`/opt/sophon/`下已经预装了相应的libsophon、sophon-opencv和sophon-ffmpeg运行库包，可直接使用它作为运行环境。通常还需要一台x86主机作为开发环境，用于交叉编译C++程序。

### 1.2 编译命令

完成环境配置后，用户可以参考 [sophon-stream编译指南](./HowToMake.md)，使用如下命令编译。

### 1.2.1 x86/arm PCIe平台

```bash
mkdir build
cd build 
cmake ../ -DCMAKE_BUILD_TYPE=Debug -DTARGET_ARCH=pcie
make -j
```

### 1.2.2 SoC平台

通常在x86主机上交叉编译程序，您需要在x86主机上使用SOPHON SDK搭建交叉编译环境，将程序所依赖的头文件和库文件打包至sophon_sdk_soc目录中，具体请参考[sophon-stream编译](./HowToMake.md)。本例程主要依赖libsophon、sophon-opencv和sophon-ffmpeg运行库包。

```bash
mkdir build
cd build 
cmake ../ -DCMAKE_BUILD_TYPE=Debug -DTARGET_ARCH=soc -DSOPHON_SDK_SOC=${path_to_sophon_soc_sdk}
make -j
```

注意：交叉编译时，${path_to_sophon_soc_sdk} 变量指运行交叉编译命令的x86主机上的arm sophon-sdk目录。

具体地，CMakeLists.txt中提供了插件化的编译指令。如果用户不需要启用sophon-stream的全部功能，可以适当选择其中一些插件的编译命令进行注释，例如:

```cmake
add_subdirectory(element/algorithm/decode)
add_subdirectory(element/algorithm/yolox)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/samples/yolox/build)
add_subdirectory(samples/yolox)
```

上例中，脚本将分别编译yolox算法插件，以及算法对应的例程。如果用户的工程中不涉及其中某项功能，可以将该部分算法及对应的例程编译命令进行注释。

### 1.3 编译结果

sophon-stream中，除了sample以外的每个模块都以插件的形式参与运行。完成 [1.2 编译命令](#12-编译命令) 后，用户可以在 ./build/lib/ 目录下看到每个参与编译的插件对应的动态库文件。

samples中的源文件，其编译结果是可执行程序。例如，yolox例程的可执行程序位于 [yolox](../samples/yolox/build/yolox_demo) 。

## 2. 概述

sophon-stream是面向算丰开发平台的数据流处理工具。本软件基于插件化的思想，使用C++11开发了一套支持多路数据流并发处理的流水线框架。基于现有的接口，sophon-stream对用户具有易使用、易二次开发的优点，可以大大简化用户配置工程或添加插件的复杂度。sophon-stream基于SophonSDK，可以充分发挥算丰硬件的编解码能力及人工智能算法的推理能力，从而获得较高的性能。

## 2.1 sophon-stream优势

sophon-stream具有以下优点：
  
 - 稳健灵活的基础框架。在保证sophon-stream基础框架的稳健性的同时，它也具有相当大的灵活性。用户可以简单地配置json文件，从而准确方便地搭建复杂的业务流水线。
 - 完备的软硬件生态体系。sophon-stream基于算丰芯片的底层特点，包含了编解码硬件加速、常规的图像处理加速以及推理加速功能，可以充分发挥算丰芯片的性能优势，极大地提升整体的吞吐效率。
 - 丰富的算法库。sophon-stream支持多种目标检测及跟踪算法，例如yolox、yolov5、bytetrack等。
 - 便于部署。sophon-stream适用于算丰BM1684、BM1684X芯片，可以在PCIE、SOC模式下灵活部署。

## 2.2 sophon-stream软件栈

sophon-stream基于SophonSDK设计。SophonSDK是算能科技基于自主研发的 AI 芯片所定制的深度学习SDK，涵盖了神经网络推理阶段所需的模型优化、高效运行时支持等能力，为深度学习应用开发和部署提供易用、高效的全栈式解决方案。

![stream_and_sdk](./pics/stream_sdk.png)

sophon-stream由framework和element两部分组成，framework是整体的框架，作为底层决定了sophon-stream的运行方式，如图的构建、数据传输等。element是所有图节点的统称，它们由同一个抽象基类派生而来，负责基于SophonSDK提供某项特定功能，如视频编解码、图像处理等。

## 3. 框架

sophon-stream框架包含三层结构，分别是Engine，Graph(Element Manager)和Element。三者之间的层次关系如下图所示。

![engine](./pics/engine.png)

Engine是sophon-stream中最外层的结构，向外部工程提供接口。Engine管理着多个Graph，而每个Graph是一张独立的有向无环图，管理着多个Element。

### 3.1 Engine

engine类是一个单例，一个进程中只存在一个engine。engine类对外的接口主要包括：

```cpp
// 启停某个graph
common::ErrorCode start(int graphId);
common::ErrorCode stop(int graphId);
// 添加一个graph
common::ErrorCode addGraph(const std::string& json);
// 向某个graph中的某个element推入数据。用于启动解码功能。
common::ErrorCode pushInputData(int graphId, int elementId, int inputPort,
                                std::shared_ptr<void> data);
// 为某个graph的某个element的sinkPort设置数据处理函数，例如绘图、发送等。
void setStopHandler(int graphId, int elementId, int outputPort,
                    DataHandler dataHandler);
```

### 3.2 Element Manager

element_manager类的实例由engine管理，它提供接口给engine调用，主要在初始化或析构一张图时起作用。element_manager类对外的接口主要包括：

```cpp
// 初始化及反初始化当前graph
common::ErrorCode init(const std::string& json);
void uninit();
// 启停当前graph
common::ErrorCode start();
common::ErrorCode stop();
// 向某个element推入数据，用于启动DecoderElement的解码任务
common::ErrorCode pushInputData(int elementId, int inputPort,
                                std::shared_ptr<void> data);
// 为某个element的sinkPort设置数据处理函数，例如绘图、发送等
void setStopHandler(int elementId, int outputPort, DataHandler dataHandler);
```
### 3.3 Element

element类是sophon-stream的通用基类，用户二次开发的插件也都基于element。作为一个抽象类，element类统一规定了所有派生类的主要接口和成员，包括数据如何传递、线程如何管理、两个element之间通过何种方式连接等。

一个element的结构如下图所示: 

![element](./pics/element.png)

Element与外部的数据传递通过connector来进行，每个输入或输出port都有一个connector与之对应。从输入connector获取数据之后，element在run()方法中调用doWork()来对数据进行处理，然后分发给outputPort对应的connector，即下一个element的输入connector。

element基类的主要成员变量:

```cpp
int mId;     // element id，用于在engine及element_manager中确定element的身份，在graph中具有唯一性
int mDeviceId;  // device id，涉及tpu操作时使用的设备id。pcie模式下可以按需设置，soc模式下应设置为0
int mThreadNumber; // element内部工作的线程数，也等于InputConnector中的DataPipe数目

// 管理输入和输出Connector的映射，key是输入或输出的port_id，value是指向Connector的指针
std::map<int, std::shared_ptr<framework::Connector>> mInputConnectorMap;
std::map<int, std::weak_ptr<framework::Connector>> mOutputConnectorMap;

/* 管理输出StopHandler的映射，key是输出的port_id，value是一个签名为void(std::shared_ptr<void>)的函数。StopHandler为graph末尾的元素提供数据处理功能，一般包括绘图、推流等。 */
std::map<int, DataHandler> mStopHandlerMap;
```

主要的成员函数: 
```cpp
/* static方法，连接两个element。设置srcElement的输出port和dstElement的输入port，并把dstElement的inputConnector注册到srcElement的mOutputConnectorMap */
static void connect(Element& srcElement, int srcElementPort, Element& dstElement, int dstElementPort);

// 从配置文件初始化element的通用属性，例如element id、thread number等
common::ErrorCode init(const std::string& json)
// 启停element
common::ErrorCode start();
common::ErrorCode stop();
// push数据，用于启动DecoderElement的解码任务
common::ErrorCode pushInputData(int inputPort, int dataPipeId, std::shared_ptr<void> data);

// 纯虚函数，派生类中用于初始化自定义的属性，例如算法相关内容
virtual common::ErrorCode initInternal(const std::string& json) = 0;
// 纯虚函数，派生类中自定义具体的算法逻辑，一般为[pop数据——组batch——运行算法——push数据]等
virtual common::ErrorCode doWork(int dataPipeId) = 0;
// 循环调用doWork()，线程资源调度
void run(int dataPipeId);

// 指定端口和该端口上的dataPipeId，获取队列中的元素数量
std::size_t getInputDataCount(int inputPort, int dataPipeId);
// 将已处理完的数据push到输出Connector。特别地，如果当前element是sink element，则执行StopHandler。
common::ErrorCode pushOutputData(int outputPort, int dataPipeId, std::shared_ptr<void> data);
```

### 3.3.1 ObjectMetadata

ObjectMetadata是sophon-stream的通用数据结构，所有element中的功能都基于此结构设计。

ObjectMetadata的主要成员包括: 

```cpp
std::shared_ptr<common::Packet> mPacket; // 储存解码前信息
std::shared_ptr<common::Frame> mFrame;   // 储存解码后信息: bm_image、frame_id、EndOfStream标识等
std::shared_ptr<bmTensors> mInputBMtensors; // 当前frame经过预处理得到的inputTensor
std::shared_ptr<bmTensors> mOutputBMtensors; // 当前frame经过推理得到的outputTensor

// 嵌套的objectMetadata，储存当前图上的子结构
std::vector<std::shared_ptr<ObjectMetadata> > mSubObjectMetadatas; 
// detect相关信息，例如box坐标
std::shared_ptr<common::DetectedObjectMetadata> mDetectedObjectMetadata; 
// track相关信息，例如track_id
std::shared_ptr<common::TrackedObjectMetadata> mTrackedObjectMetadata;
```

### 3.4 Connector

Connector是在两个element之间传递数据的桥梁。一个connector的实例可以管理多个datapipe。

Connector类的主要成员如下: 

```cpp
class Connector : public ::sophon_stream::common::NoCopyable {
 public:
  
  // 获取编号为id的队列头部的数据，并将其弹出
  std::shared_ptr<void> popDataWithId(int id);
  
  // 将data push到编号为id的队列
  common::ErrorCode pushDataWithId(int id, std::shared_ptr<void> data);
 
 private:
  
  // 多个DataPipe 
  std::vector<std::shared_ptr<DataPipe>> mDataPipes;
};
```

Connector类的成员方法都由id获取某个datapipe，然后调用该datapipe的对应方法来实现。

### 3.5 DataPipe

DataPipe类是通用的阻塞队列，其成员包括: 

```cpp
class DataPipe : public ::sophon_stream::common::NoCopyable {
 public:
  
  // 向队列尾部push数据
  common::ErrorCode pushData(std::shared_ptr<void> data);

  // 获得当前队列大小
  std::size_t getSize() const;

  // 从队列头部获取并弹出数据
  std::shared_ptr<void> popData();

 private:
  
  std::deque<std::shared_ptr<void> > mDataQueue;

  // push数据的超时时间，默认设为200ms
  const std::chrono::milliseconds timeout{200};
};
```

## 4. 插件

sophon-stream中，所有算法或多媒体功能都以插件的形式存放于 sophon_stream/element/ 目录中。

sophon-stream/element/algorithm 目录是算法插件的集合，目前包括yolox、yolov5、bytetrack算法。

sophon-stream/element/multimedia 目录是多媒体插件的集合，目前包括编解码和OSD(On-Screen Display)功能。

### 4.1 algorithm

#### 4.1.1 概述

算法插件是基于SophonSDK中BMCV和BMruntime库实现的具有图像处理和推理功能的模块，包括前处理、推理、后处理三个部分。用户根据业务需求，只需要载入对应的模型，即可调用硬件启动相应的功能。

算法插件具有以下特性：

 - element每个线程都与输入connector的一个datapipe绑定。组batch发生在doWork()函数的开始，从当前线程对应的datapipe中获取数据
 - 发送数据时，保证下游element各个线程负载均衡
 - 如果两个模块之间只有模型内部参数的差异，前处理、推理、后处理的流程完全相同时，可以复用前处理和推理element
 - 支持将前处理、推理和后处理分别配置在不同的element上。如此配置的目的是充分利用cpu和tpu资源，提高算法效率

#### 4.1.2 yolox

yolox是旷视提出的目标检测算法，具有较高的性能。

yolox的配置文件形如：

```json
{
    "configure":{
        "model_path":"../data/models/BM1684X/yolox_s_int8_4b.bmodel",
        "threshold_conf":0.5,
        "threshold_nms":0.5,
        "stage":["pre"]
    },
    "shared_object":"../../../build/lib/libyolox.so",
    "device_id":0,
    "id":0,
    "name":"yolox",
    "side":"sophgo",
    "thread_number":2
}
```
配置参数的详细介绍请参见 [yolox插件介绍](../element/algorithm/yolox/README.md)

yolox demo请参考 [yolox demo](../samples/yolox/README.md)

#### 4.1.3 yolov5

yolov5是世界上最受欢迎的视觉模型，使用十分广泛。

yolov5的配置文件形如：
```json
{
    "configure":{
        "model_path":"../data/models/yolov5s_tpukernel_int8_4b.bmodel",
        "threshold_conf":0.5,
        "threshold_nms":0.5,
        "stage":["pre"],
        "use_tpu_kernel": true
    },
    "shared_object":"../../../build/lib/libyolov5.so",
    "device_id":0,
    "id":0,
    "name":"yolov5",
    "side":"sophgo",
    "thread_number":1
}
```

配置参数的详细介绍请参见 [yolov5插件介绍](../element/algorithm/yolov5/README.md)

yolov5 demo请参考 [yolov5 demo](../samples/yolov5/README.md)

#### 4.1.4 bytetrack

bytetrack是华中科技大学、香港大学和字节跳动联合提出的一个简单、快速、强大的多目标跟踪器。

其配置文件形如：
```json
{
    "configure": {
        "track_thresh": 0.5,
        "high_thresh": 0.6,
        "match_thresh": 0.7,
        "frame_rate": 30,
        "track_buffer": 30
    },
    "shared_object": "../../../build/lib/libbytetrack.so",
    "device_id": 0,
    "id": 0,
    "name": "bytetrack",
    "side": "sophgo",
    "thread_number": 2
}
```

配置参数的详细介绍请参见 [bytetrack插件介绍](../element/algorithm/bytetrack/README.md)

bytetrack demo请参考 [bytetrack demo](../samples/bytetrack/README.md)

### 4.2 multimedia

#### 4.2.1 decode

decode是sophon-stream的起始模块，起到从各种类型的输入获得ObjectMetadata，并发送往下游element的作用。

目前，DecoderElement支持的数据源类型包括: 
 - 本地视频文件
 - 本地图片文件夹
 - RTSP流
 - RTMP流

作为sophon-stream的起始模块，decode在触发任务时具有一定的特殊性。在graph构建完毕后，需要向decode element发送一个启动任务的信号，才会使decode开始工作，后续的element才有数据流入。

```cpp
nlohmann::json channel_config = yolox_json.channel_config;

channel_config["channel_id"] = channel_id;

auto channelTask = 
    std::make_shared<sophon_stream::element::decode::ChannelTask>();

channelTask->request.operation = 
    sophon_stream::element::decode::ChannelOperateRequest::ChannelOperate::START;

channelTask->request.json = channel_config.dump();

sophon_stream::common::ErrorCode errorCode = 
    engine.pushInputData(graph_id, 
                        src_id_port.first, 
                        src_id_port.second, 
                        std::static_pointer_cast<void>(channelTask));
```

可以看到，在构造channelTask时，还需要设置当前输入码流的channel_id。channel_id作为码流的标识，起到了确定ObjectMetadata在connector中流向的作用。

decode的配置文件包括以下内容: 

```json
{
  "configure": {},
  "shared_object": "../../../build/lib/libdecode.so",
  "device_id": 0,
  "id": 0,
  "name": "decode",
  "side": "sophgo",
  "thread_number": 1
}
```

配置参数的详细介绍请参见 [decode介绍](../element/multimedia/decode/README.md)

#### 4.2.2 encode

 encode一般作为sophon-stream的尾部模块使用，用于将处理后的图像信息编码为各类视频格式。

 目前，decode支持的目标类型包括: 
  - RTSP、RTMP、本地视频文件
  - H.264/H.265编码格式
  - I420、NV12等像素格式

encode的配置文件包括以下内容:

```json
{
  "configure": {
    "encode_type": "RTSP",
    "rtsp_port": "8554",
    "rtmp_port": "1935",
    "enc_fmt": "h264_bm",
    "pix_fmt": "I420"
  },
  "shared_object": "../../../build/lib/libencode.so",
  "device_id": 0,
  "id": 0,
  "name": "encode",
  "side": "sophgo",
  "thread_number": 4
}
```

配置参数的详细介绍请参见 [encode介绍](../element/multimedia/encode/README.md)

#### 4.2.3 osd

osd插件是可视化插件，目前支持目标检测、目标跟踪算法结果的可视化。

osd插件的配置文件包括:

```json
{
  "configure": {
    "osd_type": "track",
    "class_names": "../data/coco.names"
  },
  "shared_object": "../../../build/lib/libosd.so",
  "device_id": 0,
  "id": 0,
  "name": "osd",
  "side": "sophgo",
  "thread_number": 1
}
```

其中，"osd_type" 字段标识了算法类型，可以设置为 "det" 或 "track" 

配置参数的详细介绍请参见 [osd介绍](../element/multimedia/osd/README.md)

## 5. 应用程序

基于sophon-stream创建应用程序，其实是基于sophon-stream的framework和element搭建业务流水线。

在实现了基础的算法功能之后，只需要编写配置文件和相应的入口程序，就可以完成流水线的搭建。

接下来，本文以一个典型的应用程序介绍配置文件和入口程序的编写要点。

### 5.1 例程概述

一个典型的pipeline一般包括如下操作:
  - 数据源解码
  - 目标检测
  - 目标跟踪
  - 绘制跟踪结果
  - 编码输出

构建的graph如下图所示:

![dec_det_track_osd_enc](./pics/dec_det_track_osd_enc.png)

### 5.2 配置文件

首先，需要配置上图中各个element的信息。在实际配置业务时，为了保证检测算法的效率，可以将检测算法的前处理、推理和后处理三个阶段分别配置在三个element上执行。本例程中采取这种配置方式，因此，该流水线包括decode、pre_process、inference、post_process、track、osd、encode共七个element。

该例程的配置文件包括：

```bash
./config/
├── bytetrack.json                          # 跟踪
├── decode.json                             # 解码
├── encode.json                             # 编码
├── engine.json                             # graph的总体配置
├── osd.json                                # osd模块
├── yolox_bytetrack_osd_encode_demo.json    # demo的总体配置
├── yolox_infer.json                        # yolox推理
├── yolox_post.json                         # yolox后处理
└── yolox_pre.json                          # yolox预处理
```

其中，各个element的配置文件在上文对应的章节已经进行了阐述，这里对demo和graph配置文件中的内容进行说明。

yolox_bytetrack_osd_encode_demo.json 是该demo的总体配置，其形如：

```json
{
  "channels": [
    {
      "channel_id": 2,
      "url": "../data/videos/mot17_01_frcnn.mp4",
      "source_type": 0
    },
    {
      "channel_id": 3,
      "url": "../data/videos/mot17_03_frcnn.mp4",
      "source_type": 0
    }
  ],
  "engine_config_path": "../config/engine.json"
}
```

demo的配置文件包括两个属性。一是 channels ，在一个list中记录所有输入的url、channel_id和source_type信息。需要注意的是：source_type需要参考 [decode配置](../element/multimedia/decode/README.md) 准确设置。

engine.json 是当前demo程序中构造的graph信息，储存了每个graph内包含的element及element之间如何连接等信息。其形如：

```json
[
    {
        "graph_id": 0,
        "graph_name": "yolox",
        "elements": [
            {
                "element_id": 5000,
                "element_config": "../config/decode.json",
                "ports": {
                    "input": [
                        {
                            "port_id": 0,
                            "is_sink": false,
                            "is_src": true
                        }
                    ],
                    "output": [
                        {
                            "port_id": 0,
                            "is_sink": false,
                            "is_src": false
                        }
                    ]
                }
            },
            {
                "element_id": 5001,
                "element_config": "../config/yolox_pre.json",
                "ports": {
                    "input": [
                        {
                            "port_id": 0,
                            "is_sink": false,
                            "is_src": false
                        }
                    ],
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
                "dst_element_id": 5001,
                "dst_port": 0
            }
        ]
    }
]
```

其中，需要重点关注的是 "elements" 和 "connections" 部分。"elements" 是graph内所有element的列表，对于每个element，需要配置element_id、对应的配置文件路径和端口信息。同一个graph内不同的element应具有不同的element_id。element的端口包括输入和输出端口，同一种类的不同端口之间同样应该由不同的port_id区分开。每个端口都具有 "is_src" 和 "is_sink" 属性，标志着当前是否是整张graph的输入或输出端口。

一般只有decode element才会具有输入端口，decode element在一张图中只有一个。对于此element，需要在应用程序中为其发送channelTask，以启动pipeline的工作。不同的是，输出端口不要求element的类型，任何element都可以具有输出端口，具体应该参考工程需求进行配置。对于具有输出端口的element，应为其设置StopHandler，即正确处理输出数据的回调函数。

### 5.3 入口程序

对于不同的demo，其差异主要在配置文件方面，入口程序基本是一致的。

入口程序一般包括以下几个部分：

 - 解析demo的配置文件
 - 解析engine的配置文件
 - 调用engine.addGraph()，初始化所有element及其connection
 - 设置sink element的StopHandler
 - 发送channelTask，触发decode element的工作任务
 - 等候所有码流处理完毕，结束任务
 - 统计fps等信息

### 5.4 用户侧信息
