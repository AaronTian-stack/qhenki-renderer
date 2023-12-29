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

####################
#set(source "${CMAKE_SOURCE_DIR}/resources")
#set(destination "${CMAKE_CURRENT_BINARY_DIR}/resources")

# Compile shaders
#add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD
#        COMMAND /bin/sh ${CMAKE_SOURCE_DIR}/resources/shaders/compile.sh)

#add_custom_command(TARGET ${PROJECT_NAME}
#        COMMAND ${CMAKE_COMMAND} -E create_symlink ${source} ${destination}
#        DEPENDS ${destination}
#        COMMENT "symbolic link resource folder from ${source} => ${destination}")
####################

#Include library files
include_directories(dependencies/include)

add_executable(${PROJECT_NAME} ${SOURCE_FILES} ${IMGUI_SOURCE_FILES} ${IMGUI_HEADERS} main.cpp)

find_package(Vulkan REQUIRED)
target_include_directories(${PROJECT_NAME} PUBLIC ${Vulkan_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} Vulkan::Vulkan)

find_package(glfw3 REQUIRED)
target_link_libraries(${PROJECT_NAME} glfw)
