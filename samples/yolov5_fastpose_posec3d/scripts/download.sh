#!/bin/bash
pip3 install dfss

scripts_dir=$(dirname $(readlink -f "$0"))

pushd $scripts_dir

mkdir ../data

mkdir ../data/models
python3 -m dfss --url=open@sophgo.com:/sophon-stream/yolov5/BM1684X.zip
unzip BM1684X.zip
rm -rf BM1684X.zip
mv ./BM1684X ../data/models/BM1684X

python3 -m dfss --url=open@sophgo.com:/sophon-stream/yolov5/BM1684.zip
unzip BM1684.zip
rm -rf BM1684.zip
mv ./BM1684 ../data/models/BM1684

python3 -m dfss --url=open@sophgo.com:/sophon-stream/yolov5/BM1684X_tpukernel.zip
unzip BM1684X_tpukernel.zip
rm -rf BM1684X_tpukernel.zip
mv ./BM1684X ../data/models/BM1684X_tpukernel

python3 -m dfss --url=open@sophgo.com:/sophon-stream/fastpose/coco17_BM1684X.zip
unzip coco17_BM1684X.zip
rm -rf coco17_BM1684X.zip
mv ./coco17_fastpose/* ../data/models/BM1684X
rm -r ./coco17_fastpose

python3 -m dfss --url=open@sophgo.com:/sophon-stream/posec3d/BM1684X.zip
unzip BM1684X.zip
rm -rf BM1684X.zip
mv ./posec3d/* ../data/models/BM1684X
rm -r ./posec3d

mkdir -p ../data/videos
python3 -m dfss --url=open@sophgo.com:/sophon-stream/openpose/test.mp4
mv test.mp4 ../data/videos/
python3 -m dfss --url=open@sophgo.com:/sophon-stream/posec3d/demo_skeleton.mp4
mv demo_skeleton.mp4 ../data/videos/
python3 -m dfss --url=open@sophgo.com:/sophon-stream/posec3d/S017C001P003R001A001_rgb.avi
mv S017C001P003R001A001_rgb.avi ../data/videos/
python3 -m dfss --url=open@sophgo.com:/sophon-stream/posec3d/S017C001P003R002A008_rgb.avi
mv S017C001P003R002A008_rgb.avi ../data/videos/

python3 -m dfss --url=open@sophgo.com:/sophon-stream/common/coco.names
mv ./coco.names ../data/
python3 -m dfss --url=open@sophgo.com:/sophon-stream/posec3d/label_map_gym99.txt
mv ./label_map_gym99.txt ../data/
python3 -m dfss --url=open@sophgo.com:/sophon-stream/posec3d/label_map_ntu60.txt
mv ./label_map_ntu60.txt ../data/