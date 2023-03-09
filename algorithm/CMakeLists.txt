cmake_minimum_required(VERSION 3.10)
project(algorithmAPI)
set(CMAKE_CXX_STANDARD 14)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}  -fprofile-arcs -ftest-coverage")
SET(CMAKE_BUILD_TYPE "Debug")

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

add_library(gtest ../share/3rdparty/gtest/src/gtest-all.cc ../share/3rdparty/gtest/src/gtest_main.cc)
link_libraries(gtest)

add_library(Logger ../share/common/Udp.cpp ../share/common/Logger.cpp)
link_libraries(Logger)

add_library(algorithmApi SHARED
    src/post_process/NoopPostprocess.cpp
    src/post_process/Yolov5Post.cpp
    src/pre_process/Yolov5Pre.cpp
    src/inference/Yolov5Inference.cpp
    src/factory/Yolov5Factory.cpp
    src/context/SophgoContext.cpp
    src/AlgorithmFactorySelector.cpp
    src/Algorithm.cpp)
target_link_libraries(algorithmApi ${FFMPEG_LIBS} ${OpenCV_LIBS} ${the_libbmlib.so} ${the_libbmrt.so} ${the_libbmcv.so} -lpthread)
