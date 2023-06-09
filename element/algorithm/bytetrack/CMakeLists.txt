cmake_minimum_required(VERSION 3.10)
project(algorithmAPI)
set(CMAKE_CXX_STANDARD 14)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}  -fprofile-arcs -g")

if (NOT DEFINED TARGET_ARCH)
    set(TARGET_ARCH pcie)
endif()

if (${TARGET_ARCH} STREQUAL "pcie")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -pthread")
    set(CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS} -Xcompiler -fPIC")

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

    # use Eigen
    find_package(Eigen3 REQUIRED)
    include_directories(${EIGEN3_INCLUDE_DIRS})
    link_directories(${EIGEN3_LIB_DIRS})

    include_directories(../../../framework/include)
    link_directories(../../../framework/build)
    link_libraries(framework)
    link_libraries(ivslogger)

    include_directories(../../../share)
    include_directories(../../../share/3rdparty/spdlog/include)
    include_directories(../../../share/3rdparty/nlohmann-json/include)

    include_directories(include)
    add_library(bytetrack SHARED
        src/bytetrack.cc
        src/bytetrack_kalmanfilter.cc
        src/bytetrack_lapjv.cc
        src/bytetrack_strack.cc
        src/bytetrack_bytetracker.cc
        )
    target_link_libraries(bytetrack ${FFMPEG_LIBS} ${OpenCV_LIBS} ${the_libbmlib.so} ${the_libbmrt.so} ${the_libbmcv.so} -lpthread)

elseif (${TARGET_ARCH} STREQUAL "soc")
    add_compile_options(-fPIC)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}  -fprofile-arcs -ftest-coverage -g -rdynamic")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}  -fprofile-arcs -ftest-coverage -g -rdynamic")
    set(CMAKE_C_COMPILER aarch64-linux-gnu-gcc)
    set(CMAKE_ASM_COMPILER aarch64-linux-gnu-gcc)
    set(CMAKE_CXX_COMPILER aarch64-linux-gnu-g++)
    set(BM_LIBS bmlib bmrt bmcv yuv)
    set(JPU_LIBS bmjpuapi bmjpulite)
    include_directories("${SDK}/include/")
    include_directories("${SDK}/include/opencv4")
    link_directories("${SDK}/lib/")
    include_directories(../../../framework/include)
    link_directories(../../../framework/build_soc)
    link_libraries(framework)
    link_libraries(ivslogger)
    include_directories(../../../share)
    include_directories(../../../share/3rdparty/spdlog/include)
    include_directories(../../../share/3rdparty/nlohmann-json/include)

    # use Eigen
    find_package(Eigen3 REQUIRED)
    include_directories(${EIGEN3_INCLUDE_DIRS})
    link_directories(${EIGEN3_LIB_DIRS})

    include_directories(include)
    add_library(bytetrack SHARED
        src/bytetrack.cc
        src/bytetrack_kalmanfilter.cc
        src/bytetrack_lapjv.cc
        src/bytetrack_strack.cc
        src/bytetrack_bytetracker.cc
        )
    target_link_libraries(bytetrack ${FFMPEG_LIBS} ${OpenCV_LIBS} ${BM_LIBS} ${JPU_LIBS} ${the_libbmlib.so} ${the_libbmrt.so} ${the_libbmcv.so} -fprofile-arcs -lgcov -lpthread)
endif()