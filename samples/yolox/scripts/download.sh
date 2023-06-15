#!/bin/bash
pip3 install dfn

scripts_dir=$(dirname $(readlink -f "$0"))
# echo $scripts_dir

pushd $scripts_dir

mkdir ../data

mkdir -p ../data/videos
python3 -m dfn --url http://disk-sophgo-vip.quickconnect.cn/sharing/eL6XZZLls
mv test_car_person_1080P.avi ../data/videos

mkdir -p ../data/models

python3 -m dfn --url http://disk-sophgo-vip.quickconnect.cn/sharing/3ufyhpgEs
unzip BM1684X.zip
rm -rf BM1684X.zip
mv ./BM1684X ../data/models/BM1684X

python3 -m dfn --url http://disk-sophgo-vip.quickconnect.cn/sharing/mgN5Q8v3P
unzip BM1684.zip
rm -rf BM1684.zip
mv ./BM1684 ../data/models/BM1684

python3 -m dfn --url http://disk-sophgo-vip.quickconnect.cn/sharing/6DFPVVKsd
mv ./coco.names ../data/

popd