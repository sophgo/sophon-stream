#!/bin/bash
pip3 install dfn
# sudo apt install unzip

scripts_dir=$(dirname $(readlink -f "$0"))
# echo $scripts_dir

pushd $scripts_dir
# datasets
if [ ! -d "../data" ];
then
    python3 -m dfn --url http://219.142.246.77:65000/sharing/wEeqHde0i
    unzip data.zip -d ../
    rm data.zip

else
    echo "data exist!"
fi

popd