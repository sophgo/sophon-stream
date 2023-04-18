#!/bin/bash
script_dir=$(dirname $(readlink -f "$0"))
project_dir=${script_dir}/..

if [ $# -ne 2 ] || ([ "$1" != "Debug" ] && [ "$1" != "Release" ]); then
  echo "Usage: $0 <Debug|Release>"
  exit 1
fi
SOC_SDK=$2
echo ${project_dir}
pushd ${project_dir}

echo "build algorithm-----"
cd ./algorithm
if [ ! -d "build_soc" ]; then
  mkdir build_soc
fi
cd build_soc
 rm -rf *
if [ "$1" == "Debug" ]; then
  cmake -DCMAKE_BUILD_TYPE=Debug -DTARGET_ARCH=soc -DSDK=${SOC_SDK} ..
elif [ "$1" == "Release" ]; then
  cmake -DCMAKE_BUILD_TYPE=Release -DTARGET_ARCH=soc -DSDK=${SOC_SDK} ..
fi
make -j
echo "build algorithm completed"

echo "build multimedia-----"
cd ../../multimedia
if [ ! -d "build_soc" ]; then
  mkdir build_soc
fi
cd build_soc
 rm -rf *
if [ "$1" == "Debug" ]; then
  cmake -DCMAKE_BUILD_TYPE=Debug -DTARGET_ARCH=soc -DSDK=${SOC_SDK} ..
elif [ "$1" == "Release" ]; then
  cmake -DCMAKE_BUILD_TYPE=Release -DTARGET_ARCH=soc -DSDK=${SOC_SDK} ..
fi
make -j
echo "build multimedia completed"

echo "build engine-----"
cd ../../engine
if [ ! -d "build_soc" ]; then
  mkdir build_soc
fi
if [ ! -d "lib_soc" ]; then
  mkdir lib_soc
fi
cd build_soc
 rm -rf *
if [ "$1" == "Debug" ]; then
  cmake -DCMAKE_BUILD_TYPE=Debug -DTARGET_ARCH=soc -DSDK=${SOC_SDK} ..
elif [ "$1" == "Release" ]; then
  cmake -DCMAKE_BUILD_TYPE=Release -DTARGET_ARCH=soc -DSDK=${SOC_SDK} ..
fi
make -j
echo "build engine completed"

cp ../../algorithm/build_soc/libalgorithmApi.so ../lib_soc
cp ../../multimedia/build_soc/libmultiMediaApi.so ../lib_soc

popd
echo "All completed"
