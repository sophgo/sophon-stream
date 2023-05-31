#!/bin/bash
pip3 install dfn

scripts_dir=$(dirname $(readlink -f "$0"))

pushd $scripts_dir

# datasets
if [ ! -d "../datasets" ];
then
    mkdir -p ../datasets/

    python3 -m dfn --url http://disk-sophgo-vip.quickconnect.cn/sharing/rIeMIlDND
    tar xvf kitti.tar.gz -C ../datasets/
    rm kitti.tar.gz

    echo "datasets download!"
else
    echo "datasets exist!"
fi

# models
if [ ! -d "../models" ];
then
    python3 -m dfn --url http://disk-sophgo-vip.quickconnect.cn/sharing/JxlsXqQSo
    tar xvf models.tar.gz -C ../
    rm models.tar.gz
    echo "models download!"
else
    echo "models exist!"
fi

popd
