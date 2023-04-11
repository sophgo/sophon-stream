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

if (NOT DEFINED TARGET_ARCH)
    set(TARGET_ARCH pcie)
endif()

if (${TARGET_ARCH} STREQUAL "pcie")
    #set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}  -fprofile-arcs -ftest-coverage")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}  -fprofile-arcs")
    #SET(CMAKE_BUILD_TYPE "Debug")

    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -pthread")
    set(CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS} -Xcompiler -fPIC")
    #include_directories(.)
    include_directories(../share)

    find_package(PkgConfig REQUIRED)

    # nlohmann/json https://github.com/nlohmann/json
    include_directories(../share/3rdparty/nlohmann-json/include)

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

    include_directories(.)
    include_directories(../share)
    include_directories(../share/3rdparty/spdlog/include)
    # gtest includes and libraries for all tests
    include_directories(../share/3rdparty/gtest/ ../share/3rdparty/gtest/include src)

    add_library(gtest ../share/3rdparty/gtest/src/gtest-all.cc ../share/3rdparty/gtest/src/gtest_main.cc)
    link_libraries(gtest)

    add_library(ivslogger ../share/common/Udp.cpp
        ../share/common/Logger.cpp
        ../share/common/ff_decode.cpp)
    link_libraries(ivslogger)

    add_library(multiMediaApi SHARED
        src/MultiMediaFactorySelector.cpp
        src/MultiMedia.cpp
        src/context/SophgoContext.cpp
        src/process/SophgoDecode.cpp
        src/factory/SophgoDecodeFactory.cpp
        )
    target_link_libraries(multiMediaApi ${FFMPEG_LIBS} 
        ${OpenCV_LIBS} ${the_libbmlib.so} ${the_libbmrt.so} ${the_libbmcv.so} -lpthread)

    enable_testing()

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

elseif(${TARGET_ARCH} STREQUAL "soc")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}  -fprofile-arcs -g")
    add_compile_options(-fPIC)
    set(CMAKE_C_COMPILER aarch64-linux-gnu-gcc)
    set(CMAKE_ASM_COMPILER aarch64-linux-gnu-gcc)
    set(CMAKE_CXX_COMPILER aarch64-linux-gnu-g++)
    set(BM_LIBS bmlib bmrt bmcv yuv)
    set(JPU_LIBS bmjpuapi bmjpulite)
    include_directories("${SDK}/include/")
    include_directories("${SDK}/include/opencv4")
    link_directories("${SDK}/lib/")
    include_directories(../share/3rdparty/nlohmann-json/include)
    include_directories(../share)
    include_directories(../share/3rdparty/spdlog/include)
    include_directories(../share/3rdparty/gtest/ ../share/3rdparty/gtest/include src)
    add_library(gtest ../share/3rdparty/gtest/src/gtest-all.cc ../share/3rdparty/gtest/src/gtest_main.cc)
    link_libraries(gtest)
    add_library(ivslogger ../share/common/Udp.cpp
        ../share/common/Logger.cpp
        ../share/common/ff_decode.cpp)
    link_libraries(ivslogger)
    add_library(multiMediaApi SHARED
        src/MultiMediaFactorySelector.cpp
        src/MultiMedia.cpp
        src/context/SophgoContext.cpp
        src/process/SophgoDecode.cpp
        src/factory/SophgoDecodeFactory.cpp
        )
    target_link_libraries(multiMediaApi ${FFMPEG_LIBS} 
        ${OpenCV_LIBS} ${BM_LIBS} ${JPU_LIBS} ${the_libbmlib.so} ${the_libbmrt.so} ${the_libbmcv.so} -lpthread)

    enable_testing()

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

endif()