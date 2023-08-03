#!/bin/bash
pip3 install dfn

scripts_dir=$(dirname $(readlink -f "$0"))

pushd $scripts_dir

mkdir -p ../data

python3 -m dfn --url https://disk.sophgo.vip/sharing/YCT0higWb
tar xvf videos.tar -C ../data
rm -rf videos.tar

mkdir -p ../data
python3 -m dfn --url https://disk.sophgo.vip/sharing/BnRScQZdR
tar xvf models.tar -C ../data
rm -rf models.tar

python3 -m dfn --url https://disk.sophgo.vip/sharing/7s90PTxEi
mv ./coco.names ../data

popd