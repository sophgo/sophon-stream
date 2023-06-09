cmake_minimum_required(VERSION 3.10)
project(sophon_stream)

set(CMAKE_CXX_STANDARD 14)
#set(CMAKE_BUILD_TYPE "Debug")

if (NOT DEFINED TARGET_ARCH)
    set(TARGET_ARCH pcie)
endif()

if (${TARGET_ARCH} STREQUAL "pcie")

    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}  -fprofile-arcs")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC -rdynamic")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -rdynamic")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -rdynamic")

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

    include_directories(../../share/3rdparty/gtest)
    include_directories(../../share/3rdparty/gtest/include)
    add_library(gtest ../../share/3rdparty/gtest/src/gtest-all.cc ../../share/3rdparty/gtest/src/gtest_main.cc)
    link_libraries(gtest)

    link_libraries(pthread)
    link_libraries(dl)

    link_directories(../../framework/build)
    link_libraries(ivslogger)

    include_directories(../../share)
    include_directories(../../share/3rdparty/spdlog/include)
    include_directories(../../share/3rdparty/nlohmann-json/include)
    include_directories(../../element/multimedia/decode/include)
    include_directories(../../framework/include)

    include_directories(../include)
    set(test_src
        src/usecase_yolov5.cc
    )
    get_filename_component(test_name ${test_src} NAME_WE)
    add_executable(${test_name} ${test_src})
    target_link_libraries(${test_name} -ldl ${OPENCV_LIBS} -lpthread -lavcodec -lavformat -lavutil -lframework)


elseif(${TARGET_ARCH} STREQUAL "soc")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}  -fprofile-arcs -ftest-coverage -g -rdynamic")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}  -fprofile-arcs -ftest-coverage -g -rdynamic")
    add_compile_options(-fPIC)
    set(CMAKE_C_COMPILER aarch64-linux-gnu-gcc)
    set(CMAKE_ASM_COMPILER aarch64-linux-gnu-gcc)
    set(CMAKE_CXX_COMPILER aarch64-linux-gnu-g++)
    set(BM_LIBS bmlib bmrt bmcv yuv)
    set(JPU_LIBS bmjpuapi bmjpulite)
    include_directories("${SDK}/include/")
    include_directories("${SDK}/include/opencv4")
    link_directories("${SDK}/lib/")
    include_directories(.)
    include_directories(../share)

    add_compile_options(-fPIC)
    set(CMAKE_C_COMPILER aarch64-linux-gnu-gcc)
    set(CMAKE_ASM_COMPILER aarch64-linux-gnu-gcc)
    set(CMAKE_CXX_COMPILER aarch64-linux-gnu-g++)
    set(BM_LIBS bmlib bmrt bmcv yuv)
    set(JPU_LIBS bmjpuapi bmjpulite)
    include_directories("${SDK}/include/")
    include_directories("${SDK}/include/opencv4")
    link_directories("${SDK}/lib/")

    include_directories(../../share/3rdparty/gtest)
    include_directories(../../share/3rdparty/gtest/include)
    add_library(gtest ../../share/3rdparty/gtest/src/gtest-all.cc ../../share/3rdparty/gtest/src/gtest_main.cc)
    link_libraries(gtest)
    set(OPENCV_LIBS opencv_imgproc opencv_core opencv_highgui opencv_imgcodecs opencv_videoio)
    link_libraries(pthread)
    link_libraries(dl)

    link_directories(../../framework/build_soc)
    link_libraries(ivslogger)

    include_directories(../../share)
    include_directories(../../share/3rdparty/spdlog/include)
    include_directories(../../share/3rdparty/nlohmann-json/include)
    include_directories(../../element/multimedia/decode/include)
    include_directories(../../framework/include)

    include_directories(../include)
    set(test_src
        src/usecase_yolov5.cc
    )
    get_filename_component(test_name ${test_src} NAME_WE)
    add_executable(${test_name} ${test_src})
    target_link_libraries(${test_name} -ldl ${OPENCV_LIBS} ${BM_LIBS} ${the_libbmcv.so} -fprofile-arcs -lgcov -lpthread -lavcodec -lavformat -lavutil -lframework)

endif()
