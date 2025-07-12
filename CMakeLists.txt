cmake_minimum_required(VERSION 3.31)
project(Server)

set(CMAKE_CXX_STANDARD 26)

add_executable(Server main.cpp)
target_link_libraries(Server ws2_32)