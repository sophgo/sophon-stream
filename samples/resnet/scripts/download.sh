#!/bin/bash
pip3 install dfn

scripts_dir=$(dirname $(readlink -f "$0"))
# echo $scripts_dir

pushd $scripts_dir

mkdir -p ../data

python3 -m dfn --url http://disk-sophgo-vip.quickconnect.cn/sharing/le9JmwYIZ
unzip models.zip
rm -rf models.zip
mv ./models ../data/

python3 -m dfn --url http://disk-sophgo-vip.quickconnect.cn/sharing/FC7jLpSdz
unzip images.zip
rm -rf images.zip
mv ./images ../data/

python3 -m dfn --url http://disk-sophgo-vip.quickconnect.cn/sharing/KqXJ2OpjN
unzip videos.zip
rm -rf videos.zip
mv ./videos ../data/

popd