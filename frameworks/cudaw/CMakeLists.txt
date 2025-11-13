cmake_minimum_required(VERSION 3.5)
project(cudaw)

set(CMAKE_DEBUG_POSTFIX _d)

add_library(cudaw STATIC
    include/cudaw/dyn_func_loader.h
    include/cudaw/cudaw_function_types.h
    include/cudaw/cudaw_function_defines.h
    include/cudaw/cudaw.h
    cudaw.cpp
)

if(ANDROID)
    execute_process(
        COMMAND cp -rL /usr/local/cuda/include ${CMAKE_CURRENT_SOURCE_DIR}/include/nvidia
        RESULT_VARIABLE COPY_RESULT
    )
    if(NOT COPY_RESULT EQUAL 0)
        message(FATAL_ERROR "Failed to copy CUDA headers")
    endif()

    set(CUDA_GL_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/include/nvidia/cudaGL.h)
    if(EXISTS ${CUDA_GL_HEADER})
        file(READ ${CUDA_GL_HEADER} CUDA_GL_CONTENT)
        string(REGEX REPLACE "#include[ \t]*[<\"]GL/gl\\.h[>\"][ \t]*\n" "" CUDA_GL_CONTENT_MODIFIED "${CUDA_GL_CONTENT}")
        file(WRITE ${CUDA_GL_HEADER} "${CUDA_GL_CONTENT_MODIFIED}")
        message(STATUS "Removed GL/gl.h includes from cudaGL.h")
    endif()
else()
    find_package(CUDAToolkit)
endif()

if (CUDAToolkit_FOUND)
    target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC ${CUDAToolkit_INCLUDE_DIRS})
endif ()

target_compile_definitions(cudaw PUBLIC -DCUDAW_STATIC)
target_include_directories(cudaw
    PUBLIC
        include
        include/nvidia
)

if(UNIX AND NOT APPLE AND NOT QNX)
    target_link_libraries(cudaw dl)
endif()

configure_file(${CMAKE_CURRENT_LIST_DIR}/cudawConfig.cmake.in ${CMAKE_BINARY_DIR}/cudawConfig.cmake @ONLY)

install(DIRECTORY include/cudaw DESTINATION include)
if(ANDROID)
    install(DIRECTORY include/nvidia DESTINATION include/cudaw)
endif()

install(TARGETS cudaw
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)
install(FILES ${CMAKE_BINARY_DIR}/cudawConfig.cmake DESTINATION lib/cudaw)
