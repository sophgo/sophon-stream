# sophon-stream dpu element


sophon-stream dpu element是sophon-stream框架中的一个插件，是一个用于深度估计的插件。

## 1. 特性
* 该API 使用 DPU 硬件资源, 实现半全局块匹配算法 SGBM(Semi-Global Block Maching) 跟 快速全局平滑算法FGS(Fast Global Smoothing)。

## 2. 配置参数
sophon-stream dpu插件具有一些可配置的参数，可以根据需求进行设置。以下是一些常用的参数：

```json
{
  "configure": {
    "dpu_type": "DPU_SGBM",
    "dpu_mode": "DPU_SGBM_MUX0"
  },
  "shared_object": "../../build/lib/libdpu.so",
  "name": "dpu",
  "side": "sophgo",
  "thread_number": 2
}

```

| 参数名        | 类型   | 默认值                         | 说明                                 |
| ------------- | ------ | ------------------------------ | ------------------------------------ |
| dpu_type      | string | DPU_SGBM                       | 选择DPU_SGBM还是DPU_ONLINE(SGBM+FGS) |
| dpu_mode      | string | DPU_SGBM_MUX0                  | 选择是dpu_mode            |
| shared_object | string | "../../../build/lib/libdpu.so" | libdpu动态库路径                     |
| name          | string | "distributor"                  | element名称                          |
| side          | string | "sophgo"                       | 设备类型                             |
| thread_number | int    | 1                              | 启动线程数                           |


*参数说明：
每种dpu_type对应一种dpu_mode，dpu_mode的取值范围如下：*

- DPU_ONLINE
    * - DPU_ONLINE_MUX0
      - 该模式下，使用FGS处理左图和右图，输出一张8bit视差图（也可用于图像的降噪，类似于引导滤波）。
    * - DPU_ONLINE_MUX1
      - 该模式下，使用SGBM、FGS处理左图和右图，输出一张16bit深度图。
    * - DPU_ONLINE_MUX2
      - 该模式下，单独使用SGBM处理左图和右图，输出一张16bit深度图。

- DPU_SGBM
    * - DPU_SGBM_MUX0
      - 使用SGBM处理左图和右图，输出一张没有经过后处理的8bit视差图。
    * - DPU_SGBM_MUX1
      - 使用SGBM处理左图和右图，输出一张经过后处理的16bit视差图。
    * - DPU_SGBM_MUX2
      - 使用SGBM处理左图和右图，输出一张经过后处理的8bit视差图。
- DPU_FGS
    * - DPU_FGS_MUX0
      - 使用FGS处理左图和右图，输出一张8bit视差图（也可用于图像的降噪，类似于引导滤波）。
    * - DPU_FGS_MUX1
      - 使用FGS处理左图和右图，输出一张16bit深度图。