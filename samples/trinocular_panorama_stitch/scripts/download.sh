#!/bin/bash

pip3 install dfss --upgrade

scripts_dir=$(dirname $(readlink -f "$0"))

pushd $scripts_dir
if [ ! -d "../dataets" ]; 
then
    pushd ..
    python3 -m dfss --url=open@sophgo.com:/sophon-stream/trinocular_panorama_stitch/datasets.tar.gz
    tar -zxvf datasets.tar.gz && rm -f datasets.tar.gz
    popd
else
    echo "datasets exist!"
fi

if [ ! -d "../gridinfo" ]; 
then
    pushd ..
    python3 -m dfss --url=open@sophgo.com:/sophon-stream/trinocular_panorama_stitch/gridinfo.tar.gz
    tar -zxvf gridinfo.tar.gz && rm -f gridinfo.tar.gz
    popd
else
    echo "gridinfo exist!"
fi
popd