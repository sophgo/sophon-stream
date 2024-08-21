#!/bin/bash

pip3 install dfss --upgrade

scripts_dir=$(dirname $(readlink -f "$0"))

pushd $scripts_dir
# data
if [ ! -d "../data" ]; 
then
    mkdir -p ../data
    python3 -m dfss --url=open@sophgo.com:/sophon-stream/structured_recognition/models.tgz
    tar -xzf models.tgz
    rm -f models.tgz
    mv ./models ../data

    python3 -m dfss --url=open@sophgo.com:/sophon-stream/structured_recognition/videos.tgz
    tar -xzf videos.tgz
    rm -f videos.tgz
    mv ./videos ../data

    python3 -m dfss --url=open@sophgo.com:/sophon-stream/structured_recognition/coco.tgz
    tar -xzf coco.tgz
    rm -f coco.tgz
    mv ./coco* ../data

    mkdir -p ../tools
    python3 -m dfss --url=open@sophgo.com:/sophon-stream/structured_recognition/application-web-linux_arm64.tgz
    mv application-web-linux_arm64.tgz ../tools
else
    echo "data exist!"
fi

popd