#!/bin/bash
script_dir=$(dirname $(readlink -f "$0"))
project_dir=${script_dir}/..

if [ $# -ne 2 ] || ([ "$1" != "Debug" ] && [ "$1" != "Release" ]); then
  echo "Usage: $0 <Debug|Release> <path to soc-sdk>"
  exit 1
fi
SOC_SDK=$2

echo ${project_dir}
pushd ${project_dir}

framework_dir=${project_dir}/framework
echo "build framework-----"
pushd $framework_dir
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
popd
echo "build framework completed"


element_dir=${project_dir}/element
echo "build element-----"

element_yolov5_dir=$element_dir/algorithm/yolov5
pushd $element_yolov5_dir
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
popd

# element_yolox_dir=$element_dir/algorithm/yolox
# pushd $element_yolox_dir
# if [ ! -d "build_soc" ]; then
#   mkdir build_soc
# fi
# cd build_soc
# rm -rf *
# if [ "$1" == "Debug" ]; then
#   cmake -DCMAKE_BUILD_TYPE=Debug -DTARGET_ARCH=soc -DSDK=${SOC_SDK} ..
# elif [ "$1" == "Release" ]; then
#   cmake -DCMAKE_BUILD_TYPE=Release -DTARGET_ARCH=soc -DSDK=${SOC_SDK} ..
# fi
# make -j
# popd
# echo "build yolox completed"

# element_tracker_dir=$element_dir/algorithm/tracker
# pushd $element_tracker_dir
# if [ ! -d "build_soc" ]; then
#   mkdir build_soc
# fi
# cd build_soc
# rm -rf *
# if [ "$1" == "Debug" ]; then
#   cmake -DCMAKE_BUILD_TYPE=Debug -DTARGET_ARCH=soc -DSDK=${SOC_SDK} ..
# elif [ "$1" == "Release" ]; then
#   cmake -DCMAKE_BUILD_TYPE=Release -DTARGET_ARCH=soc -DSDK=${SOC_SDK} ..
# fi
# make -j
# popd


element_decode_dir=$element_dir/multimedia/decode
pushd $element_decode_dir
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
popd
echo "build decoder completed"

# sample_dir=${project_dir}/samples/yolox
# echo "build yolox-----"
# pushd $sample_dir
# if [ ! -d "build_soc" ]; then
#   mkdir build_soc
# fi
# cd build_soc
# rm -rf *
# if [ "$1" == "Debug" ]; then
#   cmake -DCMAKE_BUILD_TYPE=Debug -DTARGET_ARCH=soc -DSDK=${SOC_SDK} ..
# elif [ "$1" == "Release" ]; then
#   cmake -DCMAKE_BUILD_TYPE=Release -DTARGET_ARCH=soc -DSDK=${SOC_SDK} ..
# fi
# make -j
# popd
# echo "build usecase yolox completed"

sample_dir=${project_dir}/samples/yolov5
echo "build yolov5-----"
pushd $sample_dir
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
popd
echo "build usecase yolov5 completed"


popd

echo "All completed"
