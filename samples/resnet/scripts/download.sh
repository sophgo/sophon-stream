#!/bin/bash
pip3 install dfn

scripts_dir=$(dirname $(readlink -f "$0"))
# echo $scripts_dir

pushd $scripts_dir

mkdir -p ../data

python3 -m dfn --url https://disk.sophgo.vip/sharing/8tO0KfvqS
unzip models.zip
rm -rf models.zip
mv ./models ../data/

python3 -m dfn --url https://disk.sophgo.vip/sharing/27FLRxS9N
unzip images.zip
rm -rf images.zip
mv ./images ../data/

python3 -m dfn --url https://disk.sophgo.vip/sharing/bwGdj0qMP
unzip videos.zip
rm -rf videos.zip
mv ./videos ../data/

popd
