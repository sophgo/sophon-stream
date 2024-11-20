#!/bin/bash
pip3 install dfss

scripts_dir=$(dirname $(readlink -f "$0"))

pushd $scripts_dir

mkdir ../data

mkdir ../data/models
python3 -m dfss --url=open@sophgo.com:/sophon-stream/retinaface_distributor_resnet_faiss_converger/BM1684X.zip
unzip BM1684X.zip
rm -rf BM1684X.zip
mv ./BM1684X ../data/models/BM1684X

python3 -m dfss --url=open@sophgo.com:/sophon-stream/retinaface_distributor_resnet_faiss_converger/BM1688.zip
unzip BM1688.zip
rm -rf BM1688.zip
mv ./BM1688 ../data/models/BM1688

python3 -m dfss --url=open@sophgo.com:/sophon-stream/retinaface_distributor_resnet_faiss_converger/face_data.zip
unzip face_data.zip
rm -rf face_data.zip
mv ./face_data ../data/face_data

python3 -m dfss --url=open@sophgo.com:/sophon-stream/retinaface_distributor_resnet_faiss_converger/images.zip
unzip images.zip
rm -rf images.zip
mv ./images ../data/images

python3 -m dfss --url=open@sophgo.com:/sophon-stream/retinaface_distributor_resnet_faiss_converger/class.names
mv ./class.names ../data/

