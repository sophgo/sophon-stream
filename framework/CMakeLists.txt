cmake_minimum_required(VERSION 3.10)
project(sophon_stream)

set(CMAKE_CXX_STANDARD 14)

if (NOT DEFINED TARGET_ARCH)
    set(TARGET_ARCH pcie)
endif()

if (${TARGET_ARCH} STREQUAL "pcie")

    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}  -fprofile-arcs")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC -rdynamic")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -rdynamic")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -rdynamic")

    include_directories(../3rdparty/spdlog/include)
    include_directories(../3rdparty/nlohmann-json/include)

    add_library(ivslogger
      common/logger.cpp
    )

    include_directories(../3rdparty/gtest)
    include_directories(../3rdparty/gtest/include)
    add_library(gtest ../3rdparty/gtest/src/gtest-all.cc ../../3rdparty/gtest/src/gtest_main.cc)

    include_directories(./)
    include_directories(include)
    add_library(framework SHARED
        src/element.cc
        src/datapipe.cc
        src/element_manager.cc
        src/element_factory.cc
        src/engine.cc
        src/connector.cc
    )
    link_libraries(dl)
    target_link_libraries(framework -ldl)
     

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

    include_directories(../3rdparty/spdlog/include)

    include_directories(../3rdparty/nlohmann-json/include)

    # gtest includes and libraries for all tests
    include_directories(../3rdparty/gtest/ ../3rdparty/gtest/include src)

    add_library(gtest ../3rdparty/gtest/src/gtest-all.cc ../3rdparty/gtest/src/gtest_main.cc)
    link_libraries(gtest)
    set(OPENCV_LIBS opencv_imgproc opencv_core opencv_highgui opencv_imgcodecs opencv_videoio)
    link_libraries(dl)

    include_directories(../3rdparty/spdlog/include)
    include_directories(../3rdparty/nlohmann-json/include)

    add_library(ivslogger
      common/logger.cpp
    )

    include_directories(.)
    include_directories(include)
    add_library(framework SHARED
        src/element.cc
        src/datapipe.cc
        src/element_manager.cc
        src/element_factory.cc
        src/engine.cc
        src/connector.cc
    )
    link_libraries(dl)
    target_link_libraries(framework -ldl -fprofile-arcs -lgcov)

endif()