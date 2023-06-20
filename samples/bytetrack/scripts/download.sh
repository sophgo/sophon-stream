#!/bin/bash
pip3 install dfn
# sudo apt install unzip

scripts_dir=$(dirname $(readlink -f "$0"))
# echo $scripts_dir

pushd $scripts_dir

mkdir -p ../data

python3 -m dfn --url http://disk-sophgo-vip.quickconnect.cn/sharing/5bYERv4FR
unzip models.zip
rm -rf models.zip
mv ./models ../data/

python3 -m dfn --url http://disk-sophgo-vip.quickconnect.cn/sharing/yd6kbSu7n
unzip videos.zip
rm -rf videos.zip
mv ./videos ../data/

python3 -m dfn --url http://disk-sophgo-vip.quickconnect.cn/sharing/6DFPVVKsd
mv ./coco.names ../data/

popd




