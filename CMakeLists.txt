cmake_minimum_required(VERSION 4.0)
project(Ki VERSION 1.0.0
	   LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Vulkan REQUIRED)
find_package(glm REQUIRED)
find_package(glfw3 REQUIRED)

add_executable(game
  src/application.cc
  src/application.h
  src/gfx/vulkan_context.cc
  src/gfx/vulkan_context.h
  src/gfx/vulkan_support.cc
  src/gfx/vulkan_support.h
  src/gfx/window.cc
  src/gfx/window.h
  src/main.cc
)

target_link_libraries(game glfw Vulkan::Vulkan)
