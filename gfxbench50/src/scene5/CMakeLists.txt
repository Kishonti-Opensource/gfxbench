cmake_minimum_required(VERSION 3.5)
project(gfxb_5)

if(CMAKE_GENERATOR STREQUAL Xcode)
	find_library( GL_FRAMEWORK OpenGLES)
	find_library( METAL_FRAMEWORK Metal)
	find_library( UIKIT_FRAMEWORK UIKit)
	find_library( QUARTZCORE_FRAMEWORK QuartzCore)
endif()

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../testbase gfxb_base_bin)

set(TFW_ROOT ${CMAKE_CURRENT_LIST_DIR}/../../../frameworks/testfw)
include_directories(
	${CMAKE_CURRENT_LIST_DIR}/src
	${CMAKE_CURRENT_LIST_DIR}/../gfxb_base)

include(compiler_flags)

set(SHADER_ROOT "../../shaders/" )
set(SHADERS
	bright_pass.frag
	dof.frag
	downsample.frag
	final.frag
	fire.frag
	forward.frag
	gauss_blur_fs.frag
	gauss_blur_fs_lod.frag
	glow.frag
	ibl.frag
	integrate_brdf.frag
	light_combine_simple.frag
	light_directional.frag
	light_emissive.frag
	light_irradiance.frag
	light_omni.frag
	light_spot.frag
	main.frag
	prefilter_envmap.frag
	sao.frag
	shadow_caster.frag
	sky.frag
	billboard.h
	common.h
	fog.h
	sun_shadow.h
	tonemapper.h
	hdr_reduction_pass1.shader
	hdr_reduction_pass2.shader
	bright_pass.vert
	dof.vert
	downsample.vert
	final.vert
	fire.vert
	forward.vert
	gauss_blur_fs.vert
	glow.vert
	ibl.vert
	integrate_brdf.vert
	light_combine_simple.vert
	light_directional.vert
	light_irradiance.vert
	light_omni.vert
	light_spot.vert
	main.vert
	prefilter_envmap.vert
	sao.vert
	shadow_caster.vert
	sky.vert
)
if(MSVC)
foreach(SHADER ${SHADERS})
	list(APPEND PREFIXED_SHADERS ${SHADER_ROOT}/scene5/${SHADER})
endforeach()
source_group( "shaders" FILES ${PREFIXED_SHADERS} )
endif()

if(NOT BUILD_OVR_APP)
list(APPEND SOURCES
	${PREFIXED_SHADERS}
	gfxb_5.cpp
	gfxb_5.h
	gfxb_scene5.cpp
	gfxb_scene5.h
	gfxb_scene5_init.cpp
	gfxb_scene5_tools.cpp
	gfxb_scene5_tools.h
	gfxb_scene5_mesh.cpp
	gfxb_scene5_mesh.h
	gfxb_scene5_mesh_filters.cpp
	gfxb_scene5_mesh_filters.h
	gfxb_sun_light.h
	gfxb_sun_light.cpp
	gfxb_lightshaft_util.h
	gfxb_lightshaft_util.cpp
	gfxb_room_setup.h
	gfxb_room_setup.cpp
	gfxb_scene5_pipeline.h
	gfxb_scene5_pipeline.cpp
	gi/gi.cpp
	gi/gi.h
)
endif()

include_directories(
	${KCL_PATH_PUBLIC}/src
	${CMAKE_CURRENT_LIST_DIR}/../../../frameworks/testfw
	${CMAKE_CURRENT_LIST_DIR}/../../../frameworks/testfw/schemas
)


if(OPT_COMMUNITY_BUILD)
	include_directories(${CMAKE_CURRENT_LIST_DIR}/community)
	find_package(netman REQUIRED)
	apply_find_vars(NETMAN)
	list(APPEND SOURCES "${CMAKE_CURRENT_LIST_DIR}/community/create_factory_gfx50_community.cpp")
else()
	include_directories(${CMAKE_CURRENT_LIST_DIR}/corporate)
	if(NOT BUILD_OVR_APP)
		list(APPEND SOURCES "${CMAKE_CURRENT_LIST_DIR}/corporate/create_factory_gfx50_corporate.cpp")
	endif()
endif()


if(BUILD_OVR_APP)
	list( APPEND SOURCES
		gfxb_ovr_gen.h
		gfxb_5_ovr_tools.h

		#android ovr viewer test
		gfxb_scene5_ovr.cpp
		gfxb_scene5_ovr.h
		gfxb_scene5_ovr_test.cpp
		gfxb_scene5_ovr_test.h
		gfxb_5_ovr_ngl_adapter.h
		gfxb_ovr_settings_manager.h
		gfxb_ovr_mesh.h
		gfxb_ovr_mesh.cpp
		gfxb_ovr_mesh_filters.cpp
		gfxb_ovr_mesh_filters.h
	)
endif()

source_group( "" FILES ${SOURCES} )

find_package(Poco REQUIRED Foundation)
apply_find_vars(POCO)

if(MSVC)
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /DELAYLOAD:vulkan-1.dll")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /DELAYLOAD:d3d12.dll")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /DELAYLOAD:d3dcompiler_47.dll")
endif()

add_library(gfxb_5 ${SOURCES})

if(NOT WIN32)
    target_compile_options(gfxb_5 PRIVATE -Wno-deprecated-declarations)
endif()

if(ANDROID)
	target_link_libraries(gfxb_5 -landroid)
elseif(APPLE AND NOT IOS)
	find_library(FOUNDATION_LIBRARIES Foundation)
	find_library(IOKIT_LIBRARIES IOKit)
	find_library(APPKIT_LIBRARIES AppKit)
	target_link_libraries(gfxb_5 ${APPKIT_LIBRARIES} ${FOUNDATION_LIBRARIES} ${IOKIT_LIBRARIES})
endif()

target_link_libraries(gfxb_5
	gfxb_common
	testbase
	KCL
	ngl_api
	tfw_schemas
	${KCL_LIBRARIES}
	${NGRTL_LIBRARIES}
	${PNG_LIBRARIES}
	${GLEW_LIBRARIES}
	${POCO_LIBRARIES}
	${APPLY_FIND_LIBRARIES}
	)

if(${WITH_OVR_SDK})
	target_link_libraries(gfxb_5 ${TFW_ROOT}/ovr/Libs/Android/armeabi-v7a/libvrapi.so)
endif()


if ("${PRODUCT_ID}" STREQUAL "gfxbench_dx")
	register_testfw_module( gfxb_5 "dx_5")
else()
	## factories for other products??
	if(BUILD_OVR_APP)
		register_testfw_module( gfxb_5 "gl_5_ovr_2")
	else()
		register_testfw_module( gfxb_5 ";gl_5")
	endif()
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
