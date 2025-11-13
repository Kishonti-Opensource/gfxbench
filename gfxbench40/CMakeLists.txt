cmake_minimum_required(VERSION 3.1)
project(GFXBench)

include(VersionParse)
version_parse(BENCHMARK "${PRODUCT_VERSION}")
set(CMAKE_CXX_STANDARD 11)
add_definitions(-DPRODUCT_VERSION=${PRODUCT_VERSION})
set(BENCHMARK_GUI false)  

if (SIGNIFICANT_FRAME_MODE)
	set(SIGNIFICANT_FRAME_MODE_DEFINITION "-DSIGNIFICANT_FRAME_MODE")
else()
	set(SIGNIFICANT_FRAME_MODE_DEFINITION "")
endif()  

if( "$ENV{DISABLE_RESULTS}" STREQUAL "1" )
	add_definitions(-DDISABLE_RESULTS)
endif()

if( "$ENV{DISABLE_NETMAN}" STREQUAL "1" )
add_definitions(-DDISABLE_NETMAN)
endif()

add_definitions(${SIGNIFICANT_FRAME_MODE_DEFINITION})

string(TOUPPER ${CMAKE_CXX_COMPILER_ID} CXX_COMPILER_ID_UPPER)
if (${CXX_COMPILER_ID_UPPER} STREQUAL CLANG)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 ")
    if (UNIX AND NOT APPLE)
        set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -s")
        set(CMAKE_EXE_LINKER_FLAGS_MINSIZEREL "${CMAKE_EXE_LINKER_FLAGS_MINSIZEREL} -s")
        set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} -s")
        set(CMAKE_SHARED_LINKER_FLAGS_MINSIZEREL "${CMAKE_SHARED_LINKER_FLAGS_MINSIZEREL} -s")
    endif()
elseif (${CXX_COMPILER_ID_UPPER} STREQUAL GNU)
    if (${CMAKE_CXX_COMPILER_VERSION} VERSION_GREATER "4.6")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x ")
    endif()
    if (UNIX AND NOT APPLE)
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -s")
        set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} -s")
    endif()
endif()


if (OPT_COMMUNITY_BUILD AND NOT OPT_WINSTORE)
	set(EDITION "community")
else()
	set(EDITION "corporate")
endif()

# TODO: don't set global variables here. Use per target if needed.
if(ANDROID)
	set(CMAKE_DEBUG_POSTFIX "")
else()
	set(CMAKE_DEBUG_POSTFIX "_d")
endif()

if(UNIX AND NOT APPLE)
    set(CMAKE_BUILD_WITH_INSTALL_RPATH true)
endif()

if ("${PRODUCT_ID}" STREQUAL "gfxbench_gl")
	set(OPT_GRAPHICS "GL")
elseif ("${PRODUCT_ID}" STREQUAL "gfxbench_dx")
	set(OPT_GRAPHICS "DX")
elseif ("${PRODUCT_ID}" STREQUAL "gfxbench_metal")
	set(OPT_GRAPHICS "METAL")
else()
	set(OPT_GRAPHICS "GL")
endif()

#TODO remove MACOSX flag
if(APPLE AND NOT IOS)
	set(MACOSX 1)
endif()

if ("${OPT_GRAPHICS}" STREQUAL "GL")
	add_definitions(-DUSE_ANY_GL)
elseif ("${OPT_GRAPHICS}" STREQUAL "DX")
	add_definitions(-DHAVE_DX)
elseif ("${OPT_GRAPHICS}" STREQUAL "METAL")
	if (NOT "${IOS_PLATFORM}" STREQUAL "OS" AND NOT MACOSX)
		message(WARNING "
*****************************************************
* Metal is only supported for iOS and OSX platform  *
*****************************************************
")
		return()
	endif()
	add_definitions(-DUSE_METAL)
endif()

# add kcl_framework here, it's source will
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../frameworks/kcl_framework kcl_framework)
set(KCL_LIBRARIES KCL)
if (NOT TARGET tfw_schemas)
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../frameworks/testfw/schemas ${CMAKE_CURRENT_BINARY_DIR}/testfw_schemas)
endif()

if ("${OPT_GRAPHICS}" STREQUAL "GL")
	find_package(OGLX REQUIRED)

	message(STATUS "OGLX_headers: ${OGLX_INCLUDE_DIRS}")
	message(STATUS "OGLX_libs: ${OGLX_LIBRARIES}")
	include_directories(${OGLX_INCLUDE_DIRS})
endif()

find_package(ngrtl REQUIRED core)
find_package(ZLIB REQUIRED)
find_package(PNG REQUIRED)

apply_find_vars(NGRTL PNG ZLIB)

if(WIN32)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /bigobj")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4250 /wd4800 /wd4244 /wd4251 /wd4131 /wd4996 /wd4244 /wd4018 /bigobj")
endif()
if(NACL)
	include_directories(${OGLX_INCLUDE_DIRS})
endif()

if(IOS)
	add_definitions(-DIPHONE)
elseif(MACOSX)
	find_library(FOUNDATION_LIBRARIES Foundation)
	find_library(IOKIT_LIBRARIES IOKit)
	find_library(APPKIT_LIBRARIES AppKit)
	set(PLATFORM_DEPENDENT_LIBS ${APPKIT_LIBRARIES} ${FOUNDATION_LIBRARIES} ${IOKIT_LIBRARIES})
	
	if ("${OPT_GRAPHICS}" STREQUAL "GL")
		find_package(GLEW REQUIRED CONFIG)
		find_package(OpenGL REQUIRED)
		list(APPEND PLATFORM_DEPENDENT_LIBS ${OPENGL_gl_LIBRARY} ${GLEW_LIBRARIES})
		add_definitions("-DOPENGL_IMPLEMENTATION_GLEW -DOPENGL_2")
	endif()
