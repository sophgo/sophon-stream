#!/bin/bash
pip3 install dfn

scripts_dir=$(dirname $(readlink -f "$0"))
# echo $scripts_dir

pushd $scripts_dir

mkdir -p ../data

python3 -m dfn --url http://disk-sophgo-vip.quickconnect.cn/sharing/a3upUqXGE
unzip models.zip
rm -rf models.zip
mv ./models ../data/

python3 -m dfn --url http://disk-sophgo-vip.quickconnect.cn/sharing/MYS5paD3l
unzip videos.zip
rm -rf videos.zip
mv ./videos ../data/

python3 -m dfn --url http://disk-sophgo-vip.quickconnect.cn/sharing/6DFPVVKsd
mv ./coco.names ../data/

popd