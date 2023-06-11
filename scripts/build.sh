#!/bin/bash
script_dir=$(dirname $(readlink -f "$0"))
project_dir=${script_dir}/..

if [ $# -ne 1 ] || ([ "$1" != "Debug" ] && [ "$1" != "Release" ]); then
  echo "Usage: $0 <Debug|Release>"
  exit 1
fi

pushd ${project_dir}


framework_dir=${project_dir}/framework
echo "build framework-----"
pushd $framework_dir
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
echo "build framework completed"

element_dir=${project_dir}/element
echo "build element-----"

element_yolov5_dir=$element_dir/algorithm/yolov5
pushd $element_yolov5_dir
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


element_yolox_dir=$element_dir/algorithm/yolox
pushd $element_yolox_dir
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

# element_unet_dir=$element_dir/algorithm/unet
# pushd $element_unet_dir
# if [ ! -d "build" ]; then
#   mkdir build
# fi
# cd build
# rm -rf *
# if [ "$1" == "Debug" ]; then
#   cmake -DCMAKE_BUILD_TYPE=Debug ..
# elif [ "$1" == "Release" ]; then
#   cmake -DCMAKE_BUILD_TYPE=Release ..
# fi
# make -j
# popd

element_tracker_dir=$element_dir/algorithm/bytetrack
pushd $element_tracker_dir
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

element_decode_dir=$element_dir/multimedia/decode
pushd $element_decode_dir
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

element_osd_dir=$element_dir/multimedia/osd
pushd $element_osd_dir
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

element_encode_dir=$element_dir/multimedia/encode
pushd $element_encode_dir
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

sample_dir=${project_dir}/samples/yolox
echo "build yolox-----"
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

sample_dir=${project_dir}/samples/yolov5
echo "build yolov5-----"
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


sample_dir=${project_dir}/samples/bytetrack
echo "build bytetrack-----"
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

sample_dir=${project_dir}/samples/yolox_bytetrack_osd_encode
echo "build yolox_bytetrack_osd_encode-----"
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
