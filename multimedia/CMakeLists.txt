cmake_minimum_required(VERSION 3.10)
project(multimedia)
set(CMAKE_CXX_STANDARD 14)

option(LYN_VERSION "use lyn implementation" OFF)
option(GPU_VERSION "use gpu implementation" OFF)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/config.h.in"
    "${CMAKE_CURRENT_SOURCE_DIR}/config.h" @ONLY)

if(GPU_VERSION)
    enable_language(CUDA)
    set(CMAKE_CUDA_STANDARD 11)
endif(GPU_VERSION)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}  -fprofile-arcs -ftest-coverage")
SET(CMAKE_BUILD_TYPE "Debug")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -pthread")
set(CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS} -Xcompiler -fPIC")
#include_directories(.)
include_directories(../share)

find_package(PkgConfig REQUIRED)

# nlohmann/json https://github.com/nlohmann/json
include_directories(../share/3rdparty/nlohmann-json/include)

# ffmpeg
pkg_check_modules(FFMPEG REQUIRED libavcodec libavutil libavformat libswscale libavfilter libswresample)
include_directories(${FFMPEG_INCLUDE_DIRS})
link_libraries(${FFMPEG_LIBRARIES})
add_compile_options(${FFMPEG_CFLAGS_OTHER})

# opencv
pkg_check_modules(OPENCV REQUIRED opencv)
include_directories(${OPENCV_INCLUDE_DIRS})
link_libraries(${OPENCV_LIBRARIES})
add_compile_options(${OPENCV_CFLAGS_OTHER})

# boost
find_path(BOOST_INCLUDE_DIR boost/cstdlib.hpp PATHS /usr/local/include)
include_directories(${BOOST_INCLUDE_DIR})
find_library(BOOST_FS_LIBRARIES boost_filesystem)
find_library(BOOST_SYS_LIBRARIES boost_system)
find_library(BOOST_RE_LIBRARIES boost_regex)
find_library(BOOST_PROGRAM_OPTIONS boost_program_options)
link_libraries(${BOOST_FS_LIBRARIES} ${BOOST_SYS_LIBRARIES} ${BOOST_RE_LIBRARIES} ${BOOST_PROGRAM_OPTIONS})

# libbson
#pkg_check_modules(LIBBSON REQUIRED libbson-1.0)
#include_directories(${LIBBSON_INCLUDE_DIRS})
#link_libraries(${LIBBSON_LIBRARIES})
#add_compile_options(${LIBBSON_CFLAGS_OTHER})

include_directories(.)
include_directories(../share)
include_directories(../share/3rdparty/spdlog/include)
# gtest includes and libraries for all tests
include_directories(../share/3rdparty/gtest/ ../share/3rdparty/gtest/include src)

add_library(gtest ../share/3rdparty/gtest/src/gtest-all.cc ../share/3rdparty/gtest/src/gtest_main.cc)
link_libraries(gtest)

add_library(ivslogger ../share/common/Udp.cpp
    ../share/common/Logger.cpp)
link_libraries(ivslogger)

add_library(multiMediaApi SHARED
    src/MultiMediaFactorySelector.cpp
    src/MultiMedia.cpp
    #            src/context/GpuContext.cpp
    #            src/process/GpuDecode.cpp
    #            src/factory/GpuDecodeFactory.cpp
    #            src/process/decode/gpu/FFmpegHwDevice-new.cpp
    #            src/process/decode/gpu/FFmpegDecoder-new.cpp
    #            src/process/decode/gpu/FFmpegFormatInput-new.cpp
    #            src/process/decode/gpu/yuv2bgr.cu
    #            src/process/decode/gpu/Resize.cu
    )

enable_testing()
set(CMAKE_BUILD_TYPE Debug)

# unit tests
file(GLOB TEST_SOURCES  test/unitcase/*.cpp test/unitcase/*.c)
foreach(test_src ${TEST_SOURCES})
    get_filename_component(test_name ${test_src} NAME_WE)
    add_executable(${test_name} ${test_src})
    TARGET_LINK_LIBRARIES(${test_name} multiMediaApi)
    add_test(NAME ${test_name} COMMAND ${test_name} --gtest_output=xml:test-reports/${test_name}.xml)
endforeach(test_src)

# usecase tests
file(GLOB TEST_SOURCES test/usecase/*.cpp test/usecase/*.c)
foreach(test_src ${TEST_SOURCES})
    get_filename_component(test_name ${test_src} NAME_WE)
    add_executable(${test_name} ${test_src})
endforeach(test_src)




