#!/bin/bash
pip3 install dfn

scripts_dir=$(dirname $(readlink -f "$0"))
# echo $scripts_dir

pushd $scripts_dir

mkdir ../data

python3 -m dfn --url https://disk.sophgo.vip/sharing/i43toW1VC
unzip videos.zip
rm -rf videos.zip
mv ./videos ../data/

mkdir ../data/models
python3 -m dfn --url https://disk.sophgo.vip/sharing/vpMpnA5Y9
unzip BM1684X.zip
rm -rf BM1684X.zip
mv ./BM1684X ../data/models/BM1684X

python3 -m dfn --url https://disk.sophgo.vip/sharing/XOypOh4w7
unzip BM1684_.zip
rm -rf BM1684_.zip
mv ./BM1684 ../data/models/BM1684

python3 -m dfn --url https://disk.sophgo.vip/sharing/3u9Xeqp54
unzip BM1684X_tpukernel.zip
rm -rf BM1684X_tpukernel.zip
mv ./BM1684X ../data/models/BM1684X_tpukernel

python3 -m dfn --url https://disk.sophgo.vip/sharing/xkH58z3qF
mv ./coco.names ../data/

popd