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

可执行文件位于sophon-stream/engine/build/，json配置文件位于sophon-stream/engine/test/usecase/json/

完成编译后，检查json配置文件的模型路径等参数，确认无误后前往build目录运行usecaseXXX即可。

stage

preelement
inferelement  
postelement


4, 8400, 85



0 29
1 0 30
1 1 0 31
1 1 1 0 32
1 1 1 1
