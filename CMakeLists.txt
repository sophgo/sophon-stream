cmake_minimum_required(VERSION 3.10)
project(sophon-stream)
set(CMAKE_CXX_STANDARD 17)

if (NOT DEFINED TARGET_ARCH)
    set(TARGET_ARCH pcie)
endif()

if (${TARGET_ARCH} STREQUAL "soc")
    if (NOT DEFINED SOPHON_SDK_SOC)
        message(FATAL_ERROR "Please set SOPHON_SDK_SOC for building soc")
    endif()
endif()

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build/lib)
add_subdirectory(framework)

add_subdirectory(element/algorithm/yolox)
add_subdirectory(element/algorithm/yolov5)
add_subdirectory(element/algorithm/bytetrack)
add_subdirectory(element/algorithm/resnet)
add_subdirectory(element/algorithm/openpose)
add_subdirectory(element/algorithm/retinaface)

add_subdirectory(element/multimedia/decode)
add_subdirectory(element/multimedia/osd)
add_subdirectory(element/multimedia/encode)
add_subdirectory(element/tools/blank)
add_subdirectory(element/tools/distributor)
add_subdirectory(element/tools/converger)
add_subdirectory(element/tools/http_push)

add_subdirectory(3rdparty/freetype2)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/samples/yolov5_bytetrack_distributor_resnet_converger/build)
add_subdirectory(samples/yolov5_bytetrack_distributor_resnet_converger)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/samples/yolov5/build)
add_subdirectory(samples/yolov5)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/samples/yolox/build)
add_subdirectory(samples/yolox)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/samples/bytetrack/build)
add_subdirectory(samples/bytetrack)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/samples/resnet/build)
add_subdirectory(samples/resnet)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/samples/openpose/build)
add_subdirectory(samples/openpose)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/samples/yolox_bytetrack_osd_encode/build)
add_subdirectory(samples/yolox_bytetrack_osd_encode)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/samples/retinaface/build)
add_subdirectory(samples/retinaface)