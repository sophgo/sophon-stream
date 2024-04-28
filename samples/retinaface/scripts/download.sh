#!/bin/bash
pip3 install dfss

res=$(dpkg -l|grep unzip)
if [ $? != 0 ];
then
    echo "Please install unzip on your system!"
    exit
fi

scripts_dir=$(dirname $(readlink -f "$0"))
# echo $scripts_dir

pushd $scripts_dir

mkdir -p ../data/images
mkdir -p ../data/images/face

# 测试集wind
python3 -m dfss --url=open@sophgo.com:/sophon-stream/retinaface/wind.zip
unzip wind.zip -d ../data/images/
rm wind.zip

# 测试集WIDER FACE
python3 -m dfss --url=open@sophgo.com:/sophon-stream/retinaface/WIDERVAL.zip
unzip WIDERVAL.zip -d ../data/images/
rm WIDERVAL.zip

## 单张图片与视频
python3 -m dfss --url=open@sophgo.com:/sophon-stream/retinaface/test.zip
unzip test.zip -d ../data/images/face
# mv ../data/images/face0*.jpg ../data/images/face
rm test.zip

# models
python3 -m dfss --url=open@sophgo.com:/sophon-stream/retinaface/models.zip
unzip models.zip -d ../data/
rm models.zip

python3 -m dfss --url=open@sophgo.com:/sophon-stream/retinaface/BM1688.tar.gz
tar -zxvf BM1688.tar.gz
mv ./BM1688 ../data/models/
rm -rf BM1688.tar.gz

python3 -m dfss --url=open@sophgo.com:/sophon-stream/retinaface/CV186X.tar.gz
tar -zxvf CV186X.tar.gz
mv ./CV186X ../data/models/
rm -rf CV186X.tar.gz

popd
