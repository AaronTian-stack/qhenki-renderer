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

file(GLOB_RECURSE MIKKTSPACE CONFIGURE_DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/include/mikktspace/*.c
        ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/include/mikktspace/*.h
)

file(GLOB_RECURSE SPIRV_CROSS CONFIGURE_DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/include/spirv_cross/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/include/spirv_cross/*.h
        ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/include/spirv_cross/*.hpp
)

file(GLOB_RECURSE VK_BOOTSTRAP CONFIGURE_DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/include/vk-bootstrap/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/include/vk-bootstrap/*.h
)

set(dir ${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${dir}/build)

# Include library files
include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/include
        ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/include/imgui
        ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/include/spirv_cross
        ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/include/mikktspace
)

add_executable(${PROJECT_NAME} ${SOURCE_FILES} ${IMGUI} ${MIKKTSPACE} ${SPIRV_CROSS} ${VK_BOOTSTRAP} main.cpp)

add_subdirectory(dependencies/include/glm)
find_package(Vulkan REQUIRED)

target_include_directories(${PROJECT_NAME} PUBLIC ${Vulkan_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} Vulkan::Vulkan)

if (APPLE)
    target_link_libraries(${PROJECT_NAME} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/libglfw.3.dylib)
elseif (WIN32)
    target_link_libraries(${PROJECT_NAME} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/libglfw3dll.a)
endif ()

# Include the code to compile shaders
include(resources/shaders/compile_shaders.cmake)
compile_shaders(${CMAKE_CURRENT_SOURCE_DIR}/resources/shaders ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
