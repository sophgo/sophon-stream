#!/bin/bash

res=$(which 7z)
if [ $? != 0 ];
then
    echo "Please install 7z on your system!"
    echo "To install, use the following command:"
    echo "sudo apt install p7zip; sudo apt install p7zip-full; sudo apt install unzip"
    exit
fi

pip3 install dfss --upgrade
# sudo apt install unzip

scripts_dir=$(dirname $(readlink -f "$0"))
# echo $scripts_dir

pushd $scripts_dir
# datasets
mkdir -p ../datasets
python3 -m dfss --url=open@sophgo.com:/sophon-demo/license_plate_recognition/1080_1920_5s.zip
unzip 1080_1920_5s.zip 
mv 1080_1920_5s.mp4 ../datasets/   
python3 -m dfss --url=open@sophgo.com:/sophon-stream/license_plate_recognition/coco.names
mv coco.names ../datasets/ 
rm -f coco.names
rm -f 1080_1920_5s.zip

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
    mv ../models/yolov5s-licensePlate ../models/yolov5s-licensePLate

    python3 -m dfss --url=open@sophgo.com:/sophon-stream/license_plate_recognition/BM1688.tar.gz
    tar -zxvf ./BM1688.tar.gz
    mkdir -p ../models/lprnet/BM1688
    mkdir -p ../models/yolov5s-licensePLate/BM1688
    mv ./BM1688/lprnet* ../models/lprnet/BM1688
    mv ./BM1688/yolov5s* ../models/yolov5s-licensePLate/BM1688
    rm -rf ./BM1688.tar.gz
    rm -rf ./BM1688
    python3 -m dfss --url=open@sophgo.com:/sophon-stream/license_area_intrusion/lprnet_2c.zip
    unzip lprnet_2c.zip
    mv ./lprnet_int8_4b_2core/compilation.bmodel ../models/lprnet/BM1688/lprnet_new_int8_4b_2core.bmodel
    rm -rf ./BM1688.tar.gz
    rm -rf ./BM1688
    rm -rf ./lprnet_int8_4b_2core/
    rm -rf ./lprnet_int8_4b/
    rm lprnet_2c.zip
    echo "models download!"
else
    echo "models exist!"
fi
popd