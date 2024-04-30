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

python3 -m dfss --url=open@sophgo.com:/sophon-stream/yolov8/BM1688.tar.gz
tar -zxvf BM1688.tar.gz
rm -rf BM1688.tar.gz
mv ./BM1688 ../data/models/BM1688

python3 -m dfss --url=open@sophgo.com:/sophon-stream/common/coco.names
mv ./coco.names ../data/

python3 -m dfss --url=open@sophgo.com:/sophon-stream/yolov8/yolov8_data.tar.gz
tar -zxvf yolov8_data.tar.gz
mv ./yolov8_data/pics ../data/
mv ./yolov8_data/videos/* ../data/videos/
rm -rf yolov8_data*

python3 -m dfss --url=open@sophgo.com:/sophon-stream/yolov8/models_opt.tar.gz
tar -zxvf ./models_opt.tar.gz
rsync -auv ./models/ ../data/models/
rm -rf ./models/
rm -rf ./models_opt.tar.gz

python3 -m dfss --url=open@sophgo.com:sophon-demo/YOLOv8/models_240403/CV186X.zip
unzip CV186X.zip
rm -r CV186X.zip
mv ./CV186X ../data/models/

popd
