编译指南

pcie
```
mkdir build
cd build
cmake ../ -DCMAKE_BUILD_TYPE=Debug -DTARGET_ARCH=pcie  
make -j4
```

soc
交叉编译: 参考SophonSDK指南准备交叉编译环境
```
mkdir build
cd build
cmake ../ -DCMAKE_BUILD_TYPE=Debug -DTARGET_ARCH=soc -DSOPHON_SDK_SOC=/home/czhang/softwares/soc-sdk
make -j4
```

`framework`和`element`会在build/lib中生成动态链接库
`samples`会在相应例程文件夹下的`build`文件夹生成可执行文件，如`samples/yolov5`会在`samples/yolov5/build`下生成`usecase_yolov5`可执行文件