#!/bin/bash

res=$(which unzip)
if [ $? != 0 ];
then
    echo "Please install unzip on your system!"
    echo "To install, use the following command:"
    echo "sudo apt install unzip"
    exit
fi

pip3 install dfss --upgrade
# sudo apt install unzip

scripts_dir=$(dirname $(readlink -f "$0"))
# echo $scripts_dir

pushd $scripts_dir
# datasets
if [ ! -d "../data" ]; 
then

    python3 -m dfss --url=open@sophgo.com:/sophon-stream/gdwa_blend_encode/data.zip
    unzip data.zip
    rm -f data.zip
    mv data ..
    python3 -m dfss --url=open@sophgo.com:/sophon-stream/dwa_blend_encode/cvi_sdr_bin
    mv cvi_sdr_bin ../data

else
    echo "test image exist!"
fi

popd