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
    mkdir ../data
    python3 -m dfss --url=open@sophgo.com:/sophon-stream/dwa_blend_encode/images.zip
    unzip images.zip
    rm -f images.zip
    mv ./images ../data/

    python3 -m dfss --url=open@sophgo.com:/sophon-stream/dwa_blend_encode/wgt.zip
    unzip wgt.zip
    rm -f wgt.zip
    mv ./wgt ../data/

    python3 -m dfss --url=open@sophgo.com:/sophon-stream/dwa_blend_encode/gridinfo.zip
    unzip gridinfo.zip
    rm -f gridinfo.zip
    mv ./gridinfo ../data/

else
    echo "test image exist!"
fi

popd