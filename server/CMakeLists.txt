cmake_minimum_required(VERSION 3.16)
project(fluxdb VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(fluxdb
    src/main.cpp
    src/store.cpp
    src/server.cpp
)

target_include_directories(fluxdb PRIVATE src)
target_link_libraries(fluxdb PRIVATE pthread)
target_compile_options(fluxdb PRIVATE -Wall -Wextra -O2)
