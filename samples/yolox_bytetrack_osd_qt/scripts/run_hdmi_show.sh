#!/bin/sh -x

export PATH=$PATH:/opt/bin:/bm_bin
export QTDIR=/usr/lib/aarch64-linux-gnu #qtsdk在系统上的路径
export QT_QPA_PLATFORM_PLUGIN_PATH=$QTDIR/qt5/plugins/
export QT_QPA_FONTDIR=$QTDIR/fonts
export LD_LIBRARY_PATH=$PWD/../../../build/lib/:$LD_LIBRARY_PATH
export NO_FRAMEBUFFER=1
if grep -aiE "bm1688|athena2|cv186" '/proc/device-tree/model'; then
# a2
export LD_LIBRARY_PATH=$QTDIR/qt5/lib:$LD_LIBRARY_PATH
export QT_QPA_PLATFORM=linuxfb

else
# 168x

fl2000=$(lsmod | grep fl2000 | awk '{print $1}')

echo $fl2000
if [ "$fl2000" != "fl2000" ]; then
        echo "insmod fl2000"
else
        echo "fl2000 already insmod"
fi

export QT_QPA_PLATFORM=linuxfb:fb=/dev/fl2000-0 #framebuffer驱动
export QWS_MOUSE_PROTO=/dev/input/event3
fi

cd ../../build/
./main --demo_config_path=../yolox_bytetrack_osd_qt/config/yolox_bytetrack_osd_qt_demo.json
