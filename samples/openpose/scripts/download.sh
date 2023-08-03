#!/bin/bash
pip3 install dfn
#sudo apt install unzip

scripts_dir=$(dirname $(readlink -f "$0"))
# echo $scripts_dir

pushd $scripts_dir
# datasets
if [ ! -d "../data" ];
then
    mkdir -p ../data

    # models
    if [ ! -d "../data/models" ];
    then
        python3 -m dfn --url https://disk.sophgo.vip/sharing/JboKjYqF5
        unzip models.zip -d ../data/
        rm models.zip
        echo "models download!"
    else
        echo "models exist!"
    fi
    # models
    if [ ! -d "../data/videos" ];
    then
        mkdir -p ../data/videos

        python3 -m dfn --url https://disk.sophgo.vip/sharing/8gY6PTREJ
        mv test.mp4 ../data/videos/
        echo "videos download!"
    else
        echo "videos exist!"
    fi
    echo "data download!"
else
    echo "data exist!"
fi



popd