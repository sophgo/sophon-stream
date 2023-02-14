cmake_minimum_required(VERSION 3.10)
project(algorithmAPI)
set(CMAKE_CXX_STANDARD 14)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}  -fprofile-arcs -ftest-coverage")
SET(CMAKE_BUILD_TYPE "Debug")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -pthread")
set(CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS} -Xcompiler -fPIC")

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
    src/AlgorithmFactorySelector.cpp
    src/Algorithm.cpp)

