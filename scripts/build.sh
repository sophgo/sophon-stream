#!/bin/bash
script_dir=$(dirname $(readlink -f "$0"))
project_dir=${script_dir}/..

if [ $# -ne 1 ] || ([ "$1" != "Debug" ] && [ "$1" != "Release" ]); then
  echo "Usage: $0 <Debug|Release>"
  exit 1
fi

pushd ${project_dir}

echo "build algorithm-----"
cd ./algorithm
if [ ! -d "build" ]; then
  mkdir build
fi
cd build
# rm -rf *
if [ "$1" == "Debug" ]; then
  cmake -DCMAKE_BUILD_TYPE=Debug ..
elif [ "$1" == "Release" ]; then
  cmake -DCMAKE_BUILD_TYPE=Release ..
fi
make -j
echo "build algorithm completed"

echo "build multimedia-----"
cd ../../multimedia
if [ ! -d "build" ]; then
  mkdir build
fi
cd build
# rm -rf *
if [ "$1" == "Debug" ]; then
  cmake -DCMAKE_BUILD_TYPE=Debug ..
elif [ "$1" == "Release" ]; then
  cmake -DCMAKE_BUILD_TYPE=Release ..
fi
make -j
echo "build multimedia completed"

echo "build engine-----"
cd ../../engine
if [ ! -d "build" ]; then
  mkdir build
fi
if [ ! -d "lib" ]; then
  mkdir lib
fi
cd build
# rm -rf *
if [ "$1" == "Debug" ]; then
  cmake -DCMAKE_BUILD_TYPE=Debug ..
elif [ "$1" == "Release" ]; then
  cmake -DCMAKE_BUILD_TYPE=Release ..
fi
make -j
echo "build engine completed"

cp ../../algorithm/build/libalgorithmApi.so ../lib
cp ../../multimedia/build/libmultiMediaApi.so ../lib

popd
echo "All completed"
