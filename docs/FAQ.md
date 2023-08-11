# 使用问题及解答

## 1. 初始化阶段报错Can not find element maker, name: xxx

这个原因是修改了某个element的json文件里的"name"项，这是Factory识别Element的依据，需要保持原样。

## 2. 交叉编译之后在盒子上运行报错 undefined symbol: _ZN5boost6system15system_categoryEv

先检查boost库的版本：```dpkg -S /usr/include/boost/version.hpp``` 1.71版本可以正常编译运行，较旧的版本可能报如上错误。

如果不便安装该版本boost，可以使用我们提供的docker镜像进行编译：

```bash
docker pull yifan996/stream_dev
docker run --privileged --name stream_dev -v $PWD:/workspace -it yifan996/stream_dev
```

## 3. 编译阶段提示找不到tpu_kernel相关定义

启用tpu_kernel相关功能需要03.01及之后的SDK，可以参考官网进行升级。

## 4. 交叉编译之后在盒子上运行报错 error while loading shared libraries: libxxx.so: cannot open shared object file: No such file or directory

首先检查sophon-stream/build/lib目录下是否有这个so文件，如果没有，需要从交叉编译的主机上scp到盒子；如果文件路径无误但无法找到，需要执行以下命令：

```bash
export LD_LIBRARY_PATH=path-to/sophon-stream/build/lib/:$LD_LIBRARY_PATH
```