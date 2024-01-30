# sophon-stream ive element


sophon-stream ive element是sophon-stream框架中的一个插件，是一个用于深度估计后处理染色的插件。

## 1. 特性
* 该 API 使用ive硬件资源, 创建 Map（映射赋值）任务，对源图像中的每个像素，查找 Map 查找表中的值，赋予目标图像相应像素查找表中的值， 支持 U8->U8, U8->U16, U8->S16 三种模式的映射。

## 2. 配置参数
sophon-stream ive插件具有一些可配置的参数，可以根据需求进行设置。以下是一些常用的参数：

```json
{
  "configure": {
    "ive_mapy": "../dwa_dpu_encode/data/maps/mapY.txt",
    "ive_mapu": "../dwa_dpu_encode/data/maps/mapU.txt",
    "ive_mapv": "../dwa_dpu_encode/data/maps/mapV.txt",
    "is_ive":false
  },
  "shared_object": "../../build/lib/libdpu.so",
  "name": "dpu",
  "side": "sophgo",
  "thread_number": 1
}
```

| 参数名        | 类型   | 默认值                                 | 说明                                 |
| ------------- | ------ | -------------------------------------- | ------------------------------------ |
| ive_mapy      | string | "../dwa_dpu_encode/data/maps/mapY.txt" | 给DPU结果进行染色的Y通道map文件      |
| ive_mapu      | string | "../dwa_dpu_encode/data/maps/mapU.txt" | 给DPU结果进行染色的U通道map文件      |
| ive_mapv      | string | "../dwa_dpu_encode/data/maps/mapV.txt" | 给DPU结果进行染色的V通道map文件      |
| is_ive        | bool   | true                                   | 选择是否对DPU结果进行染色            |
| shared_object | string | "../../../build/lib/libive.so"         | libive动态库路径                     |
| name          | string | "distributor"                          | element名称                          |
| side          | string | "sophgo"                               | 设备类型                             |
| thread_number | int    | 1                                      | 启动线程数                           |


