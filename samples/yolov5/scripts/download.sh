#!/bin/bash
pip3 install dfn

scripts_dir=$(dirname $(readlink -f "$0"))
# echo $scripts_dir

pushd $scripts_dir


mkdir -p ../data/videos
python3 -m dfn --url http://disk-sophgo-vip.quickconnect.cn/sharing/9rx7IaGeM
mv test_car_person_1080P.avi ../data/videos

mkdir -p ../data/models
python3 -m dfn --url http://disk-sophgo-vip.quickconnect.cn/sharing/EEI2Vssro
mv ./yolov5s_tpukernel_int8_4b.bmodel ../data/models

python3 -m dfn --url http://disk-sophgo-vip.quickconnect.cn/sharing/9JzgcqIYv
mv ./coco.names ../data/

python3 -m dfn --url http://disk-sophgo-vip.quickconnect.cn/sharing/tsVSE1KqJ
mv ./tpu_kernel_module ../../../share/3rdparty/

popd