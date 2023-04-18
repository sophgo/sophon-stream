编译指南

pcie: 运行scripts目录下的build.sh, 需要指定Debug或Release模式
```
./build.sh Debug
./build.sh Release
```

soc交叉编译: 参考SophonSDK指南准备交叉编译环境, 运行scripts目录下的build_arm.sh, 需要指定Debug或Release模式，并且指定soc-sdk路径
```
./build_arm.sh Debug ~/RC2/soc-sdk
```