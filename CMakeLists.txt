cmake_minimum_required(VERSION 3.10.0)
project(Aboba-Engine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(SDL2 REQUIRED)
find_package(EnTT REQUIRED)

add_executable(Aboba-Engine main.cpp)

target_link_libraries(Aboba-Engine PRIVATE SDL2::SDL2 EnTT::EnTT)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(Aboba-Engine PRIVATE -Wall -Wextra -Wpedantic)
endif()