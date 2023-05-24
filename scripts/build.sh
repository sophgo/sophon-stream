#!/bin/bash
script_dir=$(dirname $(readlink -f "$0"))
project_dir=${script_dir}/..

if [ $# -ne 1 ] || ([ "$1" != "Debug" ] && [ "$1" != "Release" ]); then
  echo "Usage: $0 <Debug|Release>"
  exit 1
fi

pushd ${project_dir}


engine_dir=${project_dir}/engine
echo "build engine-----"
pushd $engine_dir
if [ ! -d "build" ]; then
  mkdir build
fi
cd build
rm -rf *
if [ "$1" == "Debug" ]; then
  cmake -DCMAKE_BUILD_TYPE=Debug ..
elif [ "$1" == "Release" ]; then
  cmake -DCMAKE_BUILD_TYPE=Release ..
fi
make -j
popd
echo "build engine completed"

sample_dir=${project_dir}/samples
echo "build samples-----"
pushd $sample_dir
if [ ! -d "build" ]; then
  mkdir build
fi
cd build
rm -rf *
if [ "$1" == "Debug" ]; then
  cmake -DCMAKE_BUILD_TYPE=Debug ..
elif [ "$1" == "Release" ]; then
  cmake -DCMAKE_BUILD_TYPE=Release ..
fi
make -j
popd
echo "build samples completed"

popd
echo "All completed"
