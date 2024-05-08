cmake_minimum_required(VERSION 3.5)
project(vulkan_pathtracer)

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

file(GLOB_RECURSE SPIRV CONFIGURE_DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/include/spirv_cross/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/include/spirv_cross/*.h
        ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/include/spirv_cross/*.hpp
)

set(dir ${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${dir}/build)

#Include library files
include_directories(dependencies/include)

add_executable(${PROJECT_NAME} ${SOURCE_FILES} ${IMGUI} ${SPIRV} main.cpp)

find_package(Vulkan REQUIRED)
target_include_directories(${PROJECT_NAME} PUBLIC ${Vulkan_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} Vulkan::Vulkan)

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
    find_package(glfw3 REQUIRED)
    target_link_libraries(${PROJECT_NAME} glfw)
elseif (WIN32)
    # do windows stuff
    target_link_libraries(${PROJECT_NAME} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/glfw3.dll)
endif ()


####################
# Set the directory containing your shader files
set(SHADER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/resources/shaders)

# Find all .vert and .frag files in the shader directory
file(GLOB_RECURSE SHADER_SOURCES "${SHADER_DIR}/*.vert" "${SHADER_DIR}/*.frag")

# Compile shaders to SPIR-V using glslc
foreach(SHADER_SOURCE ${SHADER_SOURCES})
    # Get the filename and extension
    get_filename_component(FILENAME ${SHADER_SOURCE} NAME_WE)
    get_filename_component(EXTENSION ${SHADER_SOURCE} EXT)

    # Compile the shader to SPIR-V
    set(SPIRV_OUTPUT "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${FILENAME}.spv")
    add_custom_command(
            OUTPUT ${SPIRV_OUTPUT}
            COMMAND glslc ${SHADER_SOURCE} -o ${SPIRV_OUTPUT}
            MAIN_DEPENDENCY ${SHADER_SOURCE}
            COMMENT "Compiling ${SHADER_SOURCE}"
    )

    # Add the SPIR-V file as a source for the executable
    target_sources(${PROJECT_NAME} PRIVATE ${SPIRV_OUTPUT})
endforeach()
####################
