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
    mkdir ../data
    python3 -m dfss --url=open@sophgo.com:/sophon-stream/dwa_dpu_encode/images.zip
    unzip images.zip
    rm -f images.zip
    mv ./images ../data/

    python3 -m dfss --url=open@sophgo.com:/sophon-stream/dwa_dpu_encode/videos.zip
    unzip videos.zip
    rm -f videos.zip
    mv ./videos ../data/

else
    echo "test image exist!"
fi

popd