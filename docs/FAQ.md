# 使用问题及解答

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