cmake_minimum_required(VERSION 3.5)
project(gfxbench_common)

# set(CMAKE_DEBUG_TARGET_PROPERTIES
#   INCLUDE_DIRECTORIES
#   COMPILE_DEFINITIONS
#   POSITION_INDEPENDENT_CODE
#   CONTAINER_SIZE_REQUIRED
#   LIB_VERSION
# )

####################################

#add_definitions(-DDEMO_MODE)

####################################

set(CMAKE_DEBUG_POSTFIX "_d")
set(TFW_ROOT ${CMAKE_CURRENT_LIST_DIR}/../frameworks/testfw)
include(VersionParse)
version_parse(BENCHMARK "${PRODUCT_VERSION}")

add_definitions(-DPRODUCT_VERSION=${PRODUCT_VERSION})

find_package(ngrtl REQUIRED core pngio)
find_package(ZLIB REQUIRED)
find_package(PNG REQUIRED)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../frameworks/kcl_framework kcl_framework_bin)

string(TOUPPER ${CMAKE_CXX_COMPILER_ID} CXX_COMPILER_ID_UPPER)
if (${CXX_COMPILER_ID_UPPER} STREQUAL CLANG)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 ")
elseif (${CXX_COMPILER_ID_UPPER} STREQUAL GNU)
	if (${CMAKE_CXX_COMPILER_VERSION} VERSION_GREATER "4.6")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
	else()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x ")
	endif()
endif()

if(WIN32)
	#warning C4201: nonstandard extension used : nameless struct/union
	#warning C4100: unreferenced formal parameter
	#warning C4512: assignment operator could not be generated
	#warning C4510: default constructor could not be generated
	set(DISABLE_WARNINGS "/wd4996 /wd4100 /wd4201 /wd4512 /wd4510")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /bigobj ${DISABLE_WARNINGS}")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj ${DISABLE_WARNINGS}")
endif()

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../frameworks/ngl/src ngl_bin)

if(EXISTS ${CMAKE_CURRENT_LIST_DIR}/../scene_editor AND WIN32 AND EDITOR MATCHES "[tT][rR][uU][eE]" )
	add_definitions(-DEDITOR=1)
endif()

include(choose_display_protocol)
include_directories(
	${TFW_ROOT}
	${TFW_ROOT}/schemas
	${CMAKE_CURRENT_LIST_DIR}/src
	${CMAKE_CURRENT_LIST_DIR}/components
	${CMAKE_CURRENT_LIST_DIR}/../gfxb_dev/src)

include(${TFW_ROOT}/testfw.cmake)
include(apply_find_vars)
apply_find_vars(ZLIB PNG NGRTL)
#include(CompilerSettings.cmake)

if(BUILD_OVR_APP)
	add_definitions(-DBUILD_OVR_APP)
endif()

if(${WITH_OVR_SDK})
	add_definitions(-DWITH_OVR_SDK)
endif()

if(OVR_BENCHMARK_MODE)
	add_definitions(-DOVR_BENCHMARK)
endif()

if( "$ENV{DISABLE_NETMAN}" STREQUAL "1" )
add_definitions(-DDISABLE_NETMAN)
endif()

include(compiler_flags)

add_subdirectory(src/testbase)

set(TFW_ROOT ${CMAKE_CURRENT_LIST_DIR}/../frameworks/testfw)
include_directories(src/testbase)

if(NOT "${PRODUCT_ID}" STREQUAL "gfxbench_ngl")
	add_subdirectory(src/common)
endif()

include(compiler_flags)

message(STATUS "##################################################################")
list(LENGTH TEST_MODULE NUM_OF_TEST)
if( "${NUM_OF_TEST}" EQUAL "0")
	if( EXISTS "${CMAKE_CURRENT_LIST_DIR}/src/scene41")
		add_subdirectory(src/scene41)
	else()
		list(APPEND TEST_MODULE "scene41;scene5;ngl_test")
		foreach(TEST_DIR ${TEST_MODULE})
			message(STATUS "${TEST_DIR} is trying to add")
			if( EXISTS "${CMAKE_CURRENT_LIST_DIR}/src/${TEST_DIR}")
				add_subdirectory(src/${TEST_DIR})
				message(STATUS "${TEST_DIR} has been added")
			endif()
		endforeach()
	endif()

else()
	foreach(TEST ${TEST_MODULE})
		add_subdirectory(src/${TEST})
	endforeach()
endif()
message(STATUS "##################################################################")

if(EXISTS ${CMAKE_CURRENT_LIST_DIR}/../scene_editor AND WIN32 AND EDITOR MATCHES "[tT][rR][uU][eE]" )
	add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../scene_editor scene_editor_bin)
