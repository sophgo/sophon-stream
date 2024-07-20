# sophon-stream dwa element


sophon-stream dwa element是sophon-stream框架中的一个插件，是一个用于鱼眼展开和镜头畸变矫正的插件。

## 1. 特性
* 该插件主要包含dwa_fisheye和dwa_gdc功能。
* （1）畸变仿射(DWA)模块的鱼眼畸变校正功能，通过配置校正参数获取适当的校正模型来消除鱼眼镜头造成的图像畸变，从而使弯曲的图像呈现出人眼能够感受到的更真实的形式。
* （2）去畸变仿射(DWA)模块的几何畸变校正功能，通过校正镜头引起的图像畸变（针对桶形畸变 (Barrel Distortion) 及枕形畸变 (Pincushion Distortion) ），使图像中的直线变得更加准确和几何正确，提高图像的质量和可视化效果。其中，提供两种畸变校正的方式供用户选择，分别为：1. 用户根据图像畸变的类型及校正强度输入配置参数列表对图像进行校正；2. 用户使用 Grid_Info(输入输出图像坐标映射关系描述)文件校正图像，以获得更好的图像校正效果。

## 2. 配置参数
sophon-stream dwa插件具有一些可配置的参数，可以根据需求进行设置。以下是一些常用的参数：

```json
{
    "configure": {
      "is_gray":true,
      "is_resize":true,
      "dst_h":1080,
      "dst_w":1920,
      "resize_h":1920,
      "resize_w":1920,
      "dwa_mode":"DWA_GDC_MODE",
      "use_grid": true,
      "grid_name": "../dwa_dpu_encode/data/gridinfo/rrr.dat",
      "grid_size":183120
    },
    "shared_object": "../../build/lib/libdwa.so",
    "name": "dwa",
    "side": "sophgo",
    "thread_number": 1
  }
```

| 参数名        | 类型   | 默认值                                    | 说明                                                             |
| ------------- | ------ | ----------------------------------------- | ---------------------------------------------------------------- |
| is_gray       | bool   | false                                      | 选择是否转换为灰度图，可供下一个插件的特殊格式需要               |
| is_resize     | bool   | false                                      | 选择是否缩放图像大小                                             |
| dst_h         | int    | 无                                      | 输出图像的高                                                 |
| dst_w         | int    | 无                                      | 输出图像的宽
| resize_h         | int    | 无                                      | dwa输入图像的高                                                 |
| resize_h         | int    | 无                                      | dwa输入图像的宽                                                 |
| dwa_mode      | string | 无                              | 选择使用鱼眼展开(DWA_FISHEYE_MODE)还是镜头畸变矫正(DWA_GDC_MODE) |
| use_grid      | bool   | 无                                      | 选择是否使用gridinfo进行畸变矫正                                 |
| grid_name     | string | 无 | 选择使用gridinfo的路径                                           |
| grid_size     | int    | 无                                   | gridinfo的文件大小                                               |
| shared_object | string | "../../../build/lib/libdwa.so"            | libdwa动态库路径                                                 |
| name          | string | "dwa"                             | element名称                                                      |
| side          | string | "sophgo"                                  | 设备类型                                                         |
| thread_number | int    | 1                                         | 启动线程数                                                       |


