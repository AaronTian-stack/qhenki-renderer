cmake_minimum_required(VERSION 3.5)
project(qhenki-renderer)

set(CMAKE_CXX_STANDARD 17)

# set files to compile
file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp
)

file(GLOB_RECURSE IMGUI CONFIGURE_DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/include/imgui/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/include/imgui/*.h
        ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/include/ImGuiFileDialog-0.6.7/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/include/ImGuiFileDialog-0.6.7/*.h
)

add_subdirectory(dependencies/include/mikktspace)
add_subdirectory(dependencies/include/spirv_cross)

set(dir ${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${dir}/build)

# multi core compilation
if (WIN32)
    add_compile_options(/MP)
endif ()

#Include library files
include_directories(dependencies/include)

add_executable(${PROJECT_NAME} ${SOURCE_FILES} ${IMGUI} main.cpp)

find_package(Vulkan REQUIRED)

target_include_directories(${PROJECT_NAME} PUBLIC ${Vulkan_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} Vulkan::Vulkan
        mikktspace
        spirv_cross
)

# bootstrap-vk
include(FetchContent)
FetchContent_Declare(
        fetch_vk_bootstrap
        GIT_REPOSITORY https://github.com/charles-lunarg/vk-bootstrap
        GIT_TAG        v1.3.278
)
FetchContent_MakeAvailable(fetch_vk_bootstrap)
target_link_libraries(${PROJECT_NAME} vk-bootstrap::vk-bootstrap)

if (APPLE)
    target_link_libraries(${PROJECT_NAME} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/libglfw.3.dylib)
elseif (WIN32)
    target_link_libraries(${PROJECT_NAME} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/glfw3.dll)
endif ()

add_subdirectory(resources/shaders)
