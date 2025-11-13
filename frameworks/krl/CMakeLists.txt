cmake_minimum_required(VERSION 3.5)
project(krl)

if(NOT TARGET KCL)
	add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../kcl_framework kcl_framework)
endif()


include(UseCXX11)
include(apply_find_vars)
find_package(ngrtl REQUIRED core)
apply_find_vars(NGRTL)


set(KRL_ROOT ${CMAKE_CURRENT_LIST_DIR})
set(SRC_KRL
	${KRL_ROOT}/krl_material.cpp
	${KRL_ROOT}/krl_scene.cpp
	${KRL_ROOT}/krl_scene_reload.cpp
	${KRL_ROOT}/krl_mesh.cpp
	${KRL_ROOT}/krl_material.h
	${KRL_ROOT}/krl_scene.h
	${KRL_ROOT}/krl_mesh.h
	${KRL_ROOT}/krl_shadowmap.h
	${KRL_ROOT}/krl_cubemap.h
	${KRL_ROOT}/fbo_enums.h
)

include_directories(
	${KRL_ROOT}
	${KCL_PATH_PUBLIC}/src
	${KCL_LIB_PATH_PUBLIC}/tinyxml/src
)

SOURCE_GROUP("" FILES ${SRC_KRL})

set(KRL_INCLUDE_DIRS ${KRL_ROOT}
	${KCL_PATH_PUBLIC}/src
	${KCL_LIB_PATH_PUBLIC}/tinyxml/src PARENT_SCOPE)

if ("${OPT_GRAPHICS}" STREQUAL "GL")
	find_package(OGLX REQUIRED)
	include_directories(${OGLX_INCLUDE_DIRS})
endif()

if(NOT TARGET KRL)
	add_library(KRL STATIC ${SRC_KRL})
endif()

target_compile_definitions(KRL PRIVATE ${OGLX_DEFINITIONS})
