#!/bin/bash



pip3 install dfss --upgrade
# sudo apt install unzip

scripts_dir=$(dirname $(readlink -f "$0"))
# echo $scripts_dir

pushd $scripts_dir
# datasets
if [ ! -d "../data" ]; 
then
    python3 -m dfss --url=open@sophgo.com:/sophon-stream/dwa_blend_encode/data.tar.gz
    tar -zxvf data.tar.gz
    mv data ..
    rm -f data.tar.gz
    python3 -m dfss --url=open@sophgo.com:/sophon-stream/dwa_blend_encode/cvi_sdr_bin
    mv cvi_sdr_bin ../data
  

else
    echo "test image exist!"
fi

popd