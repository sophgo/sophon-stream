cmake_minimum_required(VERSION 3.10)
project(sophon-stream)

set(CMAKE_CXX_STANDARD 17)


if (NOT DEFINED TARGET_ARCH)
    set(TARGET_ARCH pcie)
endif()

if (${TARGET_ARCH} STREQUAL "pcie")

    set(FFMPEG_DIR  /opt/sophon/sophon-ffmpeg-latest/lib/cmake)
    find_package(FFMPEG REQUIRED)
    include_directories(${FFMPEG_INCLUDE_DIRS})
    link_directories(${FFMPEG_LIB_DIRS})

    set(OpenCV_DIR  /opt/sophon/sophon-opencv-latest/lib/cmake/opencv4)
    find_package(OpenCV REQUIRED)
    include_directories(${OpenCV_INCLUDE_DIRS})
    link_directories(${OpenCV_LIB_DIRS})

    set(OPENCV_LIBS opencv_imgproc opencv_core opencv_highgui opencv_imgcodecs opencv_videoio)

    set(LIBSOPHON_DIR  /opt/sophon/libsophon-current/data/libsophon-config.cmake)
    find_package(LIBSOPHON REQUIRED)
    include_directories(${LIBSOPHON_INCLUDE_DIRS})
    link_directories(${LIBSOPHON_LIB_DIRS})

    link_directories(../../../build/lib)

    link_libraries(pthread)

    include_directories(../../../framework)
    include_directories(../../../framework/include)

    include_directories(../../../3rdparty/websocketpp)
    include_directories(../../../3rdparty/spdlog/include)
    include_directories(../../../3rdparty/nlohmann-json/include)
    include_directories(../../../3rdparty/httplib)

    include_directories(include)

    add_executable(server src/http_struct_utils.cc src/http_interact_mgr.cc src/server.cc)
    target_link_libraries(server ${OPENCV_LIBS} -lpthread -livslogger)

elseif(${TARGET_ARCH} STREQUAL "soc")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}  -fprofile-arcs -ftest-coverage -g -rdynamic")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}  -fprofile-arcs -ftest-coverage -rdynamic")
    add_compile_options(-fPIC)
    set(CMAKE_C_COMPILER aarch64-linux-gnu-gcc)
    set(CMAKE_ASM_COMPILER aarch64-linux-gnu-gcc)
    set(CMAKE_CXX_COMPILER aarch64-linux-gnu-g++)
    set(BM_LIBS bmlib bmrt bmcv yuv)
    set(JPU_LIBS bmjpuapi bmjpulite)
    include_directories("${SOPHON_SDK_SOC}/include/")
    include_directories("${SOPHON_SDK_SOC}/include/opencv4")
    link_directories("${SOPHON_SDK_SOC}/lib/")
    include_directories(.)

    link_libraries(pthread)

    link_directories(../../../build/lib/)
    include_directories(../../../3rdparty/websocketpp)
    include_directories(../../../3rdparty/spdlog/include)
    include_directories(../../../3rdparty/nlohmann-json/include)
    include_directories(../../../3rdparty/httplib)

    include_directories(../../../framework)
    include_directories(../../../framework/include)

    include_directories(include)
    add_executable(server src/http_struct_utils.cc src/http_interact_mgr.cc src/server.cc)
    target_link_libraries(server -lpthread -livslogger)

endif()
