cmake_minimum_required(VERSION 3.16)
project(fluxdb VERSION 1.0.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)
message(STATUS "FluxDB build configured: ${CMAKE_BUILD_TYPE}")

include(FetchContent)
FetchContent_Declare(httplib GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git GIT_TAG v0.16.0)
FetchContent_MakeAvailable(httplib)

add_executable(fluxdb src/main.cpp src/store.cpp src/server.cpp src/api_server.cpp)
target_include_directories(fluxdb PRIVATE src)
target_link_libraries(fluxdb PRIVATE httplib::httplib Threads::Threads)
target_compile_options(fluxdb PRIVATE -Wall -Wextra -O2)
