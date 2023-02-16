link_libraries(pthread)

# spdlog https://github.com/gabime/spdlog
include_directories(../share/3rdparty/spdlog/include)

# nlohmann/json https://github.com/nlohmann/json
include_directories(../share/3rdparty/nlohmann-json/include)

# gtest includes and libraries for all tests
include_directories(../share/3rdparty/gtest/ ../share/3rdparty/gtest/include src)

add_library(gtest ../share/3rdparty/gtest/src/gtest-all.cc ../share/3rdparty/gtest/src/gtest_main.cc)
link_libraries(gtest)

# boost
find_path(BOOST_INCLUDE_DIR boost/cstdlib.hpp PATHS /usr/local/include)
include_directories(${BOOST_INCLUDE_DIR})
find_library(BOOST_FS_LIBRARIES boost_filesystem)
find_library(BOOST_SYS_LIBRARIES boost_system)
find_library(BOOST_RE_LIBRARIES boost_regex)
find_library(BOOST_PROGRAM_OPTIONS boost_program_options)
link_libraries(${BOOST_FS_LIBRARIES} ${BOOST_SYS_LIBRARIES} ${BOOST_RE_LIBRARIES} ${BOOST_PROGRAM_OPTIONS})

# libyaml-cpp
#pkg_check_modules(YAMLCPP REQUIRED yaml-cpp)
#include_directories(${YAMLCPP_INCLUDE_DIRS})
#link_libraries(${YAMLCPP_LIBRARIES})
#add_compile_options(${YAMLCPP_CFLAGS_OTHER})

# libbson
#pkg_check_modules(LIBBSON REQUIRED libbson-1.0)
#include_directories(${LIBBSON_INCLUDE_DIRS})
#link_libraries(${LIBBSON_LIBRARIES})
#add_compile_options(${LIBBSON_CFLAGS_OTHER})


# cuda
#if(GPU_VERSION)
#    find_path(CUDAROOT NAMES include/cuda.h PATHS /usr/local/cuda)
#    set(CUDA_INCLUDE_DIRS ${CUDAROOT}/include)
#    find_library(CUDA cuda PATHS ${CUDAROOT}/lib64/stubs)
#    find_library(CUDA_RT cudart PATHS ${CUDAROOT}/lib64)
#    find_library(NVCUVID nvcuvid)
#    find_library(CUBLAS cublas PATHS ${CUDAROOT}/lib64)
#    set(CUDA_LIBRARIES ${CUDA} ${CUDA_RT} ${NVCUVID} ${CUBLAS})
#    include_directories(${CUDA_INCLUDE_DIRS})
#    link_libraries(${CUDA_LIBRARIES})
#endif(GPU_VERSION)

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

#mkl
#find_path(MKLROOT NAMES include/mkl.h PATHS /opt/intel/mkl)
#set(MKL_INCLUDE_DIRS ${MKLROOT}/include)
#find_library(MKL mkl_rt PATHS ${MKLROOT}/lib/intel64)
#set(MKL_LIBRARIES ${MKL})
#include_directories(${MKL_INCLUDE_DIRS})
#link_libraries(${MKL_LIBRARIES})

# eigen3
find_path(EIGEN3_INCLUDE_DIR Eigen/Eigen PATHS /usr/local/include/eigen3 /usr/include/eigen3)
include_directories(${EIGEN3_INCLUDE_DIR})

#if(GPU_VERSION)
## nvml
#link_libraries(nvidia-ml)
#endif(GPU_VERSION)

link_libraries(dl)

include_directories(src)

add_library(common
    ../share/common/Logger.cpp
    ../share/common/Udp.cpp
    )
link_libraries(common)

#add_library(gpuCode
#    ../share/common/streamProcess/Resize.cu
#    ../share/common/streamProcess/yuv2bgr.cu
#    ../share/common/streamProcess/bgr2nv12.cu
#    )
#link_libraries(gpuCode)
