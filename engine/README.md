# IVS Engine development guide

## Build project for debugging

Install basic depends

    sudo apt-get install -y build-essential cmake git pkg-config
    sudo apt-get install -y libgtk2.0-dev libbson-dev libyaml-cpp-dev

Download and install cuda

    # download link from our wiki
    # http://192.168.8.64:8801/display/IVS/Linux

    ./cuda_9.2.148_396.37_linux.run

Download and install mkl

    # download link from our wiki
    # http://192.168.8.64:8801/display/IVS/Linux

    tar zxvf l_mkl_2017.3.196.tgz
    cd l_mkl_2017.3.196/
    sudo ./install.sh
    cat | sudo tee /etc/ld.so.conf.d/intel-mkl.conf <<-EOF
    /opt/intel/mkl/lib/intel64
    /opt/intel/lib/intel64
    EOF
    sudo ldconfig -v
    /opt/intel/mkl/bin/mklvars.sh intel64 mod

Download and install protobuf-c

    # download link from github
    # https://github.com/protobuf-c/protobuf-c/releases/download/v1.3.1/protobuf-c-1.3.1.tar.gz
    tar zxvf protobuf-c-1.3.1.tar.gz
    cd protobuf-c-1.3.1/
    ./configure --disable-protoc
    make
    sudo make install

Download and install cnats

    # download link from github
    # https://github.com/nats-io/cnats/archive/v1.8.0.tar.gz
    tar zxvf cnats-1.8.0.tar.gz
    cd cnats-1.8.0/
    mkdir build
    cd build/
    cmake ..
    make
    sudo make install

Download and install mxnet

    sudo apt-get install -y libopenblas-dev
    #wget https://github.com/apache/incubator-mxnet/releases/download/1.4.0.rc0/apache-mxnet-src-1.4.0.rc0-incubating.tar.gz
    tar zxf apache-mxnet-src-1.4.0.rc0-incubating.tar.gz
    cd apache-mxnet-src-1.4.0.rc0-incubating
    make USE_CPP_PACKAGE=1 USE_BLAS=openblas USE_CUDA=1 USE_CUDA_PATH=/usr/local/cuda USE_CUDNN=1 USE_F16C=0 -j`nproc`
    sudo mkdir -p /usr/local/mxnet
    sudo cp -rf include lib /usr/local/mxnet

Now you could import project to Clion or other editors for development.
Run-time environment OMP_NUM_THREADS=1

Download and install cares

    # download link
    # https://c-ares.haxx.se/download/c-ares-1.14.0.tar.gz
    tar xvf c-ares-1.14.0.tar.gz 
    cd c-ares-1.14.0/
    ./configure 
    make
    sudo make install

Download and install protobuf-cpp

    # download link from github
    # https://github.com/protocolbuffers/protobuf/releases/download/v3.6.0/protobuf-cpp-3.6.0.tar.gz
    tar xvf protobuf-cpp-3.6.0.tar.gz 
    cd protobuf-3.6.0/
    make
    sudo make install

Download and install grpc

    # download link from github
    # https://codeload.github.com/grpc/grpc/tar.gz/v1.19.0
    tar xvf v1.19.0
    cd grpc-1.19.0/
    make
    sudo make install

If modify FaceRecognition.proto, run following:

    cd src/plugins/faceRecognition/common/
    protoc  --cpp_out=. Recognition.proto
    protoc --plugin=protoc-gen-grpc=`which grpc_cpp_plugin` --grpc_out=. Recognition.proto

## Mount file storage

    sudo add-apt-repository ppa:gluster/glusterfs-3.11
    sudo apt-get update
    sudo apt-get install glusterfs-client

    sudo vi /etc/hosts
    192.168.8.12	lynxi-ivs-master

    # mount volumes in ivs-engine workdir
    cd ivs-engine/build
    mkdir -p ../volumes

    echo "lynxi-ivs-master:/ivs $PWD/../volumes glusterfs defaults,_netdev 0 0" | sudo tee -a /etc/fstab
    sudo mount -a

    #before switch server or system rebooted
    sudo umount lynxi-ivs-master:/ivs
    sudo mount -a

## Testing

Run `make test` to do unit and use case tests.

    make test

Use lcov to get code coverage report

    lcov --directory . --capture --output-file x.info

    (or lcov -d . -c -o x.info)

prune the report

    lcov -r x.info '*/include/*' '*3rdparty/*' -o coverage-result.info

convert the report to html format

    genhtml -o coverage-result coverage-result.info

open the html report

    xdg-open coverage-result/index.html

Google Test Samples

* https://github.com/google/googletest/tree/master/googletest/samples

## Code styles

Please run this once for auto format code before commit.

    cp utils/code-styles/pre-commit .git/hooks/pre-commit
