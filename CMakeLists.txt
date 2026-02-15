cmake_minimum_required(VERSION 3.21.0)
project(Aboba-Engine)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SDL2 REQUIRED)
find_package(EnTT REQUIRED)
find_package(glm REQUIRED)
find_package(Vulkan REQUIRED)
find_package(VulkanMemoryAllocator REQUIRED)

add_executable(Aboba-Engine 
  src/main.cpp
  src/core/Engine.cpp
  src/core/Window.cpp
  src/system/InputSystem.cpp
  src/system/RenderSystem.cpp
  src/system/CollisionSystem.cpp
  src/graphics/VulkanRenderer.cpp
  src/graphics/VulkanBuffer.cpp
  src/graphics/VulkanContext.cpp
  src/graphics/VulkanTexture.cpp
)

target_link_libraries(Aboba-Engine PRIVATE
  SDL2::SDL2main
  SDL2::SDL2
  EnTT::EnTT
  glm::glm
  Vulkan::Vulkan
  GPUOpen::VulkanMemoryAllocator
)

target_include_directories(Aboba-Engine PRIVATE ${CMAKE_SOURCE_DIR}/src/vendor)

add_custom_command(TARGET Aboba-Engine POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
          ${CMAKE_SOURCE_DIR}/textures
          $<TARGET_FILE_DIR:Aboba-Engine>/textures
)

add_custom_command(TARGET Aboba-Engine POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
          ${CMAKE_SOURCE_DIR}/shaders
          $<TARGET_FILE_DIR:Aboba-Engine>/shaders
)

add_custom_command(TARGET Aboba-Engine POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
          ${CMAKE_SOURCE_DIR}/models
          $<TARGET_FILE_DIR:Aboba-Engine>/models
)