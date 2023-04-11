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

    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -rdynamic")

    include(src/framework/framework.cmake)

    include(run.cmake)

    #include(src/grpc/grpc.cmake)
    include(src/apps/apps.cmake)
    #include(test/test.cmake)


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
    include_directories(.)
    include_directories(../share)
    include(src/framework/framework.cmake)

    include(run.cmake)

    #include(src/grpc/grpc.cmake)
    include(src/apps/apps.cmake)

endif()