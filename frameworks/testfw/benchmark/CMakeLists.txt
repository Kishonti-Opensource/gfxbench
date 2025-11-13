cmake_minimum_required(VERSION 2.8.11)

project(tfw_benchmark_utils)

find_package(ngrtl REQUIRED core)
find_package(OGLX REQUIRED)

if( "$ENV{NO_RESULT}" STREQUAL "1" )
	add_definitions(-DNO_RESULT)
endif()

add_library(tfw_benchmark_utils STATIC
    rendertest.h
    rendertest.cpp
    offscreenmanager.h
    offscreenmanager.cpp)
target_compile_definitions(tfw_benchmark_utils PRIVATE ${OGLX_DEFINITIONS})
target_include_directories(tfw_benchmark_utils PUBLIC ${CMAKE_CURRENT_LIST_DIR}/.. PRIVATE ${NGRTL_INCLUDE_DIRS} ${OGLX_INCLUDE_DIRS})
target_link_libraries(tfw_benchmark_utils ${NGRTL_LIBRARIES})
