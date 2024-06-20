cmake_minimum_required(VERSION 3.10)
project(algorithmAPI)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}  -fprofile-arcs -g")

if (NOT DEFINED TARGET_ARCH)
    set(TARGET_ARCH pcie)
endif()

if (${TARGET_ARCH} STREQUAL "pcie")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -pthread -fpermissive")

    set(FFMPEG_DIR  /opt/sophon/sophon-ffmpeg-latest/lib/cmake)
    find_package(FFMPEG REQUIRED)
    include_directories(${FFMPEG_INCLUDE_DIRS})
    link_directories(${FFMPEG_LIB_DIRS})

    set(OpenCV_DIR  /opt/sophon/sophon-opencv-latest/lib/cmake/opencv4)
    find_package(OpenCV REQUIRED)
    include_directories(${OpenCV_INCLUDE_DIRS})
    link_directories(${OpenCV_LIB_DIRS})

    set(LIBSOPHON_DIR  /opt/sophon/libsophon-current/data/libsophon-config.cmake)
    find_package(LIBSOPHON REQUIRED)
    include_directories(${LIBSOPHON_INCLUDE_DIRS})
    link_directories(${LIBSOPHON_LIB_DIRS})

    set(BM_LIBS bmlib bmrt bmcv yuv)
    find_library(BMJPU bmjpuapi)
    if(BMJPU)
        set(JPU_LIBS bmjpuapi bmjpulite)
    endif()

    include_directories(../)
    include_directories(../../../framework)
    include_directories(../../../framework/include)

    include_directories(../../../3rdparty/spdlog/include)
    include_directories(../../../3rdparty/nlohmann-json/include)
    include_directories(../../../3rdparty/httplib)

    include_directories(include)
    add_library(yolov5 SHARED
        src/yolov5_pre_process.cc
        src/yolov5_post_process.cc
        src/yolov5_inference.cc
        src/yolov5.cc
    )

    target_link_libraries(yolov5 ${FFMPEG_LIBS} ${OpenCV_LIBS} ${BM_LIBS} ${JPU_LIBS} -lpthread)

elseif (${TARGET_ARCH} STREQUAL "soc")
    add_compile_options(-fPIC)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}  -fprofile-arcs -ftest-coverage -g -rdynamic")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}  -fprofile-arcs -ftest-coverage -rdynamic -fpermissive")
    set(CMAKE_C_COMPILER aarch64-linux-gnu-gcc)
    set(CMAKE_ASM_COMPILER aarch64-linux-gnu-gcc)
    set(CMAKE_CXX_COMPILER aarch64-linux-gnu-g++)

    include_directories("${SOPHON_SDK_SOC}/include/")
    include_directories("${SOPHON_SDK_SOC}/include/opencv4")
    link_directories("${SOPHON_SDK_SOC}/lib/")
    set(BM_LIBS bmlib bmrt bmcv yuv)
    find_library(BMJPU bmjpuapi)
    if(BMJPU)
        set(JPU_LIBS bmjpuapi bmjpulite)
    endif()

    include_directories(../)
    include_directories(../../../framework)
    include_directories(../../../framework/include)

    include_directories(../../../3rdparty/spdlog/include)
    include_directories(../../../3rdparty/nlohmann-json/include)
    include_directories(../../../3rdparty/httplib)

    include_directories(include)
    add_library(yolov5 SHARED
        src/yolov5_pre_process.cc
        src/yolov5_post_process.cc
        src/yolov5_inference.cc
        src/yolov5.cc
    )
    target_link_libraries(yolov5 ${FFMPEG_LIBS} ${OpenCV_LIBS} ${BM_LIBS} ${JPU_LIBS} -fprofile-arcs -lgcov -lpthread)
endif()
