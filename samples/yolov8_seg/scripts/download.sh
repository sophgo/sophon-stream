#!/bin/bash
pip3 install dfss

scripts_dir=$(dirname $(readlink -f "$0"))

pushd $scripts_dir

mkdir -p ../data
mkdir -p ../data/models

python3 -m dfss --url=open@sophgo.com:/sophon-demo/YOLOv8_plus_seg/BM1684X.tar.gz
tar -zxvf BM1684X.tar.gz
rm -rf BM1684X.tar.gz
mv ./BM1684X ../data/models/BM1684X

python3 -m dfss --url=open@sophgo.com:/sophon-stream/yolov5/videos.zip
unzip videos.zip
rm -rf videos.zip
mv ./videos ../data/

python3 -m dfss --url=open@sophgo.com:/sophon-stream/common/coco.names
mv ./coco.names ../data/

python3 -m dfss --url=open@sophgo.com:/sophon-stream/yolov8/yolov8_data.tar.gz
tar -zxvf yolov8_data.tar.gz
mv ./yolov8_data/pics ../data/
mv ./yolov8_data/videos/* ../data/videos/
rm -rf yolov8_data*

popd
