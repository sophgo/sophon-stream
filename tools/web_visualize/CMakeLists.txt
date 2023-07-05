cmake_minimum_required(VERSION 3.10)
project(sophon-stream)

set(CMAKE_CXX_STANDARD 17)


include_directories(../../3rdparty/websocketpp)
include_directories(../../3rdparty/spdlog/include)
include_directories(../../3rdparty/nlohmann-json/include)
include_directories(../../3rdparty/httplib)

include_directories(include)
add_executable(server src/http_struct_utils.cc src/http_interact_mgr.cc src/server.cc)
target_link_libraries(server -lpthread)
