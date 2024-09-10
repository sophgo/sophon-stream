cmake_minimum_required(VERSION 3.10)
project(sophon_stream)

set(CMAKE_CXX_STANDARD 17)

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

    link_libraries(pthread)
    link_libraries(dl)

    link_directories(../build/lib)

    include_directories(../3rdparty/spdlog/include)
    include_directories(../3rdparty/nlohmann-json/include)
    include_directories(../3rdparty/httplib)
    include_directories(../element/multimedia/decode/include)

    include_directories(../framework)
    include_directories(../framework/include)
    include_directories(../element/multimedia/osd/include)

    include_directories(./include)
    set(demo_src
        src/main.cc
    )
    get_filename_component(demo_name ${demo_src} NAME_WE)
    add_executable(${demo_name} ${demo_src})
    target_link_libraries(${demo_name}  cvunitext)

    add_dependencies(${demo_name} ivslogger framework cvunitext)

    find_package(OpenSSL REQUIRED)
    if(OPENSSL_FOUND)
        add_definitions(-DCPPHTTPLIB_OPENSSL_SUPPORT)
    endif()

    if(OPENSSL_FOUND)
        target_link_libraries(${demo_name} -ldl ${OPENCV_LIBS} ${OPENSSL_LIBRARIES} -lpthread -lavcodec -lavformat -lavutil -livslogger -lframework)
    else()
        target_link_libraries(${demo_name} -ldl ${OPENCV_LIBS} -lpthread -lavcodec -lavformat -lavutil -livslogger -lframework)
    endif()

elseif(${TARGET_ARCH} STREQUAL "soc")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}  -fprofile-arcs -ftest-coverage -g -rdynamic")
    # set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}  -fprofile-arcs -ftest-coverage -rdynamic")
    add_compile_options(-fPIC)
    set(CMAKE_C_COMPILER aarch64-linux-gnu-gcc)
    set(CMAKE_ASM_COMPILER aarch64-linux-gnu-gcc)
    set(CMAKE_CXX_COMPILER aarch64-linux-gnu-g++)
    set(BM_LIBS bmlib bmrt bmcv yuv)
    find_library(BMJPU bmjpuapi)
    if(BMJPU)
        set(JPU_LIBS bmjpuapi bmjpulite)
    endif()
    include_directories("${SOPHON_SDK_SOC}/include/")
    include_directories("${SOPHON_SDK_SOC}/include/opencv4")
    link_directories("${SOPHON_SDK_SOC}/lib/")
    include_directories(.)

    set(OPENCV_LIBS opencv_imgproc opencv_core opencv_highgui opencv_imgcodecs opencv_videoio)
    link_libraries(pthread)
    link_libraries(dl)

    link_directories(../build/lib/)

    include_directories(../3rdparty/spdlog/include)
    include_directories(../3rdparty/nlohmann-json/include)
    include_directories(../3rdparty/httplib)
    include_directories(../element/multimedia/decode/include)
    include_directories(../framework)
    include_directories(../framework/include)
    include_directories(../element/multimedia/osd/include)

    if (DEFINED OPENSSL_PATH)
        include_directories(${OPENSSL_PATH}/include)
        link_directories(${OPENSSL_PATH}/lib)
        add_definitions(-DCPPHTTPLIB_OPENSSL_SUPPORT)
    endif()

    # ======== awtk =====
    set(AWTK_ROOT ../3rdparty/awtk)
    set(COMMON_FLAGS
    -DHAS_STD_MALLOC
    -DHAS_STDIO
    -DHAS_FAST_MEMCPY
    -DWITH_VGCANVAS
    -DWITH_UNICODE_BREAK
    -DLOAD_ASSET_WITH_MMAP=1
    -DWITH_SOCKET=1
    -DWITH_ASSET_LOADER
    -DWITH_FS_RES
    -DHAS_GET_TIME_US64=1
    -DSTBTT_STATIC
    -DSTB_IMAGE_STATIC
    -DWITH_STB_IMAGE
    -DWITH_STB_FONT
    -DWITH_TEXT_BIDI=1
    -DAPP_TYPE=APP_MOBILE
    -DLINUX_FB
    -DWITH_NANOVG_AGGE
    -DWITH_LINUX_DRM
    -DWITH_FAST_LCD_PORTRAIT
    -DWITH_IME_PINYIN
    -DLINUX
    -DHAS_PTHREAD
    -DWITH_DATA_READER_WRITER=1
    -DWITH_EVENT_RECORDER_PLAYER=1
    -DWITH_WIDGET_TYPE_CHECK=1
    -DENABLE_CURSOR
)

set(OS_FLAGS "-Wall -fno-strict-aliasing -Os")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OS_FLAGS}")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OS_FLAGS}")

add_definitions(${COMMON_FLAGS})

include_directories(${AWTK_ROOT}/include)
include_directories(${AWTK_ROOT}/include/ext_widgets)


link_directories(${AWTK_ROOT}/lib)



    # ======== awtk end ====

    include_directories(./include)
    set(demo_src
        src/main.cc
    )
    get_filename_component(demo_name ${demo_src} NAME_WE)
    add_executable(${demo_name} ${demo_src}  src/assets.c src/EdgeAwtk.cpp)
    target_link_libraries(${demo_name}  cvunitext awtk)

    add_dependencies(${demo_name} ivslogger framework cvunitext)
    add_dependencies(${demo_name} ivslogger framework)
    if (DEFINED OPENSSL_PATH)
        target_link_libraries(${demo_name} -ldl ${OPENCV_LIBS} ${BM_LIBS} ${JPU_LIBS} ssl crypto -fprofile-arcs -lgcov -lpthread -lavcodec -lavformat -lavutil -livslogger -lframework)
    else()
        target_link_libraries(${demo_name} -ldl ${OPENCV_LIBS} ${BM_LIBS} ${JPU_LIBS} -fprofile-arcs -lgcov -lpthread -lavcodec -lavformat -lavutil -livslogger -lframework)
    endif()

endif()