endif()

include(mklink)
set(TFW_DEVELOPMENT_DIR ${CMAKE_BINARY_DIR}/tfw-dev)

# if not android override RUNTIME_OUTPUT_DIRECTORY so app will find plugins when launching from the IDE
if(NOT ANDROID)
foreach(module ${TESTFW_MODULES})
	get_target_property(module_type ${module} TYPE)
	if ("${module_type}" STREQUAL "SHARED_LIBRARY")
		set_target_properties(${module} PROPERTIES
			RUNTIME_OUTPUT_DIRECTORY_DEBUG ${TFW_DEVELOPMENT_DIR}/plugins
			RUNTIME_OUTPUT_DIRECTORY_RELEASE ${TFW_DEVELOPMENT_DIR}/plugins
			LIBRARY_OUTPUT_DIRECTORY_DEBUG ${TFW_DEVELOPMENT_DIR}/plugins
			LIBRARY_OUTPUT_DIRECTORY_RELEASE ${TFW_DEVELOPMENT_DIR}/plugins
		)
	endif()
endforeach()
endif()

file(MAKE_DIRECTORY "${TFW_DEVELOPMENT_DIR}/plugins")
file(MAKE_DIRECTORY "${TFW_DEVELOPMENT_DIR}/config")
file(MAKE_DIRECTORY "${TFW_DEVELOPMENT_DIR}/data/gfx")
set(NG_TARGET_PLATFORM "$ENV{NG_TARGET_PLATFORM}" CACHE STRING "store platform")

if ("${PRODUCT_ID}" STREQUAL "gfxbench_dx")

	file(MAKE_DIRECTORY "${TFW_DEVELOPMENT_DIR}/D3D12")

	if(NG_TARGET_PLATFORM MATCHES "vs20[0-9][0-9]-arm64" )
		message(STATUS "Copying ARM64 binaries for the Agility SDK")
		file(COPY ${CMAKE_CURRENT_LIST_DIR}/../3rdparty/AgilitySDK/bin/arm64/ DESTINATION ${TFW_DEVELOPMENT_DIR}/D3D12)
	else()
		message(STATUS "Copying x64 binaries for the Agility SDK")
		file(COPY ${CMAKE_CURRENT_LIST_DIR}/../3rdparty/AgilitySDK/bin/x64/ DESTINATION ${TFW_DEVELOPMENT_DIR}/D3D12)
	endif()
endif()

# Prepare runtime environment in build directory (tfw-dev)
if(NOT "$ENV{TFW_DISABLE_LINKS}")
	file(GLOB_RECURSE CONFIGS "${CMAKE_CURRENT_LIST_DIR}/resources/*/*/config/*.json")
	file(MAKE_DIRECTORY ${TFW_DEVELOPMENT_DIR}/config)
	foreach(config ${CONFIGS})
		get_filename_component(FILE_NAME ${config} NAME)
		mklink(${config} ${TFW_DEVELOPMENT_DIR}/config/${FILE_NAME})
	endforeach()

	file(GLOB_RECURSE IMAGES "${CMAKE_CURRENT_LIST_DIR}/resources/*/*/images/*")
	file(MAKE_DIRECTORY ${TFW_DEVELOPMENT_DIR}/images)
	foreach(image ${IMAGES})
		get_filename_component(FILE_NAME ${image} NAME)
		mklink(${image} ${TFW_DEVELOPMENT_DIR}/images/${FILE_NAME})
	endforeach()

	link_all_files(${CMAKE_CURRENT_LIST_DIR}/resources/data/gfx ${TFW_DEVELOPMENT_DIR}/data/gfx)

	if(EXISTS ${CMAKE_CURRENT_LIST_DIR}/../gfxbench-data/data/gfx)
		link_all_files(${CMAKE_CURRENT_LIST_DIR}/../gfxbench-data/data/gfx ${TFW_DEVELOPMENT_DIR}/data/gfx)

	elseif(EXISTS ${CMAKE_CURRENT_LIST_DIR}/../gfxbench-data50/data/gfx)
		link_all_files(${CMAKE_CURRENT_LIST_DIR}/../gfxbench-data50/data/gfx ${TFW_DEVELOPMENT_DIR}/data/gfx)
	endif()

	mklink(${CMAKE_CURRENT_LIST_DIR}/shaders ${TFW_DEVELOPMENT_DIR}/data/gfx/shaders)
endif()

if(TFW_PACKAGE_DIR)
	include(tfw_package.cmake)
endif()
