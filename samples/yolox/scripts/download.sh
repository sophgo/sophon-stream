#!/bin/bash
pip3 install dfn

scripts_dir=$(dirname $(readlink -f "$0"))
# echo $scripts_dir

pushd $scripts_dir


mkdir -p ../data/videos
python3 -m dfn --url http://disk-sophgo-vip.quickconnect.cn/sharing/9rx7IaGeM
mv test_car_person_1080P.avi ../data/videos

mkdir -p ../data/models
python3 -m dfn --url http://disk-sophgo-vip.quickconnect.cn/sharing/1oARRrhko
unzip BM1684X.zip
rm -rf BM1684X.zip
mv ./BM1684X ../data/models

python3 -m dfn --url http://disk-sophgo-vip.quickconnect.cn/sharing/bfIf7SKRo
unzip BM1684.zip
rm -rf BM1684.zip
mv ./BM1684 ../data/models

popd