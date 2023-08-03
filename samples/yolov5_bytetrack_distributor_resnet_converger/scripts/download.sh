#!/bin/bash
pip3 install dfn

scripts_dir=$(dirname $(readlink -f "$0"))

pushd $scripts_dir

mkdir ../data

mkdir ../data/models
python3 -m dfn --url https://disk.sophgo.vip/sharing/8StGdpPar
unzip BM1684X.zip
rm -rf BM1684X.zip
mv ./BM1684X ../data/models/BM1684X

python3 -m dfn --url https://disk.sophgo.vip/sharing/T5LZRfQiD
unzip BM1684_.zip
rm -rf BM1684_.zip
mv ./BM1684 ../data/models/BM1684

python3 -m dfn --url https://disk.sophgo.vip/sharing/QVAqwwdDK
unzip BM1684X_tpukernel.zip
rm -rf BM1684X_tpukernel.zip
mv ./BM1684X_tpukernel ../data/models/BM1684X_tpukernel

python3 -m dfn --url https://disk.sophgo.vip/sharing/TuopqMU10
mv ./coco.names ../data/

python3 -m dfn --url https://disk.sophgo.vip/sharing/frJFh8Oih
mv ./car.attributes ../data/

python3 -m dfn --url https://disk.sophgo.vip/sharing/w1o2J2gHm
mv ./person.attributes ../data/

python3 -m dfn --url https://disk.sophgo.vip/sharing/jKGGeHCVP
unzip videos.zip
rm -rf videos.zip
mv ./videos ../data/