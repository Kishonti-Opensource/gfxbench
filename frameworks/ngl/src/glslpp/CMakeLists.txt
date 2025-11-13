cmake_minimum_required(VERSION 2.8)
set(LIB_NAME "lib_glsl_pp")
project(${LIB_NAME}) 

set(SOURCE_FILES
	${CMAKE_CURRENT_LIST_DIR}/glslpp.h
	${CMAKE_CURRENT_LIST_DIR}/glslpp.cpp
	${CMAKE_CURRENT_LIST_DIR}/glsl/glcpp/glcpp-lex.c
	${CMAKE_CURRENT_LIST_DIR}/glsl/glcpp/glcpp-parse.c
	${CMAKE_CURRENT_LIST_DIR}/glsl/glcpp/pp.c
	${CMAKE_CURRENT_LIST_DIR}/mesa/program/prog_hash_table.c
	${CMAKE_CURRENT_LIST_DIR}/util/ralloc.c
)


add_library( ${LIB_NAME} STATIC ${SOURCE_FILES} )

set( target_include_directories_
	${CMAKE_CURRENT_LIST_DIR}
	${CMAKE_CURRENT_LIST_DIR}/mesa
	${CMAKE_CURRENT_LIST_DIR}/include
)

if (MSVC_VERSION LESS 1800)
	include_directories(${CMAKE_CURRENT_LIST_DIR}/msvc)
endif()

target_include_directories( ${LIB_NAME} PRIVATE ${target_include_directories_} )

