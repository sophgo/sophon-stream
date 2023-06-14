# 编译指南
- [编译指南](#编译指南)
  - [x86/arm PCIe平台](#x86arm-pcie平台)
  - [SoC平台](#soc平台)
  - [编译结果](#编译结果)

## x86/arm PCIe平台
```bash
mkdir build
cd build
cmake ../ -DCMAKE_BUILD_TYPE=Debug -DTARGET_ARCH=pcie
make -j4
```

## SoC平台
通常在x86主机上交叉编译程序，您需要在x86主机上使用SOPHON SDK搭建交叉编译环境，将程序所依赖的头文件和库文件打包至sophon_sdk_soc目录中。您可以下载SOPHON SDK自行打包，也可以下载我们打包好的[文件](http://disk-sophgo-vip.quickconnect.cn/sharing/vmOeAUrI9)。
```bash
mkdir build
cd build
cmake ../ -DCMAKE_BUILD_TYPE=Debug -DTARGET_ARCH=soc -DSOPHON_SDK_SOC=/path/to/sophon_sdk_soc
make -j4
```

## 编译结果
1.`framework`和`element`会在`build/lib`中生成动态链接库

2.`samples`会在相应例程文件夹下的`build`文件夹生成可执行文件，如`samples/yolov5`会在`samples/yolov5/build`下生成`yolov5_demo`可执行文件

对于PCIe平台，可以直接在PCIe平台上运行测试；对于SoC平台，需将交叉编译生成的动态链接库和可执行文件拷贝到SoC平台中测试。