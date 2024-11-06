#!/bin/bash
pip3 install dfss -i https://pypi.tuna.tsinghua.edu.cn/simple --upgrade
scripts_dir=$(dirname $(readlink -f "$0"))

pushd $scripts_dir

if [ ! -d "../data" ]; 
then
    mkdir ../data
fi

pushd ../data
python3 -m dfss --url=open@sophgo.com:sophon-stream/yolov8_obb/dotav1.names
python3 -m dfss --url=open@sophgo.com:sophon-stream/yolov8_obb/test.mp4
if [ ! -d "videos" ]; 
then
    mkdir videos
fi
mv test.mp4 videos

if [ ! -d "models" ]; 
then
    mkdir models
    pushd models
    python3 -m dfss --url=open@sophgo.com:sophon-demo/YOLOv8_obb/models/BM1684X.tar.gz
    tar xvf BM1684X.tar.gz && rm BM1684X.tar.gz
    python3 -m dfss --url=open@sophgo.com:sophon-demo/YOLOv8_obb/models/BM1688.tar.gz
    tar xvf BM1688.tar.gz && rm BM1688.tar.gz
    python3 -m dfss --url=open@sophgo.com:sophon-demo/YOLOv8_obb/models/CV186X.tar.gz
    tar xvf CV186X.tar.gz && rm CV186X.tar.gz
    popd
    echo "models download!"
else
    echo "models folder exist! Remove it if you need to update."
fi
popd

popd