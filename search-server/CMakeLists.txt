cmake_minimum_required(VERSION 3.8 FATAL_ERROR)
project(search_server)

set(CMAKE_CXX_STANDARD 17)

file(GLOB sources
    *.cpp
    *.h
)

add_executable(
    search_server
    ${sources}
)