endif()

include("${CMAKE_CURRENT_LIST_DIR}/../frameworks/testfw/testfw.cmake")

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/src/common)

# TODO: Frameplayer is not supported in this release. Move this code to src/tests/fp_egypt/CMakeLists.txt
#if(PVRTC4)##this one used in frameplayer
#	add_definitions(-DPVRTC4)
#endif()
#
#if(ENABLE_FRAME_CAPTURE)
#	add_definitions(-DENABLE_FRAME_CAPTURE)
#	if(CAPTURE_OUTPUT_PATH)
#		add_definitions(-DEXPATH=${CAPTURE_OUTPUT_PATH})
#	endif()
#endif()
#message(STATUS "OPT_TEST_FRAMEPLAYER: ${OPT_TEST_FRAMEPLAYER}")
#if(OPT_TEST_FP)
#
#	include_directories(
#		${CMAKE_CURRENT_LIST_DIR}/tools/gfx_frameplayer
#		${CMAKE_CURRENT_LIST_DIR}/src/gfxbench
#		${CMAKE_CURRENT_LIST_DIR}/src/tests/fp_egypt
#		${CMAKE_CURRENT_LIST_DIR}/src/common
#		${CMAKE_CURRENT_LIST_DIR}/../frameworks/kcl_framework/kcl/src
#		${CMAKE_CURRENT_LIST_DIR}/../frameworks/kcl_framework/kcl_libraries/libpng/src
#		${CMAKE_CURRENT_LIST_DIR}/../frameworks/kcl_framework/kcl_libraries/libz/src
#		${CMAKE_CURRENT_LIST_DIR}/../frameworks/kcl_framework/kcl_libraries/libetc1/src
#		${CMAKE_CURRENT_LIST_DIR}/../frameworks/kcl_framework/kcl_libraries/tinyxml/src
#		${CMAKE_CURRENT_LIST_DIR}/../frameworks/testfw
#	)
#
#	add_library(frameplayer_core STATIC
#		${CMAKE_CURRENT_LIST_DIR}/src/frame_player/testfw_functions.cpp
#		${CMAKE_CURRENT_LIST_DIR}/tools/gfx_frameplayer/gl_player_helper.cpp
#		${CMAKE_CURRENT_LIST_DIR}/tools/gfx_frameplayer/gl_player_helper.h
#	)
#endif()


include(${CMAKE_CURRENT_LIST_DIR}/src/gfxbench/CMakeLists.txt)


message(STATUS "Adding test directory: ${CMAKE_CURRENT_LIST_DIR}/src/tests/${BENCHMARK_VERSION_MAJOR}${BENCHMARK_VERSION_MINOR}")
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/src/tests/${BENCHMARK_VERSION_MAJOR}${BENCHMARK_VERSION_MINOR})

if (OPT_BUILD_MAINAPP_CHROME)
	set(TFW_ROOT ${CMAKE_CURRENT_LIST_DIR}/../frameworks/testfw)
	include_directories("${CMAKE_CURRENT_LIST_DIR}/testfw")
	add_subdirectory(../gfxbench_chrome_ui mainapp)
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

# Prepare runtime environment in build directory (tfw-dev)
if(NOT "$ENV{TFW_DISABLE_LINKS}")
    file(GLOB_RECURSE CONFIGS "${CMAKE_CURRENT_LIST_DIR}/resources/*/config/*.json")
    file(MAKE_DIRECTORY ${TFW_DEVELOPMENT_DIR}/config)
    foreach(config ${CONFIGS})
        get_filename_component(FILE_NAME ${config} NAME)
        mklink(${config} ${TFW_DEVELOPMENT_DIR}/config/${FILE_NAME})
    endforeach()
    file(GLOB_RECURSE IMAGES "${CMAKE_CURRENT_LIST_DIR}/resources/*/images/*")
    file(MAKE_DIRECTORY ${TFW_DEVELOPMENT_DIR}/images)
    foreach(image ${IMAGES})
        get_filename_component(FILE_NAME ${image} NAME)
        mklink(${image} ${TFW_DEVELOPMENT_DIR}/images/${FILE_NAME})
    endforeach()
    link_all_files(${CMAKE_CURRENT_LIST_DIR}/resources/data/gfx ${TFW_DEVELOPMENT_DIR}/data/gfx)

    if(EXISTS ${CMAKE_CURRENT_LIST_DIR}/../gfxbench-data/data/gfx)
        link_all_files(${CMAKE_CURRENT_LIST_DIR}/../gfxbench-data/data/gfx ${TFW_DEVELOPMENT_DIR}/data/gfx)

    elseif(EXISTS ${CMAKE_CURRENT_LIST_DIR}/../gfxbench-data40/data/gfx)
        link_all_files(${CMAKE_CURRENT_LIST_DIR}/../gfxbench-data40/data/gfx ${TFW_DEVELOPMENT_DIR}/data/gfx)
    endif()
endif()

if(TFW_PACKAGE_DIR)
    include(tfw_package.cmake)
endif()
