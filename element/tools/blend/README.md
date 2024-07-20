# sophon-stream blend element


sophon-stream blend element是sophon-stream框架中的一个插件，是一个用于图像拼接的插件。

## 1. 特性
* 图片融合模块（Image Blend）用于加速影像拼接的相关计算。对于有着不同图片输入数量的图片拼接任务，目前支持两路拼接。

## 2. 配置参数
sophon-stream dwa插件具有一些可配置的参数，可以根据需求进行设置。以下是一些常用的参数：

```json
{
  "configure": {
    "wgt1": "../dwa_blend_encode/data/wgt/c01_alpha_444p_m2__0_2240x128.bin",
    "wgt2": "../dwa_blend_encode/data/wgt/c01_beta_444p_m2__0_2240x128.bin",
    "ovlp_lx": 2112,
    "ovlp_rx": 2239,
    "src_h":2240,
    "bd_lx0": 0,
    "bd_rx0": 0,
    "bd_lx1": 0,
    "bd_rx1": 0
  },
  "shared_object": "../../build/lib/libblend.so",
  "name": "blend",
  "side": "sophgo",
  "thread_number": 1
}
```

| 参数名        | 类型   | 默认值                                                           | 说明                            |
| ------------- | ------ | ---------------------------------------------------------------- | ------------------------------- |
| wgt1          | string | 无 | 左路的权重文件 |
| wgt2          | string | 无  | 右路的权重文件 |
| src_h          | int | 无  | 右路的权重文件 |
| ovlp_lx       | int    | 无                                                            | 重叠区域左边界点x坐标           |
| ovlp_rx       | int    | 无                                                            | 重叠区域右边界点x坐标           |
| bd_lx0        | int    | 无                                                                 | 左图左侧黑边宽度                |
| bd_rx0        | int    | 无                                                                 | 左图右侧黑边宽度                |
| bd_lx1        | int    | 无                                                                 | 右图左侧黑边宽度                |
| bd_rx1        | int    | 无                                                                 | 右图右侧黑边宽度                |
| shared_object | string | "../../../build/lib/libblend.so"                                   | libdwa动态库路径                |
| name          | string | "blend"                                                    | element名称                     |
| side          | string | "sophgo"                                                         | 设备类型                        |
| thread_number | int    | 1                                                                | 启动线程数                      |


