#!/bin/bash
pip3 install dfss

scripts_dir=$(dirname $(readlink -f "$0"))
# echo $scripts_dir

pushd $scripts_dir

mkdir -p ../data

python3 -m dfss --url=open@sophgo.com:sophon-demo/ResNet/models_0918/models.zip
unzip models.zip
rm -rf models.zip
mv ./models ../data/

python3 -m dfss --url=open@sophgo.com:/sophon-stream/resnet/images.zip
unzip images.zip
rm -rf images.zip
mv ./images ../data/

python3 -m dfss --url=open@sophgo.com:/sophon-stream/resnet/videos.zip
unzip videos.zip
rm -rf videos.zip
mv ./videos ../data/

popd
