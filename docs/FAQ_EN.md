# Usage Issues and Answers

English | [简体中文](FAQ.md)

#### 1. Error during initialization phase: `Can not find element maker, name: xxx`

This error occurs because you have modified the "name" field in the JSON file of an element. This "name" field is used by the Factory to identify elements and should remain unchanged.

#### 2. Compilation error: `fatal error: boost/version.hpp: No such file or directory`
Please use the following command to install the library:
```bash
sudo apt-get update
sudo apt-get install libboost-all-dev
```

#### 3. Error when running on the box after cross-compilation: `undefined symbol: _ZN5boost6system15system_categoryEv`

First, check the version of the boost library with the command: `dpkg -S /usr/include/boost/version.hpp`. Version 1.71 should compile and run correctly, while older versions may result in the above error.

If it's inconvenient to install the specific version of boost, you can use our provided [Docker image for compilation](./HowToMake.md#compiling-using-development-image).

#### 4. Error when running on the box after cross-compilation: `/lib/aarch64-linux-gnu/libc.so.6: version 'GLIBC_2.33' not found`

This error occurs because the version of the cross-compiler toolchain on your host is too high. You can resolve it by reinstalling with the following commands:
```bash
sudo apt remove cpp-*-aarch64-linux-gnu
sudo apt-get install gcc-7-aarch64-linux-gnu g++-7-aarch64-linux-gnu
sudo ln -s /usr/bin/aarch64-linux-gnu-gcc-7 /usr/bin/aarch64-linux-gnu-gcc
sudo ln -s /usr/bin/aarch64-linux-gnu-g++-7 /usr/bin/aarch64-linux-gnu-g++
```

If you prefer not to install the specific version of the cross-compiler, you can use our provided [Docker image for compilation](./HowToMake.md#compiling-using-development-image).

#### 5. Error during compilation: `Unable to find definitions related to tpu_kernel`

To enable the tpu_kernel related functionality, you need SDK 03.01 or later. You can refer to the official website for upgrading instructions.

#### 6. Error when running on the box after cross-compilation: `error while loading shared libraries: libxxx.so: cannot open shared object file: No such file or directory`

First, check if the `lib` directory under `sophon-stream/build` contains the required `.so` file. If it's missing, you need to transfer it from the host where cross-compilation was done to the box. If the file path is correct but still cannot be found, execute the following command:

```bash
export LD_LIBRARY_PATH=path-to/sophon-stream/build/lib/:$LD_LIBRARY_PATH
```

#### 7. Compilation error: `Unable to find -lframework, -livslogger`

This error typically occurs when the compilation path is incorrect, such as compiling within the `element` or `samples` directory. You need to return to the `sophon-stream` project directory for compilation.

#### 8. Runtime error: `[BM_CHECK][error] BM_CHECK_RET fail`, detailed error as follows:
```
[bmlib_memory][error] bm_alloc_gmem failed, dev_id = 0, size = 0x20
[BM_CHECK][error] BM_CHECK_RET fail /workspace/libsophon/bmlib/src/bmlib_memory.cpp: bm_malloc_device_byte_heap_mask: 705
MAT Allocate Err: dims = 2, size = [1, 8], type = 5
terminate called after throwing an instance of 'cv::Exception'
[bmlib_memory][error] bm_alloc_gmem failed, dev_id = 0, size = 0x10
  what():  OpenCV(4.1.0) /workspace/middleware-soc/bm_opencv/modules/core/src/matrix.cpp:448: error: (-215:Assertion failed) u != 0 in function 'create'

[BM_CHECK][error] BM_CHECK_RET fail /workspace/libsophon/bmlib/src/bmlib_memory.cpp: bm_malloc_device_byte_heap_mask: 705
Aborted (core dumped)
```

This may be caused by a limit on the number of open file handles in Linux, where some devices have a default limit of 1024. You can try increasing it to 20480 with the following command:

```bash
ulimit -n 20480
```

#### 9. Some pictures are not recognized/detected.

It is normal that there are a few false detections in pictures/videos, because the original model's accuracy cannot reach 100%. Now stream does not provide accuracy evaluation function for the time being, so it is enough to observe that most of the pictures are recognized/detected normally.