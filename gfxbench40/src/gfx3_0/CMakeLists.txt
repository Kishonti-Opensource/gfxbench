cmake_minimum_required(VERSION 3.5)
project(gfx3)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../../../frameworks/krl krl)
include_directories(${KRL_INCLUDE_DIRS})
set(ALL_LIBS ${ALL_LIBS} KRL)

if ("${OPT_GRAPHICS}" STREQUAL "GL")
	list(APPEND SRC_KRL
		${CMAKE_CURRENT_LIST_DIR}/../es2_common/glb_scene_.cpp
		${CMAKE_CURRENT_LIST_DIR}/../es2_common/glb_scene_.h
	)
endif()

set(SRC_HIGH_LEVEL_TESTS_3_0_ENGINE
	${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/engine.h
	${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/engine.cpp
)

list(APPEND SRC_HIGH_LEVEL_TESTS_3_0
	${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/spacepartition.h
	${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/spacepartition.cpp
	${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/qualitymatch.h
	${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/qualitymatch.cpp
	${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/test_wrapper.h
	${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/test_wrapper.cpp
	${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/battery_test.h
	${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/battery_test.cpp
	${SRC_KRL}
)

set(SRC_LOW_LEVEL_BASE
	${CMAKE_CURRENT_LIST_DIR}/low_level_tests/alu_base.cpp
	${CMAKE_CURRENT_LIST_DIR}/low_level_tests/alu_base.h
	${CMAKE_CURRENT_LIST_DIR}/low_level_tests/blending_base.cpp
	${CMAKE_CURRENT_LIST_DIR}/low_level_tests/blending_base.h
	${CMAKE_CURRENT_LIST_DIR}/low_level_tests/driver_base.cpp
	${CMAKE_CURRENT_LIST_DIR}/low_level_tests/driver_base.h
	${CMAKE_CURRENT_LIST_DIR}/low_level_tests/fill_base.cpp
	${CMAKE_CURRENT_LIST_DIR}/low_level_tests/fill_base.h
)

file (GLOB SRC_LOW_LEVELS_GL "${CMAKE_CURRENT_LIST_DIR}/low_level_tests/opengl/*.h"
      					     "${CMAKE_CURRENT_LIST_DIR}/low_level_tests/opengl/*.cpp")

if ("${OPT_GRAPHICS}" STREQUAL "GL")
	set(SRC_LOW_LEVEL_BASE ${SRC_LOW_LEVEL_BASE} ${SRC_LOW_LEVELS_GL})
endif()

if ("${OPT_GRAPHICS}" STREQUAL "DX")
	set(SRC_LOW_LEVEL_BASE
		${CMAKE_CURRENT_LIST_DIR}/low_level_tests/d3d11/new_low_level_base_dx.cpp
		${CMAKE_CURRENT_LIST_DIR}/low_level_tests/d3d11/new_low_level_base_dx.h
		${CMAKE_CURRENT_LIST_DIR}/low_level_tests/d3d11/dx_alu.cpp
		${CMAKE_CURRENT_LIST_DIR}/low_level_tests/d3d11/dx_alu.h
		${CMAKE_CURRENT_LIST_DIR}/low_level_tests/d3d11/dx_blending.cpp
		${CMAKE_CURRENT_LIST_DIR}/low_level_tests/d3d11/dx_blending.h
		${CMAKE_CURRENT_LIST_DIR}/low_level_tests/d3d11/dx_driver.cpp
		${CMAKE_CURRENT_LIST_DIR}/low_level_tests/d3d11/dx_driver.h
		${CMAKE_CURRENT_LIST_DIR}/low_level_tests/d3d11/dx_fill.cpp
		${CMAKE_CURRENT_LIST_DIR}/low_level_tests/d3d11/dx_fill.h
	)
endif()

if ("${OPT_GRAPHICS}" STREQUAL "METAL")
	set(SRC_LOW_LEVEL_BASE
		${SRC_LOW_LEVEL_BASE}
		${CMAKE_CURRENT_LIST_DIR}/low_level_tests/metal/mtl_low_level_base.h
		${CMAKE_CURRENT_LIST_DIR}/low_level_tests/metal/mtl_low_level_base.mm
		${CMAKE_CURRENT_LIST_DIR}/low_level_tests/metal/mtl_low_level.h
		${CMAKE_CURRENT_LIST_DIR}/low_level_tests/metal/mtl_alu.mm
		${CMAKE_CURRENT_LIST_DIR}/low_level_tests/metal/mtl_blending.mm
		${CMAKE_CURRENT_LIST_DIR}/low_level_tests/metal/mtl_driver.mm
		${CMAKE_CURRENT_LIST_DIR}/low_level_tests/metal/mtl_fill.mm
	)
endif()

if ("${OPT_GRAPHICS}" STREQUAL "GL")
	list(APPEND SRC_HIGH_LEVEL_TESTS_3_0
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/opengl/glb_scene_utils_opengl.cpp
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/opengl/cubemap.h
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/opengl/cubemap_opengl.cpp
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/opengl/glb_light.cpp
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/opengl/glb_material.h
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/opengl/glb_material.cpp
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/opengl/glb_material_opengl.cpp
		${CMAKE_CURRENT_LIST_DIR}/../es2_common/glb_mesh.h
		${CMAKE_CURRENT_LIST_DIR}/../es2_common/glb_mesh.cpp
		${CMAKE_CURRENT_LIST_DIR}/../es2_common/glb_mesh_opengl.cpp
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/opengl/glb_planarmap.h
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/opengl/glb_planarmap_opengl.cpp
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/opengl/glb_scene_opengl27.cpp
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/opengl/shadowmap.h
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/opengl/shadowmap_opengl.cpp
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/opengl/glb_scene.h
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/opengl/glb_scene.cpp
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/opengl/xiph_decode/glb_ogg_decoder.cpp
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/opengl/glb_particlesystem.h
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/opengl/glb_particlesystem.cpp
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/opengl/gbuffer.h
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/opengl/gbuffer.cpp
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/opengl/glb_scene_opengl.h
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/opengl/glb_scene_opengl.cpp
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/opengl/vao_storage.h
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/opengl/vao_storage.cpp
	)

elseif ("${OPT_GRAPHICS}" STREQUAL "METAL")
	list(APPEND SRC_HIGH_LEVEL_TESTS_3_0
		${SRC_LOW_LEVEL_BASE}

		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/metal/vbopool.h
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/metal/vbopool.cpp

		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/metal/shader.h
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/metal/shader.mm

		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/metal/mtl_light.h
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/metal/mtl_light.mm

		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/metal/mtl_emitterAdvectConsts.h
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/metal/mtl_factories.h
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/metal/mtl_framebuffer_27.h
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/metal/mtl_framebuffer_27.mm
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/metal/mtl_framebuffer_30.h
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/metal/mtl_framebuffer_30.mm
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/metal/mtl_frameConsts.h
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/metal/mtl_instanced_particle_layout.h
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/metal/mtl_lensflare_30.h
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/metal/mtl_lensflare_30.mm
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/metal/mtl_lightbuffer.h
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/metal/mtl_lightbuffer.mm
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/metal/mtl_lightConsts.h
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/metal/mtl_matConsts.h
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/metal/mtl_material.h
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/metal/mtl_material.mm
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/metal/mtl_mesh.h
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/metal/mtl_mesh.mm
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/metal/mtl_meshConsts.h
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/metal/mtl_ogg_decoder.h
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/metal/mtl_ogg_decoder.mm
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/metal/mtl_particle_render_30.mm
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/metal/mtl_particlesystem.h
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/metal/mtl_particlesystem.mm
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/metal/mtl_pipeline.h
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/metal/mtl_pipeline.mm
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/metal/mtl_pipeline_shader_source_helper.mm
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/metal/mtl_planarmap.h
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/metal/mtl_planarmap.mm
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/metal/mtl_scene_base.h
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/metal/mtl_scene_base.mm
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/metal/mtl_scene_27.h
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/metal/mtl_scene_27.mm
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/metal/mtl_scene_30.h
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/metal/mtl_scene_30.mm
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/metal/mtl_scene_30_imp.h
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/metal/mtl_scene_30_imp.mm
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/metal/mtl_shader_constant_layouts_27.h
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/metal/mtl_shader_constant_layouts_30.h
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/metal/mtl_shadowmap.h
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/metal/mtl_shadowmap.mm
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/metal/mtl_cubemap.mm
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/metal/mtl_cubemap.h
	)
elseif ("${OPT_GRAPHICS}" STREQUAL "DX")
	list(APPEND SRC_HIGH_LEVEL_TESTS_3_0
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/d3d11/cubemap.h
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/d3d11/cubemap_dx.cpp
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/d3d11/dxb_light.cpp
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/d3d11/dxb_material.h
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/d3d11/dxb_material.cpp
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/d3d11/dxb_material_dx.cpp
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/d3d11/dxb_mesh.h
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/d3d11/dxb_mesh.cpp
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/d3d11/dxb_emitter.h
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/d3d11/dxb_emitter.cpp
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/d3d11/dxb_planarmap.h
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/d3d11/dxb_planarmap_dx.cpp
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/d3d11/dxb_scene_27.cpp
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/d3d11/shadowmap.h
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/d3d11/shadowmap_dx.cpp
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/d3d11/dxb_scene.h
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/d3d11/dxb_scene.cpp
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/d3d11/dxb_scene_30.h
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/d3d11/dxb_scene_30.cpp
	)
endif()

if(ENABLE_FRAME_CAPTURE)
	list(APPEND SRC_HIGH_LEVEL_TESTS_3_0 ../framecapture/capturewrapper.cpp)
endif()

SOURCE_GROUP("" FILES ${SRC_HIGH_LEVEL_TESTS_3_0})

list(APPEND INCL_GFX30
	${CMAKE_CURRENT_LIST_DIR}/../common
	${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3
	${CMAKE_CURRENT_LIST_DIR}/low_level_tests

	${KCL_PATH_PUBLIC}/src
	${KCL_LIB_PATH_PUBLIC}/tinyxml/src
	${KCL_LIB_PATH_PUBLIC}/libz/src
)

if ("${OPT_GRAPHICS}" STREQUAL "GL")
	list(APPEND INCL_GFX30
	)
elseif ("${OPT_GRAPHICS}" STREQUAL "METAL")
	list(APPEND INCL_GFX30
		${CMAKE_CURRENT_LIST_DIR}/../common/metal
		${CMAKE_CURRENT_LIST_DIR}/low_level_tests
		${CMAKE_CURRENT_LIST_DIR}/low_level_tests/metal
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/metal

	)
elseif ("${OPT_GRAPHICS}" STREQUAL "DX")
	list(APPEND INCL_GFX30
		${PROJECT_SOURCE_DIR}/../frameworks/testfw # TODO HACK
	)
endif()

include_directories(${INCL_GFX30})

set(LIB_NAME gfxbench30_shared)
if ("${OPT_GRAPHICS}" STREQUAL "DX")
	set(LIB_NAME gfxshared30_dx)
endif()

if(NOT TARGET ${LIB_NAME})
	if(FALSE) # (${EDITION} STREQUAL "community" AND NOT IOS)
		add_library(gfxbench30_shared_es2 STATIC ${SRC_HIGH_LEVEL_TESTS_3_0})
		set_target_properties(gfxbench30_shared_es2 PROPERTIES CXX_STANDARD 11)
		target_compile_definitions(gfxbench30_shared_es2 PRIVATE ${OGLX_DEFINITIONS_ES2})
	endif()

	add_library(${LIB_NAME} STATIC ${SRC_HIGH_LEVEL_TESTS_3_0})
	set_target_properties(${LIB_NAME} PROPERTIES CXX_STANDARD 11)
	target_compile_definitions(${LIB_NAME} PRIVATE ${OGLX_DEFINITIONS})
	if ("${OPT_GRAPHICS}" STREQUAL "METAL")
		set_target_properties(${LIB_NAME} PROPERTIES XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC "YES")
	endif()

endif()
