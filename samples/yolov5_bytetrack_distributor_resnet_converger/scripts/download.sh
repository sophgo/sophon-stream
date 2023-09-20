#!/bin/bash
pip3 install dfss

scripts_dir=$(dirname $(readlink -f "$0"))

pushd $scripts_dir

mkdir ../data

mkdir ../data/models
python3 -m dfss --url=open@sophgo.com:/sophon-stream/yolov5_bytetrack_distributor_resnet_converger/BM1684X.zip
unzip BM1684X.zip
rm -rf BM1684X.zip
mv ./BM1684X ../data/models/BM1684X

python3 -m dfss --url=open@sophgo.com:/sophon-stream/yolov5_bytetrack_distributor_resnet_converger/BM1684.zip
unzip BM1684.zip
rm -rf BM1684.zip
mv ./BM1684 ../data/models/BM1684

python3 -m dfss --url=open@sophgo.com:/sophon-stream/yolov5_bytetrack_distributor_resnet_converger/BM1684X_tpukernel.zip
unzip BM1684X_tpukernel.zip
rm -rf BM1684X_tpukernel.zip
mv ./BM1684X_tpukernel ../data/models/BM1684X_tpukernel

python3 -m dfss --url=open@sophgo.com:/sophon-stream/common/coco.names
mv ./coco.names ../data/

python3 -m dfss --url=open@sophgo.com:/sophon-stream/yolov5_bytetrack_distributor_resnet_converger/car.attributes
mv ./car.attributes ../data/

python3 -m dfss --url=open@sophgo.com:/sophon-stream/yolov5_bytetrack_distributor_resnet_converger/person.attributes
mv ./person.attributes ../data/

python3 -m dfss --url=open@sophgo.com:/sophon-stream/yolov5_bytetrack_distributor_resnet_converger/videos.zip
unzip videos.zip
rm -rf videos.zip
mv ./videos ../data/