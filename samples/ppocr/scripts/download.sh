#!/bin/bash
pip3 install dfss
#sudo apt install unzip

scripts_dir=$(dirname $(readlink -f "$0"))
# echo $scripts_dir

pushd $scripts_dir
# datasets
if [ ! -d "../data" ];
then
    mkdir -p ../data

    # models
    if [ ! -d "../data/models" ];
    then
        mkdir -p ../data/models
        python3 -m dfss --url=open@sophgo.com:sophon-stream/ppocr/BM1684.zip
        unzip BM1684.zip -d ../data/models
        rm BM1684.zip
        python3 -m dfss --url=open@sophgo.com:sophon-stream/ppocr/BM1684X.zip
        unzip BM1684X.zip -d ../data/models
        rm BM1684X.zip
        echo "models download!"
    else
        echo "models exist!"
    fi
    # datasets
    if [ ! -d "../data/datasets" ];
    then
        python3 -m dfss --url=open@sophgo.com:sophon-stream/ppocr/datasets.zip
        unzip datasets.zip -d ../data/
        rm datasets.zip
        echo "datasets download!"
    else
        echo "datasets exist!"
    fi
    echo "data download!"

    python3 -m dfss --url=open@sophgo.com:/sophon-stream/license_plate_recognition/wqy-microhei.ttc
    mv wqy-microhei.ttc ../data/

    python3 -m dfss --url=open@sophgo.com:/sophon-stream/ppocr/class.names
    mv class.names ../data/
else
    echo "data exist!"
fi



popd
