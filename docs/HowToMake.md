# 编译指南

[English](HowToMake_EN.md) | 简体中文

- [编译指南](#编译指南)
  - [使用开发镜像编译](#使用开发镜像编译)
  - [x86/arm PCIe平台](#x86arm-pcie平台)
  - [SoC平台](#soc平台)
  - [编译结果](#编译结果)

* 需要注意，编译需要在sophon-stream目录下进行。

## 使用开发镜像编译

* 原则上stream编译不强依赖docker镜像；
* 如果您使用的主机部分环境不兼容，且不方便更改本机环境，可以使用我们提供的docker镜像进行编译；
* 请注意，不要将下文的stream_dev镜像和用于模型编译的tpuc_dev镜像混用。

通过dfss下载：
```bash
pip3 install dfss
python3 -m dfss --url=open@sophgo.com:/sophon-stream/docker/stream_dev.tar
```

如果是首次使用Docker, 可执行下述命令进行安装和配置(仅首次执行):
```bash
sudo apt install docker.io
sudo systemctl start docker
sudo systemctl enable docker
sudo groupadd docker
sudo usermod -aG docker $USER
newgrp docker
```

在下载好的镜像目录中加载镜像
```bash
docker load -i stream_dev.tar
```
可以通过`docker images`查看加载好的镜像，默认为stream_dev:latest

创建容器
```bash
docker run --privileged --name stream_dev -v $PWD:/workspace  -it stream_dev:latest
# stream_dev只是举个名字的例子, 请指定成自己想要的容器的名字
```
容器中的`workspace`目录会挂载到您运行`docker run`时所在的宿主机目录，您可以在此容器中编译项目


## x86/arm PCIe平台
```bash
mkdir build
cd build
cmake ..
make -j4
```

## SoC平台
通常在x86主机上交叉编译程序，您需要在x86主机上使用SOPHON SDK搭建交叉编译环境，将程序所依赖的头文件和库文件打包至sophon_sdk_soc目录中。您可以下载SOPHON SDK自行打包，也可以下载我们打包好的文件(根据您的SOC环境选择一个即可)。

下面的三个文件，分别对应官网BM1684/BM1684X SDK的v23.03.01、v23.05.01、v23.07.01版本。
```bash
pip3 install dfss
python3 -m dfss --url=open@sophgo.com:/sophon-stream/soc-sdk/soc0301.tar.gz
python3 -m dfss --url=open@sophgo.com:/sophon-stream/soc-sdk/soc0501.tar.gz
python3 -m dfss --url=open@sophgo.com:/sophon-stream/soc-sdk/soc0701.tar.gz
```

下面两个文件，分别对应BM1688 SDK的1.2、1.3、1.4版本。
```bash
pip3 install dfss
python3 -m dfss --url=open@sophgo.com:/sophon-stream/soc-sdk/1688_1.2.tar.gz
python3 -m dfss --url=open@sophgo.com:/sophon-stream/soc-sdk/1688_1.3.tar.gz
python3 -m dfss --url=open@sophgo.com:/sophon-stream/soc-sdk/1688_1.4.tar.gz
```

交叉编译时，`SOPHON_SDK_SOC`需要填写绝对路径

```bash
mkdir build
cd build
cmake ../ -DTARGET_ARCH=soc -DSOPHON_SDK_SOC=/path/to/sophon_sdk_soc
make -j4
```

如果您需要的SDK版本上文未提供，需要自己打包soc-sdk，可以参考以下流程进行打包。需要注意的是，对于SE9设备，下列命令中的sophon-mw目录名或压缩包文件名应改为sophon-media。

 1. 解压SDK目录下，sophon-img包里的libsophon_soc_<x.y.z>_aarch64.tar.gz，将lib和include的所有内容分别拷贝到您的soc-sdk目录
 ```bash
 cd sophon-img_<date>_<hash>
# 创建依赖文件的根目录
mkdir -p soc-sdk
# 解压sophon-img release包里的libsophon_soc_${x.y.z}_aarch64.tar.gz，其中x.y.z为版本号
tar -zxf libsophon_soc_<x.y.z>_aarch64.tar.gz
# 将相关的库目录和头文件目录拷贝到依赖文件根目录下
cp -rf libsophon_soc_<x.y.z>_aarch64/opt/sophon/libsophon-<x.y.z>/lib ${soc-sdk}
cp -rf libsophon_soc_<x.y.z>_aarch64/opt/sophon/libsophon-<x.y.z>/include ${soc-sdk}
 ```
 2. 解压sophon-mw包里的sophon-mw-soc_<x.y.z>_aarch64.tar.gz，将sophon-mw下lib和include的所有内容拷贝到您的soc-sdk目录。
 ```bash
 cd sophon-mw_<date>_<hash>
# 解压sophon-mw包里的sophon-mw-soc_<x.y.z>_aarch64.tar.gz，其中x.y.z为版本号
tar -zxf sophon-mw-soc_<x.y.z>_aarch64.tar.gz
# 将ffmpeg和opencv的库目录和头文件目录拷贝到依赖文件根目录下
cp -rf sophon-mw-soc_<x.y.z>_aarch64/opt/sophon/sophon-ffmpeg_<x.y.z>/lib ${soc-sdk}
cp -rf sophon-mw-soc_<x.y.z>_aarch64/opt/sophon/sophon-ffmpeg_<x.y.z>/include ${soc-sdk}
cp -rf sophon-mw-soc_<x.y.z>_aarch64/opt/sophon/sophon-opencv_<x.y.z>/lib ${soc-sdk}
cp -rf sophon-mw-soc_<x.y.z>_aarch64/opt/sophon/sophon-opencv_<x.y.z>/include ${soc-sdk}
 ```

## 编译结果
1.`framework`和`element`会在`build/lib`中生成动态链接库

2.`samples`会在`samples/build`文件夹生成可执行文件

对于PCIe平台，可以直接在PCIe平台上运行测试；对于SoC平台，需将交叉编译生成的动态链接库和可执行文件拷贝到SoC平台中测试。

3.交叉编译完成后，将编译结果scp到盒子上时，需要保证目录结构不变。

可以使用如下命令直接将主机上的stream目录拷贝到盒子上：

```bash
scp -r ./sophon-stream linaro@<your ip>:<your path>
```

其中，盒子的ip和文件目录您可以根据实际情况进行设置。