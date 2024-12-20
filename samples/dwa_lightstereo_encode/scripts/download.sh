#!/bin/bash

pip3 install dfss --upgrade

scripts_dir=$(dirname $(readlink -f "$0"))
# echo $scripts_dir

pushd $scripts_dir
# datasets
if [ ! -d "../data" ]; 
then
    python3 -m dfss --url=open@sophgo.com:/sophon-stream/dwa_lightstereo_encode/gridinfo.tar.gz
    tar xvf gridinfo.tar.gz && rm gridinfo.tar.gz
    mv ./gridinfo ../data/

    mkdir -p ../data/models/BM1688
    python3 -m dfss --url=open@sophgo.com:/sophon-stream/dwa_lightstereo_encode/models/BM1688/LightStereo-S-SceneFlow_int8_1b_2core_480x736.bmodel
    mv LightStereo-S-SceneFlow_int8_1b_2core_480x736.bmodel ../data/models/BM1688
    python3 -m dfss --url=open@sophgo.com:/sophon-stream/dwa_lightstereo_encode/models/BM1688/LightStereo-S-SceneFlow_int8_1b_480x736.bmodel #1core
    mv LightStereo-S-SceneFlow_int8_1b_480x736.bmodel ../data/models/BM1688

    tar xvf gridinfo.tar.gz && rm gridinfo.tar.gz
    mv ./gridinfo ../data/

    python3 -m dfss --url=open@sophgo.com:/sophon-stream/dwa_dpu_encode/cvi_sdr_bin
    mv cvi_sdr_bin ../data/

else
    echo "data folder exists!"
fi

popd