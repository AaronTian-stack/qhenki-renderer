file(GLOB SOURCES "*c" "*.cpp" "*.h" "*hpp")

add_library(spirv_cross ${SOURCES})