#!/bin/bash
pip3 install dfss

scripts_dir=$(dirname $(readlink -f "$0"))
# echo $scripts_dir

pushd $scripts_dir

mkdir ../data

python3 -m dfss --url=open@sophgo.com:/sophon-stream/yolov5/videos.zip
unzip videos.zip
rm -rf videos.zip
mv ./videos ../data/

mkdir ../data/models
python3 -m dfss --url=open@sophgo.com:/sophon-stream/yolov8/BM1684X.tar.gz
tar -zxvf BM1684X.tar.gz
rm -rf BM1684X.tar.gz
mv ./BM1684X ../data/models/BM1684X

python3 -m dfss --url=open@sophgo.com:/sophon-stream/yolov8/BM1684.tar.gz
tar -zxvf BM1684.tar.gz
rm -rf BM1684.tar.gz
mv ./BM1684 ../data/models/BM1684

python3 -m dfss --url=open@sophgo.com:/sophon-stream/yolov8/BM1688.zip
unzip BM1688.zip
rm -rf BM1688.zip
mv ./BM1688 ../data/models/BM1688

python3 -m dfss --url=open@sophgo.com:/sophon-stream/common/coco.names
mv ./coco.names ../data/

popd
