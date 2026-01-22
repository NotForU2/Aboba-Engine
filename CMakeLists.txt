cmake_minimum_required(VERSION 3.10.0)
project(Aboba-Engine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SDL2 REQUIRED)
find_package(EnTT REQUIRED)

add_executable(Aboba-Engine src/main.cpp src/core/Engine.cpp src/core/Window.cpp)

target_link_libraries(Aboba-Engine PRIVATE SDL2::SDL2main SDL2::SDL2 EnTT::EnTT)