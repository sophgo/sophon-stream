cmake_minimum_required(VERSION 3.10)
project(sophon_stream)

set(CMAKE_CXX_STANDARD 17)

if (NOT DEFINED TARGET_ARCH)
    set(TARGET_ARCH pcie)
endif()

if (${TARGET_ARCH} STREQUAL "pcie")

    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}  -fprofile-arcs")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC -rdynamic")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -rdynamic -fpermissive")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -rdynamic")

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

    include_directories(../3rdparty/spdlog/include)
    include_directories(../3rdparty/nlohmann-json/include)
    include_directories(../3rdparty/httplib)

    add_library(ivslogger SHARED
      common/logger.cc
      common/profiler.cc
      common/common_defs.h
      common/http_defs.cc
      common/common_tool.cc
    )
    target_link_libraries(ivslogger -ldl ${OPENCV_LIBS} ${BM_LIBS} ${JPU_LIBS})

    include_directories(./)
    include_directories(include)
    add_library(framework SHARED
        src/element.cc
        src/datapipe.cc
        src/graph.cc
        src/element_factory.cc
        src/engine.cc
        src/connector.cc
        src/listen_thread.cc
    )
    link_libraries(dl)
    target_link_libraries(framework -ldl ${OPENCV_LIBS} ${BM_LIBS} ${JPU_LIBS})
     

elseif(${TARGET_ARCH} STREQUAL "soc")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}  -fprofile-arcs -ftest-coverage -g -rdynamic")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}  -fprofile-arcs -ftest-coverage -rdynamic -fpermissive")
    add_compile_options(-fPIC)
    set(CMAKE_C_COMPILER aarch64-linux-gnu-gcc)
    set(CMAKE_ASM_COMPILER aarch64-linux-gnu-gcc)
    set(CMAKE_CXX_COMPILER aarch64-linux-gnu-g++)
    include_directories("${SOPHON_SDK_SOC}/include/")
    include_directories("${SOPHON_SDK_SOC}/include/opencv4")
    link_directories("${SOPHON_SDK_SOC}/lib/")
    include_directories(.)

    set(BM_LIBS bmlib bmrt bmcv yuv)
    find_library(BMJPU bmjpuapi)
    if(BMJPU)
        set(JPU_LIBS bmjpuapi bmjpulite)
    endif()

    link_libraries(pthread)

    include_directories(../3rdparty/spdlog/include)

    include_directories(../3rdparty/nlohmann-json/include)
    include_directories(../3rdparty/httplib)

    set(OPENCV_LIBS opencv_imgproc opencv_core opencv_highgui opencv_imgcodecs opencv_videoio)
    link_libraries(dl)

    add_library(ivslogger SHARED
      common/logger.cc
      common/profiler.cc
      common/common_defs.h
      common/http_defs.cc
      common/common_tool.cc
    )
    target_link_libraries(ivslogger -ldl ${OPENCV_LIBS} ${BM_LIBS} ${JPU_LIBS} -fprofile-arcs -lgcov)

    include_directories(.)
    include_directories(include)
    add_library(framework SHARED
        src/element.cc
        src/datapipe.cc
        src/graph.cc
        src/element_factory.cc
        src/engine.cc
        src/connector.cc
        src/listen_thread.cc
    )
    link_libraries(dl)
    target_link_libraries(framework -ldl ${OPENCV_LIBS} ${BM_LIBS} ${JPU_LIBS} -fprofile-arcs -lgcov)

endif()