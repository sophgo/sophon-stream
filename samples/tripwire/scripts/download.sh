#!/bin/bash
pip3 install dfss

scripts_dir=$(dirname $(readlink -f "$0"))

pushd $scripts_dir

mkdir -p ../data

python3 -m dfss --url=open@sophgo.com:/sophon-stream/tripwire/test.mp4
mv test.mp4 ../data
mkdir -p ../data
python3 -m dfss --url=open@sophgo.com:/sophon-stream/yolox_bytetrack_osd_encode/models.tar
tar xvf models.tar -C ../data
rm -rf models.tar

python3 -m dfss --url=open@sophgo.com:/sophon-stream/yolox_bytetrack_osd_encode/BM1688.tar.gz
tar -zxvf BM1688.tar.gz
rm -rf BM1688.tar.gz
mv ./BM1688 ../data/models/

python3 -m dfss --url=open@sophgo.com:/sophon-stream/common/coco.names
mv ./coco.names ../data

popd