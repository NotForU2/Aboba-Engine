cmake_minimum_required(VERSION 3.10.0)
project(Aboba-Engine)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SDL2 REQUIRED)
find_package(EnTT REQUIRED)
find_package(glm REQUIRED)
find_package(Vulkan REQUIRED)

add_executable(Aboba-Engine 
  src/main.cpp
  src/core/Engine.cpp
  src/core/Window.cpp
  src/system/InputSystem.cpp
  src/system/RenderSystem.cpp
  src/system/CollisionSystem.cpp
)

target_link_libraries(Aboba-Engine PRIVATE
  SDL2::SDL2main
  SDL2::SDL2
  EnTT::EnTT
  glm::glm
  Vulkan::Vulkan
)