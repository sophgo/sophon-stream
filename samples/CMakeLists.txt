cmake_minimum_required(VERSION 3.10)
project(sophon_stream)

set(CMAKE_CXX_STANDARD 14)
#set(CMAKE_BUILD_TYPE "Debug")

if (NOT DEFINED TARGET_ARCH)
    set(TARGET_ARCH pcie)
endif()

if (${TARGET_ARCH} STREQUAL "pcie")

    include_directories(.)
    include_directories(../share)

    #set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}  -fprofile-arcs -ftest-coverage")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}  -fprofile-arcs")

    find_package(PkgConfig REQUIRED)

    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC -rdynamic")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -rdynamic")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -rdynamic")

    # include(framework/framework.cmake)
    # include(element/element.cmake)

    link_directories("../engine/build")
    include_directories("../engine")


    link_libraries(pthread)

    # spdlog https://github.com/gabime/spdlog
    include_directories(../share/3rdparty/spdlog/include)

    # nlohmann/json https://github.com/nlohmann/json
    include_directories(../share/3rdparty/nlohmann-json/include)

    # gtest includes and libraries for all tests
    include_directories(../share/3rdparty/gtest/ ../share/3rdparty/gtest/include src)

    # add_library(gtest ../share/3rdparty/gtest/src/gtest-all.cc ../share/3rdparty/gtest/src/gtest_main.cc)
    link_libraries(gtest)

    # ffmpeg
    set(FFMPEG_DIR  /opt/sophon/sophon-ffmpeg-latest/lib/cmake)
    find_package(FFMPEG REQUIRED)
    include_directories(${FFMPEG_INCLUDE_DIRS})
    link_directories(${FFMPEG_LIB_DIRS})

    # opencv
    set(OpenCV_DIR  /opt/sophon/sophon-opencv-latest/lib/cmake/opencv4)
    find_package(OpenCV REQUIRED)
    include_directories(${OpenCV_INCLUDE_DIRS})
    link_directories(${OpenCV_LIB_DIRS})

    find_package(libsophon REQUIRED)
    include_directories(${LIBSOPHON_INCLUDE_DIRS})
    link_directories(${LIBSOPHON_LIB_DIRS})
    set(OPENCV_LIBS opencv_imgproc opencv_core opencv_highgui opencv_imgcodecs opencv_videoio)

    link_libraries(dl)

    # include_directories(src)

    # add_library(common
    #     ../share/common/Logger.cpp
    #     )

    link_libraries(common)
    link_libraries(ivslogger)
    link_libraries(framework)

    # usecase tests
    file(GLOB TEST_SOURCES usecase/*.cpp usecase/*.c)
    foreach(test_src ${TEST_SOURCES})
        get_filename_component(test_name ${test_src} NAME_WE)
        add_executable(${test_name} ${test_src})
        target_link_libraries(${test_name} -ldl ${OPENCV_LIBS} -lpthread -lavcodec -lavformat -lavutil)
    endforeach(test_src)


elseif(${TARGET_ARCH} STREQUAL "soc")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}  -fprofile-arcs -g -rdynamic")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}  -fprofile-arcs -g -rdynamic")
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

    # include(framework/framework.cmake)
    # include(element/element.cmake)

    link_directories("../engine/build")
    include_directories("../engine")

    add_compile_options(-fPIC)
    set(CMAKE_C_COMPILER aarch64-linux-gnu-gcc)
    set(CMAKE_ASM_COMPILER aarch64-linux-gnu-gcc)
    set(CMAKE_CXX_COMPILER aarch64-linux-gnu-g++)
    set(BM_LIBS bmlib bmrt bmcv yuv)
    set(JPU_LIBS bmjpuapi bmjpulite)
    include_directories("${SDK}/include/")
    include_directories("${SDK}/include/opencv4")
    link_directories("${SDK}/lib/")
    link_libraries(pthread)

    # spdlog https://github.com/gabime/spdlog
    include_directories(../share/3rdparty/spdlog/include)

    # nlohmann/json https://github.com/nlohmann/json
    include_directories(../share/3rdparty/nlohmann-json/include)

    # gtest includes and libraries for all tests
    include_directories(../share/3rdparty/gtest/ ../share/3rdparty/gtest/include src)

    # add_library(gtest ../share/3rdparty/gtest/src/gtest-all.cc ../share/3rdparty/gtest/src/gtest_main.cc)
    link_libraries(gtest)
    set(OPENCV_LIBS opencv_imgproc opencv_core opencv_highgui opencv_imgcodecs opencv_videoio)
    link_libraries(dl)

    # include_directories(src)

    # add_library(common
    #     ../share/common/Logger.cpp
    #     ../share/common/Udp.cpp
    #     )
    link_libraries(common)
    link_libraries(ivslogger)

    link_libraries(framework)

    # usecase tests
    file(GLOB TEST_SOURCES usecase/*.cpp usecase/*.c)
    foreach(test_src ${TEST_SOURCES})
        get_filename_component(test_name ${test_src} NAME_WE)
        add_executable(${test_name} ${test_src})
        target_link_libraries(${test_name} ${OPENCV_LIBS} -lpthread -lavcodec -lavformat -lavutil)
    endforeach(test_src)

endif()
