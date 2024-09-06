#!/bin/bash

res=$(which 7z)
if [ $? != 0 ];
then
    echo "Please install 7z on your system!"
    echo "To install, use the following command:"
    echo "sudo apt install p7zip;sudo apt install p7zip-full"
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
    pushd ..
    python3 -m dfss --url=open@sophgo.com:/sophon-stream/bird_dwa_blend_encode/data.tar.gz
    tar -zxvf data.tar.gz
    rm -f data.tar.gz
    popd
else
    echo "test image exist!"
fi

popd