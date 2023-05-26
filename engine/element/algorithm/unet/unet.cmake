cmake_minimum_required(VERSION 3.10)
project(algorithmAPI)
set(CMAKE_CXX_STANDARD 14)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}  -fprofile-arcs -g")

if (NOT DEFINED TARGET_ARCH)
    set(TARGET_ARCH pcie)
endif()

if (${TARGET_ARCH} STREQUAL "pcie")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -pthread")
    set(CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS} -Xcompiler -fPIC")
    set(FFMPEG_DIR  /opt/sophon/sophon-ffmpeg-latest/lib/cmake)
    find_package(FFMPEG REQUIRED)
    include_directories(${FFMPEG_INCLUDE_DIRS})
    link_directories(${FFMPEG_LIB_DIRS})

    set(OpenCV_DIR  /opt/sophon/sophon-opencv-latest/lib/cmake/opencv4)
    find_package(OpenCV REQUIRED)
    include_directories(${OpenCV_INCLUDE_DIRS})
    link_directories(${OpenCV_LIB_DIRS})

    find_package(libsophon REQUIRED)
    include_directories(${LIBSOPHON_INCLUDE_DIRS})
    link_directories(${LIBSOPHON_LIB_DIRS})

    include_directories(../share)

    find_package(PkgConfig REQUIRED)

    # nlohmann/json https://github.com/nlohmann/json
    include_directories(../share/3rdparty/nlohmann-json/include)

    include_directories(../share)
    include_directories(../share/3rdparty/spdlog/include)
    # gtest includes and libraries for all tests
    include_directories(../share/3rdparty/gtest/ ../share/3rdparty/gtest/include src)

    add_library(unet SHARED
        element/algorithm/unet/UnetPre.cpp
        element/algorithm/unet/UnetPost.cpp
        element/algorithm/unet/UnetInference.cpp
        element/algorithm/unet/UnetSophgoContext.cpp
        element/algorithm/unet/UnetAlgorithm.cpp
        )
    target_link_libraries(unet ${FFMPEG_LIBS} ${OpenCV_LIBS} ${the_libbmlib.so} ${the_libbmrt.so} ${the_libbmcv.so} -lpthread)
elseif (${TARGET_ARCH} STREQUAL "soc")
    add_compile_options(-fPIC)
    set(CMAKE_C_COMPILER aarch64-linux-gnu-gcc)
    set(CMAKE_ASM_COMPILER aarch64-linux-gnu-gcc)
    set(CMAKE_CXX_COMPILER aarch64-linux-gnu-g++)
    set(BM_LIBS bmlib bmrt bmcv yuv)
    set(JPU_LIBS bmjpuapi bmjpulite)
    include_directories("${SDK}/include/")
    include_directories("${SDK}/include/opencv4")
    link_directories("${SDK}/lib/")
    include_directories(../share)
    include_directories(../share/3rdparty/spdlog/include)
    include_directories(../share/3rdparty/nlohmann-json/include)
    include_directories(../share/3rdparty/gtest/ ../share/3rdparty/gtest/include src)

    add_library(unet SHARED
        element/algorithm/unet/UnetPre.cpp
        element/algorithm/unet/UnetPost.cpp
        element/algorithm/unet/UnetInference.cpp
        element/algorithm/unet/UnetSophgoContext.cpp
        element/algorithm/unet/UnetAlgorithm.cpp
        )
    target_link_libraries(unet ${FFMPEG_LIBS} ${OpenCV_LIBS} ${BM_LIBS} ${JPU_LIBS} ${the_libbmlib.so} ${the_libbmrt.so} ${the_libbmcv.so} -lpthread)
endif()