# 使用问题及解答

[English](FAQ_EN.md) | 简体中文

#### 1. 初始化阶段报错`Can not find element maker, name: xxx`

这个原因是修改了某个element的json文件里的"name"项，这是Factory识别Element的依据，需要保持原样。


#### 2. 编译报错`fatal error: boost/version.hpp: No such file or directory`
请使用如下命令安装该库：
```bash
sudo apt-get update
sudo apt-get install libboost-all-dev
```

#### 3. 交叉编译之后在盒子上运行报错 `undefined symbol: _ZN5boost6system15system_categoryEv`

先检查boost库的版本：`dpkg -S /usr/include/boost/version.hpp` 1.71版本可以正常编译运行，较旧的版本可能报如上错误。

如果不便安装该版本boost，可以使用我们提供的[docker镜像进行编译](./HowToMake.md#使用开发镜像编译)

#### 4. 交叉编译之后在盒子上运行报错`/lib/aarch64-linux-gnu/libc.so.6: version 'GLIBC_2.33' not found`
这是由于您主机上的交叉编译工具链版本太高导致，可以通过如下命令重新安装：
```bash
sudo apt remove cpp-*-aarch64-linux-gnu
sudo apt-get install gcc-7-aarch64-linux-gnu g++-7-aarch64-linux-gnu
sudo ln -s /usr/bin/aarch64-linux-gnu-gcc-7 /usr/bin/aarch64-linux-gnu-gcc
sudo ln -s /usr/bin/aarch64-linux-gnu-g++-7 /usr/bin/aarch64-linux-gnu-g++
```

如果不便安装该版本交叉编译器，可以使用我们提供的[docker镜像进行编译](./HowToMake.md#使用开发镜像编译)

#### 5. 编译阶段报错`找不到tpu_kernel相关定义`

启用tpu_kernel相关功能需要03.01及之后的SDK，可以参考官网进行升级。

#### 6. 交叉编译之后在盒子上运行报错`error while loading shared libraries: libxxx.so: cannot open shared object file: No such file or directory`

首先检查sophon-stream/build/lib目录下是否有这个so文件，如果没有，需要从交叉编译的主机上scp到盒子；如果文件路径无误但无法找到，需要执行以下命令：

```bash
export LD_LIBRARY_PATH=path-to/sophon-stream/build/lib/:$LD_LIBRARY_PATH
```

#### 7. 编译阶段报错`找不到-lframework、-livslogger`

一般是编译路径有误，例如在element目录或samples目录编译等。需要回到sophon-stream项目目录进行编译。

#### 8. 运行报错`[BM_CHECK][error] BM_CHECK_RET fail`，详细报错如下：
```
[bmlib_memory][error] bm_alloc_gmem failed, dev_id = 0, size = 0x20
[BM_CHECK][error] BM_CHECK_RET fail /workspace/libsophon/bmlib/src/bmlib_memory.cpp: bm_malloc_device_byte_heap_mask: 705
MAT Allocate Err: dims = 2, size = [1, 8], type = 5
terminate called after throwing an instance of 'cv::Exception'
[bmlib_memory][error] bm_alloc_gmem failed, dev_id = 0, size = 0x10
  what():  OpenCV(4.1.0) /workspace/middleware-soc/bm_opencv/modules/core/src/matrix.cpp:448: error: (-215:Assertion failed) u != 0 in function 'create'

[BM_CHECK][error] BM_CHECK_RET fail /workspace/libsophon/bmlib/src/bmlib_memory.cpp: bm_malloc_device_byte_heap_mask: 705
Aborted (core dumped)
```
可能是linux文件句柄数量限制导致的，某些设备上默认句柄数量限制为1024，可尝试将其设置为20480
```
ulimit -n 20480
```

#### 9. 有的图片没有识别/检测结果

图片/视频存在少数的漏检误检是正常情况，因为原模型精度也无法达到100%。stream暂时未提供精度评估功能，观察大部分图片识别/检测结果正常即可。

#### 10. 推流失败

推流失败最常见的原因是未开启流服务器，此种情况下，终端会有如下打印：

```bash
[tcp @ 0x7f6cbe4a00] Connection to tcp://localhost:1935?tcp_nodelay=0 failed: Connection refused
[rtmp @ 0x7f6cbe4580] Cannot open connection tcp://localhost:1935?tcp_nodelay=0
```

或：

```bash
[tcp @ 0x7f40104eb0] Connection to tcp://localhost:8554?timeout=0 failed: Connection refused
```

此种情况下，请参考 [encode element](../element/multimedia/encode/README.md)，开启流服务器后重试。