# Calibration

## 目录
- [Calibration](#calibration)
  - [目录](#目录)
  - [整体标定流程](#整体标定流程)
  - [环境准备](#环境准备)
  - [标定工具说明](#标定工具说明)
  - [单目标定](#单目标定)
  - [双目标定](#双目标定)

## 整体标定流程
整体的标定流程分为两部分，首先进行单目的标定，然后进行双目转共面实现左右摄像头对应的特征点在统一水平线上。
[![5a279cd06e55ca7330dbd606924d7118.png](https://s1.imagehub.cc/images/2024/02/20/5a279cd06e55ca7330dbd606924d7118.png)](https://www.imagehub.cc/image/1hHShT)

## 环境准备
（1）安装04a10驱动，参照[使用手册](README.md)

（2）加载isp参数，参照[使用手册](README.md)

（3）进入root账户，打开CviIspTool.sh，执行以下命令：
```bash
sudo -s
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

## 标定工具说明
（1）点击菜单栏的calibration，选择下方的Distrotion calibration。
[![9374cc476fafc40500f6315d48b5cfb7.png](https://s1.imagehub.cc/images/2024/02/20/9374cc476fafc40500f6315d48b5cfb7.png)](https://www.imagehub.cc/image/1hJd5O)

（2）参与矫正的图片来源有2种，分别是：从板端获取和从电脑本地加载。从板端获取和从电脑本地加载标定图片的唯一区别是：当点击“capture”按键时，则从板端获取。不点击“capture”按键而点击“Import Image”按键时，则从电脑本地获取。如下图：
[![7a88a2419a9df821bbee3711b0aa746e.png](https://s1.imagehub.cc/images/2024/02/20/7a88a2419a9df821bbee3711b0aa746e.png)](https://www.imagehub.cc/image/1hJbm6)

下面以本地加载标定的方式来说明单目标定，双目标定流程。

## 单目标定
（1）先选择类型是左目sensor还是右目sensor（左右目是哪一个sensor，根据情况由你自己来决定），比如此时选择类型为左目sensor，即Left eye，如下图：
[![f8395681a1f448d87de2840068793b7b.png](https://s1.imagehub.cc/images/2024/02/20/f8395681a1f448d87de2840068793b7b.png)](https://www.imagehub.cc/image/1hJMRJ)

（2）通过“Input”窗口的“Import Image”按键导入左目sensor的本地标定图片目录，导入后的结果如下图：
[![e000000ddd2ced96d1d09cf456c0aec0.png](https://s1.imagehub.cc/images/2024/02/20/e000000ddd2ced96d1d09cf456c0aec0.png)](https://www.imagehub.cc/image/1hJgVe)

特别说明：对于本地左右单目，及双目标定图片所在目录，其目录只能存放一种类型名的图片，即本地左目目录下的图片名均是以“left_xx”、 右目则为“right_xx”、双目则为“stereo_xx”为前缀的jpg图片，否则会导入失败。具体名字形式如下图：
[![74554bab1e926da434ef1df6a9320543.png](https://s1.imagehub.cc/images/2024/02/20/74554bab1e926da434ef1df6a9320543.png)](https://www.imagehub.cc/image/1hJlts)

（3）图片导入后，需要设定标定前的参数选项，具体是下面所提到的参数。下图中的“Pattern Info”参数设置，取决于所拍标定板的类型。
[![da38132aee740ea4341dbce480c49fbc.png](https://s1.imagehub.cc/images/2024/02/20/da38132aee740ea4341dbce480c49fbc.png)](https://www.imagehub.cc/image/1hJBKr)

下图中的“Image Calibrate Param”参数设置，取决于sensor镜头和实际需求的opencv参数选项。
[![e414d59992d0da76796ac4fc654390e7.png](https://s1.imagehub.cc/images/2024/02/20/e414d59992d0da76796ac4fc654390e7.png)](https://www.imagehub.cc/image/1hJtRS)

上两图的每个参数的详细设置说明，也可将鼠标移动到对应的参数名位置，停留片刻后，其会自动弹出对应说明。

（4）在启动标定之前，需选择标定时产生数据存放的位置，否则会保存到默认路径下，点击“Save Root Path”按键即可选择。如下图：

[![a362205c9e68d300cb048c52ba464843.png](https://s1.imagehub.cc/images/2024/02/20/a362205c9e68d300cb048c52ba464843.png)](https://www.imagehub.cc/image/1hJGtq)

（5）以上设置完成后，即可点击“Start calibration”按键进行标定，待标定完成后，右侧画面会自动跳转到标定后的效果界面。即“Output”窗口。如下图：
[![10aa57e34d98d941f740dcc1d6d2bd6d.png](https://s1.imagehub.cc/images/2024/02/20/10aa57e34d98d941f740dcc1d6d2bd6d.png)](https://www.imagehub.cc/image/1hJRA0)

（6）标定完成后，如果要保存标定后的结果图片，只需要点击上图右上角的“Save”按键即可完成保存，保存的位置，皆在选择的目录下。

标定后产生的数据文件分布情况如下图所示，以左目标定目录为例：
[![be12fa00ead877c237aaec3080b79bb4.png](https://s1.imagehub.cc/images/2024/02/20/be12fa00ead877c237aaec3080b79bb4.png)](https://www.imagehub.cc/image/1hJJgo)

右目标定流程，同左目流程，但要按照下图先选择“Right eye”。如下图。之后通过“Import Image”按键导入参与右目标定的图片。
[![215a9826ca04fa9b96d777c44c77d0b0.png](https://s1.imagehub.cc/images/2024/02/20/215a9826ca04fa9b96d777c44c77d0b0.png)](https://www.imagehub.cc/image/1hJUhb)

## 双目标定

（1）“Stereo eye”标定流程也同左目标定流程，需先选择“Stereo eye”，如下图，之后通过“Import Image”按键导入参与“Stereo eye”标定的图片。

[![4de6809f96ffcd02ae043348b628f69a.png](https://s1.imagehub.cc/images/2024/02/20/4de6809f96ffcd02ae043348b628f69a.png)](https://www.imagehub.cc/image/1hJQAa)

特别地，在进行“Stereo eye”标定前，需要先将左目和右目都标定完后或者已经存在由左右目标定时产生的标定数据，方可进行“Stereo eye”标定，否则无法进行此标定，同时工具会有相关警告提示。

（2）标定完成后，点击右上方的save，会将矫正后的图片全部存放在指定目录的undistort下。
[![5de7dedd7b980d351fd477d4657468d7.png](https://s1.imagehub.cc/images/2024/02/20/5de7dedd7b980d351fd477d4657468d7.png)](https://www.imagehub.cc/image/1hJunO)


下面是经过双目标定后生成的gridinfo、矫正图片以及矫正参数。

[![397bac344e58a2408245bf92fd9d3187.png](https://s1.imagehub.cc/images/2024/02/20/397bac344e58a2408245bf92fd9d3187.png)](https://www.imagehub.cc/image/1hJvKA)

下面的gridinfo文件即dwa的输入文件。
[![eb07276c3864ffe71721ba3eba8d2d7f.png](https://s1.imagehub.cc/images/2024/02/20/eb07276c3864ffe71721ba3eba8d2d7f.png)](https://www.imagehub.cc/image/1hJ9zk)

打开cameraParams.yaml，可以看到每张图重投影后的误差情况。
[![cae9075bd1c01d461767ac3e55fa330c.png](https://s1.imagehub.cc/images/2024/02/20/cae9075bd1c01d461767ac3e55fa330c.png)](https://www.imagehub.cc/image/1hJVw6)

至此，完成了双目相机的标定。