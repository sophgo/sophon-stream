#!/bin/bash

res=$(which 7z)
if [ $? != 0 ];
then
    echo "Please install 7z on your system!"
    echo "To install, use the following command:"
    echo "sudo apt install p7zip;sudo apt install p7zip-full"
    exit
fi

pip3 install dfss --upgrade
# sudo apt install unzip

scripts_dir=$(dirname $(readlink -f "$0"))
# echo $scripts_dir

pushd $scripts_dir
# datasets
if [ ! -d "../data" ]; 
then
    mkdir ../data
    python3 -m dfss --url=open@sophgo.com:/sophon-stream/lprnet/test_green.zip
    unzip test_green.zip
    rm -f test_green.zip
    mv ./test ../data/

    python3 -m dfss --url=open@sophgo.com:/sophon-stream/license_plate_recognition/coco.names
    mv coco.names ../data/
    rm -f coco.names
    echo "test image download!"

    python3 -m dfss --url=open@sophgo.com:/sophon-stream/license_plate_recognition/wqy-microhei.ttc
    mv wqy-microhei.ttc ../data/
else
    echo "test image exist!"
fi

# models
if [ ! -d "../models" ]; 
then
    mkdir -p ../models ../models/lprnet/
    python3 -m dfss --url=open@sophgo.com:/sophon-stream/lprnet/models.7z    
    7z x models.7z 
    mv models/* ../models/lprnet/
    rm -r models.7z models
    
    python3 -m dfss --url=open@sophgo.com:/sophon-stream/license_plate_recognition/yolov5s-licensePlate.7z
    7z x yolov5s-licensePlate.7z -o../models
    rm yolov5s-licensePlate.7z

    python3 -m dfss --url=open@sophgo.com:/sophon-stream/license_plate_recognition/BM1688.tar.gz
    tar -zxvf ./BM1688.tar.gz
    mkdir -p ../models/lprnet/BM1688
    mkdir -p ../models/yolov5s-licensePLate/BM1688
    mv ./BM1688/lprnet* ../models/lprnet/BM1688
    mv ./BM1688/yolov5s* ../models/yolov5s-licensePLate/BM1688
    rm -rf ./BM1688.tar.gz
    rm -rf ./BM1688
    
    echo "models download!"
else
    echo "models exist!"
fi
popd