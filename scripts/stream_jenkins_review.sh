#!/bin/bash
# set -ex
shell_dir=$(dirname $(readlink -f "$0"))

STREAM_BASIC_PATH=$shell_dir/..

function judge_ret() {
  if [[ $1 == 0 ]]; then
    echo -e "\033[32m Passed: $2 \033[0m"
  else
    echo -e "\033[31m Failed: $2 \033[0m"
    exit 1
  fi
  sleep 1
}

function apt_install()
{
    apt-get update -y
    apt-get install libboost-all-dev -y
}

function download_soc_sdk_allin()
{
    sdk_version=$1
    pushd $STREAM_BASIC_PATH/
    python3 -m dfss --url=open@sophgo.com:/soc-sdk-allin/$sdk_version/soc-sdk-allin.tgz
    judge_ret $? "download $sdk_version/soc-sdk-allin.tgz"
    if [ -f $STREAM_BASIC_PATH/soc-sdk-allin ]; then
        rm -rf $STREAM_BASIC_PATH/soc-sdk-allin
    fi
    tar -xzvf soc-sdk-allin.tgz
    judge_ret $? "tar -xzvf soc-sdk-allin.tgz"
    popd
}

function build_soc_stream()
{
    pushd $STREAM_BASIC_PATH/
    if [ -d build ];then
        rm -rf build
    fi
    mkdir build && cd build 
    cmake ../ -DCMAKE_BUILD_TYPE=Release -DTARGET_ARCH=soc -DSOPHON_SDK_SOC=$STREAM_BASIC_PATH/soc-sdk-allin
    make -j
    judge_ret $? "Stream run make"
    popd
}

echo "-------------------------Start Install dfss --------------------------------------"
pip3 install dfss --upgrade
echo "-------------------------Start apt install --------------------------------------"
apt_install
echo "-------------------------Start Download soc-sdk-allin v23.07.01 ---------------------------"
download_soc_sdk_allin v23.07.01
echo "-------------------------Start Build soc stream ----------------------"
build_soc_stream
