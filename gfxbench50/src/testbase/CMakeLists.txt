cmake_minimum_required(VERSION 3.5)
project(testbase)

####################################
#add_definitions(-DDEMO_MODE)
####################################

set(CMAKE_DEBUG_POSTFIX "_d")
set(TFW_ROOT ${CMAKE_CURRENT_LIST_DIR}/../../../frameworks/testfw)
include(VersionParse)
version_parse(BENCHMARK "${PRODUCT_VERSION}")

find_package(ngrtl REQUIRED core pngio)
find_package(ZLIB REQUIRED)
find_package(PNG REQUIRED)

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

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4250 /wd4800 /wd4244 /wd4251 /wd4131 /wd4244 /wd4018")
if(WIN32)
	#warning C4201: nonstandard extension used : nameless struct/union
	#warning C4100: unreferenced formal parameter
	#warning C4512: assignment operator could not be generated
	#warning C4510: default constructor could not be generated
	set(DISABLE_WARNINGS "/wd4996 /wd4100 /wd4201 /wd4512 /wd4510")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /bigobj ${DISABLE_WARNINGS}")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj ${DISABLE_WARNINGS}")
endif()

if(NOT TARGET ngl_api)
	add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../../../ngl_api/src ngl_api_bin)
endif()

if(NOT TARGET ksl_compiler)
	add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../../../frameworks/ksl_compiler ksl_compiler)
endif()

if(EXISTS ${CMAKE_CURRENT_LIST_DIR}/../scene_editor AND WIN32 AND EDITOR MATCHES "[tT][rR][uU][eE]" )
	add_definitions(-DEDITOR=1)
endif()

if( "$ENV{NO_RESULT}" STREQUAL "1" )
	add_definitions(-DDISABLE_RESULTS)
endif()

if( "$ENV{DISABLE_RESULTS}" STREQUAL "1" )
	add_definitions(-DDISABLE_RESULTS)
endif()

add_definitions(-DTEST_COMPILE_ACHITECTURE="${PLATFORM}")

include_directories(${KCL_PATH_PUBLIC}/src
	${KCL_LIB_PATH_PUBLIC}/tinyxml/src
	${TFW_ROOT}
	${TFW_ROOT}/schemas
	${CMAKE_CURRENT_LIST_DIR}
	${CMAKE_CURRENT_LIST_DIR}/components
	${CMAKE_CURRENT_LIST_DIR}/../gfxb_dev/src)


include(${TFW_ROOT}/testfw.cmake)
include(apply_find_vars)

apply_find_vars(ZLIB PNG NGRTL)

list(APPEND SOURCES
	CPUQuery.h
	CPUQuery.cpp

	test_base_gfx.h
	test_base_gfx.cpp

	test_descriptor.h
	test_descriptor.cpp

	render_statistics.h
	render_statistics.cpp

	gfxb_barrier.h
	gfxb_barrier.cpp
	gfxb_font.h
	gfxb_font.cpp
	gfxb_font_renderer.h
	gfxb_font_renderer.cpp

	system_messager.h
	)


foreach(api ${RENDER_API})
	if(api STREQUAL Metal)
		list(APPEND SOURCES
				context_ngl_adapter_metal.h
				context_ngl_adapter_metal.mm)
	endif()
endforeach()


list(APPEND GFXB_COMPONENTS
	components/test_component.h
	components/test_component.cpp

	components/time_component.h
	components/time_component.cpp

	components/input_component.h
	components/input_component.cpp

	components/screenshot_component.h
	components/screenshot_component.cpp

	components/statistics_component.h
	components/statistics_component.cpp

	components/charts_component.h
	components/charts_component.cpp

	components/vsync_component.h
	components/vsync_component.cpp

	components/screen_manager_component.h
	components/screen_manager_component.cpp
	)

source_group( "" FILES ${SOURCES} )
source_group( "COMPONENTS" FILES ${GFXB_COMPONENTS} )
include(compiler_flags)

if(NOT TARGET testbase)
	add_library(testbase STATIC ${SOURCES} ${GFXB_COMPONENTS} )

	target_link_libraries(testbase ngl_api ksl_compiler ${NGRTL_LIBRARIES} )

	if(CMAKE_GENERATOR STREQUAL Xcode)
		set_target_properties(testbase PROPERTIES XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC "YES")
	endif()

endif()
