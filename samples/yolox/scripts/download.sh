#!/bin/bash
pip3 install dfss

scripts_dir=$(dirname $(readlink -f "$0"))
# echo $scripts_dir

pushd $scripts_dir

mkdir -p ../data

python3 -m dfss --url=open@sophgo.com:/sophon-stream/yolox/models.zip
unzip models.zip
rm -rf models.zip
mv ./models ../data/

python3 -m dfss --url=open@sophgo.com:/sophon-stream/yolox/videos.zip
unzip videos.zip
rm -rf videos.zip
mv ./videos ../data/

python3 -m dfss --url=open@sophgo.com:/sophon-stream/common/coco.names
mv ./coco.names ../data/

popd