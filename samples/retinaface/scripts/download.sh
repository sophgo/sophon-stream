#!/bin/bash
pip3 install dfn

res=$(dpkg -l|grep unzip)
if [ $? != 0 ];
then
    echo "Please install unzip on your system!"
    exit
fi

scripts_dir=$(dirname $(readlink -f "$0"))
# echo $scripts_dir

pushd $scripts_dir

mkdir -p ../data/images
mkdir -p ../data/videos
mkdir -p ../data/images/face



http://disk-sophgo-vip.quickconnect.to/sharing/TKlYk3tz4
# 测试集wind
python3 -m dfn --url http://219.142.246.77:65000/sharing/4ryx2MOgm
tar -xf wind.tar -C ../data/images/
rm wind.tar

# 测试集WIDER FACE
python3 -m dfn --url http://219.142.246.77:65000/sharing/4ryx2MOgm
tar -xf WIDERVAL.tar -C ../data/images/
rm WIDERVAL.tar

## 单张图片与视频
python3 -m dfn --url http://219.142.246.77:65000/sharing/BaQm9vdpp
tar -xf test.tar -C ../data
mv ../data/images/face0*.jpg ../data/images/face
rm test.tar

# models
python3 -m dfn --url http://219.142.246.77:65000/sharing/BmC4MEfEl
unzip models.zip -d ../data/
rm models.zip


popd
