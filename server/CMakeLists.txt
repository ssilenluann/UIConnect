cmake_minimum_required(VERSION 3.01)
project(uiconnect)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread -g -O0 -Wall")

set(SRC_FILE main.cpp)

add_subdirectory(network)
add_executable(server ${SRC_FILE})

target_link_libraries(server PUBLIC network)
