cmake_minimum_required(VERSION 3.10)
project(save_video)
set(CMAKE_CXX_STANDARD 17)

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

    find_package(OpenSSL REQUIRED)
    if(OPENSSL_FOUND)
        add_definitions(-DCPPHTTPLIB_OPENSSL_SUPPORT)
    endif()

    set(BM_LIBS bmlib bmrt bmcv yuv)
    find_library(BMJPU bmjpuapi)
    if(BMJPU)
        set(JPU_LIBS bmjpuapi bmjpulite)
    endif()

    include_directories(../../../framework)
    include_directories(../../../framework/include)

    include_directories(../../../3rdparty/spdlog/include)
    include_directories(../../../3rdparty/httplib)
    include_directories(../../../3rdparty/nlohmann-json/include)

    include_directories(include)
    add_library(save_video SHARED
        src/save_video.cc
    )

    if(OPENSSL_FOUND)
        target_link_libraries(save_video ${FFMPEG_LIBS} ${OpenCV_LIBS} ${BM_LIBS} ${JPU_LIBS} ${OPENSSL_LIBRARIES} -lpthread)
    else()
        target_link_libraries(save_video ${FFMPEG_LIBS} ${OpenCV_LIBS} ${BM_LIBS} ${JPU_LIBS} -lpthread)
    endif()

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
    # Align with encode module: explicitly list OpenCV libs needed by VideoWriter
    set(OPENCV_LIBS opencv_imgproc opencv_core opencv_highgui opencv_imgcodecs opencv_videoio opencv_video)
    
    include_directories(../../../framework)
    include_directories(../../../framework/include)

    include_directories(../../../3rdparty/spdlog/include)
    include_directories(../../../3rdparty/httplib)
    include_directories(../../../3rdparty/nlohmann-json/include)

    if (DEFINED OPENSSL_PATH)
        include_directories(${OPENSSL_PATH}/include)
        link_directories(${OPENSSL_PATH}/lib)
        add_definitions(-DCPPHTTPLIB_OPENSSL_SUPPORT)
    endif()

    include_directories(include)
    add_library(save_video SHARED
        src/save_video.cc
    )
    if (DEFINED OPENSSL_PATH)
        target_link_libraries(save_video ${FFMPEG_LIBS} ${OPENCV_LIBS} ${BM_LIBS} ${JPU_LIBS} ssl crypto -fprofile-arcs -lgcov -lpthread)
    else()
        target_link_libraries(save_video ${FFMPEG_LIBS} ${OPENCV_LIBS} ${BM_LIBS} ${JPU_LIBS} -fprofile-arcs -lgcov -lpthread)
    endif()
endif()
