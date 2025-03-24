# HowToMake

English | [简体中文](HowToMake.md)

- [HowToMake](#howtomake)
  - [Building Using Development Docker Image](#building-using-development-docker-image)
  - [x86/arm PCIe Platform](#x86arm-pcie-platform)
  - [SoC Platform](#soc-platform)
  - [Compilation Results](#compilation-results)

## Building Using Development Docker Image

* In principle, stream compilation does not strongly depend on docker images;
* If you are using a host with a partially incompatible environment and it is not convenient to change the local environment, you can use the docker image we provide for compilation;
* Please note that you should not mix the stream_dev image below with the tpuc_dev image for model compilation.

If your local environment is partially incompatible and it's inconvenient to change it, you can use the Docker image we provide for compiling.

Download the image through dfss:

```bash
pip3 install dfss
python3 -m dfss --url=open@sophgo.com:/sophon-stream/docker/stream_dev.tar
```

If the device is bm1688/cv186ah, please pull this image:

```bash
pip3 install dfss
python3 -m dfss --url=open@sophgo.com:/sophon-stream/docker/stream_dev_22.04.tar
```

If you're using Docker for the first time, you can execute the following commands to install and configure it (only for the first-time setup):

```bash
sudo apt install docker.io
sudo systemctl start docker
sudo systemctl enable docker
sudo groupadd docker
sudo usermod -aG docker $USER
newgrp docker
```

Load the image from the downloaded image directory:

```bash
docker load -i stream_dev.tar
```

You can check the loaded image using `docker images`, which defaults to `stream_dev:latest`.

Create a container:

```bash
docker run --privileged --name stream_dev -v $PWD:/workspace -it stream_dev:latest
# stream_dev is just an example name; please specify your own container name.
```

The `workspace` directory in the container will be mounted to the directory of the host machine where you ran `docker run`. You can use this container to compile the project.

## x86/arm PCIe Platform
```bash
mkdir build
cd build
cmake ..
make -j4
```
If QT is needed, please download QT with official version first. Or, QT element will not be compiled.

Download QT with command:
```bash
sudo apt install qtbase5-dev
```

If https protocol is needed in http_push unit, please download OpenSSL with official version first. Or, https support will not be compiled.

Download OpenSSL with command
```bash
sudo apt install libssl-dev
```



## SoC Platform
Usually, when cross-compiling programs on an x86 host, you need to set up a cross-compilation environment using the SOPHON SDK on the x86 host. You will package the required header files and library files into the `sophon_sdk_soc` directory. You can either download the SOPHON SDK and package it yourself or download our pre-packaged files (choose one based on your SOC environment).

The following files correspond to the v23.03.01, v23.05.01, v23.07.01, v23.10.01 and v25.03.01 versions of the official SDK, for BM1684/BM1684X.

```bash
pip3 install dfss
python3 -m dfss --url=open@sophgo.com:/sophon-stream/soc-sdk/soc0301.tar.gz
python3 -m dfss --url=open@sophgo.com:/sophon-stream/soc-sdk/soc0501.tar.gz
python3 -m dfss --url=open@sophgo.com:/sophon-stream/soc-sdk/soc0701.tar.gz
python3 -m dfss --url=open@sophgo.com:/sophon-stream/soc-sdk/soc1001.tar.gz
python3 -m dfss --url=open@sophgo.com:/soc-sdk-allin/v25.03.01/soc-sdk-allin.tgz
```

For BM1688, these files are available:
```bash
pip3 install dfss
python3 -m dfss --url=open@sophgo.com:/sophon-stream/soc-sdk/1688_1.7.tar.gz
python3 -m dfss --url=open@sophgo.com:/sophon-stream/soc-sdk/1688_1.8.tar.gz
python3 -m dfss --url=open@sophgo.com:/sophon-stream/soc-sdk/1688_1.9.tar.gz
```

If QT is needed, make sure to download QT of cross-compiling version on x86 device. The environment on the edge device does have QT already, there is no need to re-download/install it. Use the `QTPATH` parameter at compile time to specify the qt path: (If not using QT, this part can be ignored and `QTPATH` while cross-compiling is not necessary)

For BM1684/BM1684X:
```bash
python3 -m dfss --url=open@sophgo.com:sophon-demo/MultiYolov5/qt-5.14-amd64-aarch64-fl2000fb_v1.1.0.tar.xz
```

For BM1688/CV186AH:
```bash
python3 -m dfss --url=open@sophgo.com:sophon-pipeline/a2_bringup/qtbase.zip
```

If https is needed in http_push unit, make sure to download OPENSSL of cross-compiling version on x86 device. Use the `OPENSSL_PATH` parameter at compile time to specify the openssl path: (If not using https, this part can be ignored and `OPENSSL_PATH` while cross-compiling is not necessary)
```bash
python3 -m dfss --dflag=openssl_1.1.1f_aarch64
```

Use command below to get newer openssl for SDK 1.9 or after.
```bash
python3 -m dfss --url=open@sophgo.com:sophon-stream/soc-sdk/openssl_3_aarch64.tar.gz
```

When cross-compiling, make sure to provide the absolute path to `SOPHON_SDK_SOC`, `QTPATH` and `OPENSSL_PATH`:

```bash
mkdir build
cd build
cmake ../ -DTARGET_ARCH=soc -DSOPHON_SDK_SOC=/path/to/sophon_sdk_soc -DQTPATH=/path/to/qt -DOPENSSL_PATH=/path/to/openssl
make -j4
```

If the required SDK version was not provided in the previous sections, you'll need to package the `soc-sdk` by following this process. Note that for SE9 devices, sophon-mw needs to be changed to sophon-media.

1. Unzip the contents of the `libsophon_soc_<x.y.z>_aarch64.tar.gz` file located in the `sophon-img` directory of the SDK. Copy all the contents of the `lib` and `include` folders to your `soc-sdk` directory.

```bash
cd sophon-img_<date>_<hash>
# Create a root directory for dependency files
mkdir -p soc-sdk
# Unzip the 'libsophon_soc_${x.y.z}_aarch64.tar.gz' from the 'sophon-img' release package, where x.y.z is the version number.
tar -zxf libsophon_soc_<x.y.z>_aarch64.tar.gz
# Copy the relevant library directories and header files to the root directory of the dependency files
cp -rf libsophon_soc_<x.y.z>_aarch64/opt/sophon/libsophon-<x.y.z>/lib ${soc-sdk}
cp -rf libsophon_soc_<x.y.z>_aarch64/opt/sophon/libsophon-<x.y.z>/include ${soc-sdk}
```

2. Unzip the contents of the `sophon-mw-soc_<x.y.z>_aarch64.tar.gz` file located in the `sophon-mw` directory of the SDK. Copy all the contents of the `lib` and `include` folders under `sophon-mw` to your `soc-sdk` directory.

```bash
cd sophon-mw_<date>_<hash>
# Unzip the 'sophon-mw-soc_<x.y.z>_aarch64.tar.gz' from the 'sophon-mw' package, where x.y.z is the version number.
tar -zxf sophon-mw-soc_<x.y.z>_aarch64.tar.gz
# Copy the library directories and header files of ffmpeg and OpenCV to the root directory of the dependency files.
cp -rf sophon-mw-soc_<x.y.z>_aarch64/opt/sophon/sophon-ffmpeg_<x.y.z>/lib ${soc-sdk}
cp -rf sophon-mw-soc_<x.y.z>_aarch64/opt/sophon/sophon-ffmpeg_<x.y.z>/include ${soc-sdk}
cp -rf sophon-mw-soc_<x.y.z>_aarch64/opt/sophon/sophon-opencv_<x.y.z>/lib ${soc-sdk}
cp -rf sophon-mw-soc_<x.y.z>_aarch64/opt/sophon/sophon-opencv_<x.y.z>/include ${soc-sdk}
```

## Compilation Results

1. `framework` and `element` will generate dynamic link libraries in `build/lib`.

2. `samples` will generate executable files in the `samples/build` folder .

For PCIe platforms, you can run tests directly on the PCIe platform. For SoC platforms, you'll need to copy the dynamically linked libraries and executable files generated by cross-compilation to the SoC platform for testing.

3. After cross-compilation, when copying the compiled results to your Micro Server, make sure to maintain the directory structure. You can use the following command to directly copy the `sophon-stream` directory from your host to your Micro Server:

```bash
scp -r ./sophon-stream linaro@<your ip>:<your path>
```

You can set the IP address and file directory of your Micro Server as per your specific situation.

4. Log into your Micro Server and add an environment variable to ensure that the dynamic link libraries can be found when running programs in `sample`:
```bash
echo 'export LD_LIBRARY_PATH=<your path>/sophon-stream/build/lib/:$LD_LIBRARY_PATH' >> ~/.bashrc
source ~/.bashrc
```

Replace `<your path>` with the absolute path to `sophon-stream` on your Micro Server.
