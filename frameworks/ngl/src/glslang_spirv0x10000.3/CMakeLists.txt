cmake_minimum_required(VERSION 3.5)

add_subdirectory(glslang)
add_subdirectory(SPIRV)
add_subdirectory(hlsl)
set(GLSLANG_INCLUDE_DIRS ${CMAKE_CURRENT_LIST_DIR} PARENT_SCOPE)
set(GLSLANG_LIBRARIES glslang SPIRV HLSL PARENT_SCOPE)
