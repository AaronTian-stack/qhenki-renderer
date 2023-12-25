cmake_minimum_required(VERSION 3.3)
project(vulkan_pathtracer)

set(CMAKE_CXX_STANDARD 17)

# set files to compile
file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c)

file(GLOB IMGUI_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/include/imgui/*.cpp)
file(GLOB IMGUI_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/include/imgui/*.h)

set(dir ${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${dir}/build)

#Include library files
include_directories(dependencies/include)

add_executable(vulkan_pathtracer ${SOURCE_FILES} ${IMGUI_SOURCE_FILES} ${IMGUI_HEADERS} main.cpp)

find_package(Vulkan REQUIRED)
target_include_directories(vulkan_pathtracer PUBLIC ${Vulkan_INCLUDE_DIRS})
target_link_libraries(vulkan_pathtracer Vulkan::Vulkan)

find_package(glfw3 REQUIRED)
target_link_libraries(vulkan_pathtracer glfw)
