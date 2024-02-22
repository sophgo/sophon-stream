# Calibration

## 目录
- [Calibration](#calibration)
  - [目录](#目录)
  - [环境准备](#环境准备)
  - [拍摄照片](#拍摄照片)
  - [调参说明](#调参说明)
  - [参数文件更新](#参数文件更新)


## 环境准备
（1）安装04e10驱动，参照[使用手册](README.md
（2）加载isp参数，参照[使用手册](README.md)

（3）打开CviIspTool.sh，执行以下命令：
```bash
cd /opt/sophon/sophon-soc-libisp_1.0.0/bin
```
将cfg.json中的"dev-num":改为2，保证可以读取两路的视频。
```bash
./CviIspTool.sh
```

（4）在window打开pqtool工具，输入主机ip，即可连接。下载链接如下：
```bash
python3 -m dfss --url=open@sophgo.com:/sophon-stream/dwa_dpu_encode/CviPQtool_20240111.zip
```
（5）在window打开鱼眼拼接调参工具，下载链接如下：
```bash
python3 -m dfss --url=open@sophgo.com:/sophon-stream/dwa_blend_encode/stitchtool_circular_fisheye_v240103.7.zip
```

## 拍摄照片

（1）分别用两个相机各拍摄一张具有重叠区域的照片（打开CviPQTool工具，点击Preview，再点击Get Single Image和Save来拍摄并保存图像），分别命名为l和r保存在L和R文件夹下（l为拼接图中的左图，r为拼接图中的右图），手动旋转为正方向。
[![9b8c262d0b85997994ef5dc685de28fa.png](https://s1.imagehub.cc/images/2024/02/20/9b8c262d0b85997994ef5dc685de28fa.png)](https://www.imagehub.cc/image/1hKM2j)

下面是两张样图：

[![794a97b2ef03d46232a9602bb8dbd1cf.png](https://s1.imagehub.cc/images/2024/02/20/794a97b2ef03d46232a9602bb8dbd1cf.png)](https://www.imagehub.cc/image/1hKweg)[![34d47fb2fdb2f810516df641a0630803.png](https://s1.imagehub.cc/images/2024/02/20/34d47fb2fdb2f810516df641a0630803.png)](https://www.imagehub.cc/image/1hKeso)

（2）或者用vlc++的截图功能，分别截取左右摄像头的图像，下载链接如下：
```bash
python3 -m dfss --url=open@sophgo.com:/sophon-stream/dwa_blend_encode/vlc++.zip
```
在vlc++中输入两路视频流地址，点击右上角的截图按钮。
[![c76b8a584e600c2c7ccfc7d54ba983ba.png](https://s1.imagehub.cc/images/2024/02/20/c76b8a584e600c2c7ccfc7d54ba983ba.png)](https://www.imagehub.cc/image/1hKWiO)

## 调参说明
（1）运行下列命令（鱼眼拼接调参工具目录下readme.txt中step1命令）来将鱼眼图像展开为平面图像（命令中图像后缀要与第1步保存的图片后缀一致）(在stitchtool_circular_fisheye_xxx目录下，打开cmd)。
[![23ee60cca7190a5f36700e4edda3a106.png](https://s1.imagehub.cc/images/2024/02/20/23ee60cca7190a5f36700e4edda3a106.png)](https://www.imagehub.cc/image/1hKEH6)

命令说明如下：
```bash
gen_gridinfo.exe mode data_folder/ image_path do_flip map_y_shift theta_x theta_y theta_z

data_folder：放置_mapx.bin及_mapy.bin的地方
image_path：使用影像档的绝对路径
map_x_shift：map矩阵要移动的pixel
map_y_shift：map矩阵要移动的pixel
theta_x, theta_y, theta_z: three angles in degree
```

如：
```bash
gen_gridinfo.exe 0 ./231204/L ./231204/L/l.bmp 0 0 0 0 0
gen_gridinfo.exe 0 ./231204/R ./231204/R/r.bmp 0 0 0 0 0
```
通过移动窗口的滑杆选择合适的单位视球球心和半径，以此将鱼眼图像还原到单位球面上，从而投影为平面图像（注意：找圆时尽量往内圈一点点，不要圈到边缘的黑边）。

[![9f8fc6298cd9bf992d5aa9a42ccb0e70.png](https://s1.imagehub.cc/images/2024/02/20/9f8fc6298cd9bf992d5aa9a42ccb0e70.png)](https://www.imagehub.cc/image/1hKRbJ)[![3e26f31a6233ecec4ad5c6f677aa7466.png](https://s1.imagehub.cc/images/2024/02/20/3e26f31a6233ecec4ad5c6f677aa7466.png)](https://www.imagehub.cc/image/1hKcEe)

（2）运行下列命令（鱼眼拼接调参工具目录下readme.txt中step2命令）产生DWA要使用的gridinfo文件及拼接要用的类似DWA的输出结果 dc_src.jpg （这个命令一般无需改动）：
命令说明如下：
```bash
gen_gridinfo.exe mode data_folder dc_src.jpg_path
```
如：
```bash
gen_gridinfo.exe 2 ./231204/L ./231204/L/dc_src.jpg
gen_gridinfo.exe 2 ./231204/R ./231204/R/dc_src.jpg
```

(3) 运行下列命令（鱼眼拼接调参工具目录下readme.txt中step3命令）按照默认参数将两张图片拼接得到拼接图（3/c01_stitch_444p_c2.png）。
命令说明如下：
```bash
gen_gridinfo.exe mode left_image_path right_image_path 融合区起始位置 融合区宽度
```
如：
```bash
gen_gridinfo.exe 1 ./231204/3/ ./231204/L/mask_result.png ./231204/R/mask_result.png 2144 32
```
注意：（注意：最后两个数值需要被32整除，相加不需要等于2240，融合时，左图右边缘会直接取右图，如示例中，2240 – 2144 – 32 = 64，即左图右边缘64pixel直接取右图）。
[![177b45c44ae6dad8915dcd4314ff70a1.png](https://s1.imagehub.cc/images/2024/02/20/177b45c44ae6dad8915dcd4314ff70a1.png)](https://www.imagehub.cc/image/1hKsuZ)

（4）根据拼接图来调整拼接参数，重复234步尝试找到最佳拼接效果

注意：一般固定右图不动，通过调节左图位置来对齐。

X方向（map_x_shift）：map矩阵要移动的pixel，正数代表向右移动图片，负数代表向左移动图片，以此可以对图片在X方向上的位置进行微调。

Y方向（map_y_shift）：map矩阵要移动的pixel，正数代表向下移动图片，负数代表向上移动图片，以此可以对图片在Y方向上的位置进行微调。
如：
```bash
gen_gridinfo.exe 0 ./231204/L ./231204/L/l.bmp -16 6 0 0 0
```
该命令的含义是：左图整体向左移动16个pixel，向下移动6个pixel。
上述命令中最后三个数字分别可以调整图像theta_x 、theta_y 、theta_z三个角度，其效果分别如下图所示(一般不使用这三个参数，默认设置为0)。下面是效果演示：
首先，原图为：
[![bcd976c4c80f5271526e07216cc9df94.png](https://s1.imagehub.cc/images/2024/02/20/bcd976c4c80f5271526e07216cc9df94.png)](https://www.imagehub.cc/image/1hKJih)

设置theta_x = 3，theta_y = 0，theta_z = 0，效果如下：
[![2f558fac322adcf054bf5120c8f947b2.png](https://s1.imagehub.cc/images/2024/02/20/2f558fac322adcf054bf5120c8f947b2.png)](https://www.imagehub.cc/image/1hKilr)

设置theta_x = 0，theta_y = 3，theta_z = 0，效果如下：
[![4045c79506e89c80beb32dfd6dd7e711.png](https://s1.imagehub.cc/images/2024/02/20/4045c79506e89c80beb32dfd6dd7e711.png)](https://www.imagehub.cc/image/1hKkHv)

设置theta_x = 0，theta_y = 0，theta_z = 3，效果如下：
[![1229d68bdba61af016dda4264c710342.png](https://s1.imagehub.cc/images/2024/02/20/1229d68bdba61af016dda4264c710342.png)](https://www.imagehub.cc/image/1hKyot)

注意：theta_x 表示绕横轴旋转，theta_y 表示绕纵轴旋转，theta_z表示垂直纸面旋转。

(5)下列命令中的两个参数可以调整两张图像X方向的拼接融合区域大小，其中2144为左图起始拼接位置，32为融合区域大小。在默认参数基础上以32为最小调节单位来进行拼接时X方向上的调整，选择效果最好的拼接图作为最终结果。
```bash
gen_gridinfo.exe 1 ./231204/3/ ./231204/L/mask_result.png ./231204/R/mask_result.png 2144 32
```
调整后的拼接图如下图所示:

[![0e6aeae69a2ea2c4f2e7b8906a3faee1.png](https://s1.imagehub.cc/images/2024/02/20/0e6aeae69a2ea2c4f2e7b8906a3faee1.png)](https://www.imagehub.cc/image/1hKQbS)

(7)拼接完成后保存以下文件（alpha、beta权重文件以及静态拼接图像在3文件夹中，Lgrid_info和Rgrid_info文件在231204文件中）。

[![07bd58bc0789514a0f96d43a39683a1c.png](https://s1.imagehub.cc/images/2024/02/20/07bd58bc0789514a0f96d43a39683a1c.png)](https://www.imagehub.cc/image/1hKaEL)


## 参数文件更新
将生成的权重文件和girdinfo文件替换sophon-stream/sample/dwa_blend_encode/data下相应的文件，并修改对应的dwa和blend的json文件即可（修改说明见对应插件的README）。

[![cf7b8955fad073f099fff6c4227e71ae.png](https://s1.imagehub.cc/images/2024/02/20/cf7b8955fad073f099fff6c4227e71ae.png)](https://www.imagehub.cc/image/1hKuvB)