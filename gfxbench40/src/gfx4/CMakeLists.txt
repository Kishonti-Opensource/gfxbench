cmake_minimum_required(VERSION 3.5)
project(gfx4)

include(${CMAKE_CURRENT_LIST_DIR}/../common_31/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/../gfx3_1/CMakeLists.txt)



if(BENCHMARK_GUI)
	add_definitions(-DHAVE_GUI_FOLDER)
	include(${CMAKE_CURRENT_LIST_DIR}/../gui/CMakeLists.txt)
	list(REMOVE_ITEM SRC_GUI "${GUI_ROOT}/gfxgui31.h" "${GUI_ROOT}/gfxgui31.cpp")
endif()

set(ROOT30 ${CMAKE_CURRENT_LIST_DIR}/../gfx3_0)
set(ROOT31 ${CMAKE_CURRENT_LIST_DIR}/../gfx3_1)

include_directories(
	${ROOT30}/high_level_tests_3

	${KCL_PATH_PUBLIC}/src
	${KCL_LIB_PATH_PUBLIC}/tinyxml/src
	${KCL_LIB_PATH_PUBLIC}/libz/src

)

if ("${OPT_GRAPHICS}" STREQUAL "GL")
	set(SRC_HIGH_LEVEL_TESTS_4_0
		${SRC_COMMON31}

		${ROOT30}/high_level_tests_3/qualitymatch.h
		${ROOT30}/high_level_tests_3/qualitymatch.cpp
		${ROOT30}/high_level_tests_3/test_wrapper.h
		${ROOT30}/high_level_tests_3/test_wrapper.cpp
		${ROOT30}/high_level_tests_3/battery_test.h
		${ROOT30}/high_level_tests_3/battery_test.cpp

		${ROOT30}/high_level_tests_3/engine.cpp

		${ROOT30}/high_level_tests_3/opengl/cubemap.h
		${ROOT30}/high_level_tests_3/opengl/cubemap_opengl.cpp
		${ROOT30}/high_level_tests_3/opengl/glb_light.cpp
		${ROOT30}/high_level_tests_3/opengl/glb_material.h
		${ROOT30}/high_level_tests_3/opengl/glb_material.cpp
		${ROOT30}/high_level_tests_3/opengl/glb_material_opengl.cpp
		${ROOT30}/../es2_common/glb_mesh.h
		${ROOT30}/../es2_common/glb_mesh.cpp
		${ROOT30}/../es2_common/glb_mesh_opengl.cpp
		${ROOT30}/high_level_tests_3/opengl/glb_planarmap.h
		${ROOT30}/high_level_tests_3/opengl/glb_planarmap_opengl.cpp
		${ROOT30}/high_level_tests_3/opengl/shadowmap.h
		${ROOT30}/high_level_tests_3/opengl/shadowmap_opengl.cpp
		${ROOT30}/high_level_tests_3/opengl/xiph_decode/glb_ogg_decoder.cpp
		${ROOT30}/high_level_tests_3/opengl/glb_particlesystem.h
		${ROOT30}/high_level_tests_3/opengl/glb_particlesystem.cpp
		${ROOT30}/high_level_tests_3/opengl/vao_storage.h
		${ROOT30}/high_level_tests_3/opengl/vao_storage.cpp


		${CMAKE_CURRENT_LIST_DIR}/../gfx4/opengl/glb_scene_opengl4.h
		${CMAKE_CURRENT_LIST_DIR}/../gfx4/opengl/glb_scene_opengl4.cpp
		${CMAKE_CURRENT_LIST_DIR}/../gfx4/opengl/glb_scene_opengl4_support.h
		${CMAKE_CURRENT_LIST_DIR}/../gfx4/opengl/glb_scene_opengl4_support.cpp
		${CMAKE_CURRENT_LIST_DIR}/../gfx4/opengl/glb_gbuffer.h
		${CMAKE_CURRENT_LIST_DIR}/../gfx4/opengl/glb_gbuffer.cpp
		${CMAKE_CURRENT_LIST_DIR}/../gfx4/opengl/glb_compute_motion_blur.h
		${CMAKE_CURRENT_LIST_DIR}/../gfx4/opengl/glb_compute_motion_blur.cpp
		${CMAKE_CURRENT_LIST_DIR}/../gfx4/opengl/glb_material4.h
		${CMAKE_CURRENT_LIST_DIR}/../gfx4/opengl/glb_material4.cpp
		${CMAKE_CURRENT_LIST_DIR}/../gfx4/opengl/glb_cascaded_shadow_map.h
		${CMAKE_CURRENT_LIST_DIR}/../gfx4/opengl/glb_cascaded_shadow_map.cpp
		${CMAKE_CURRENT_LIST_DIR}/../gfx4/opengl/glb_stride_blur.h
		${CMAKE_CURRENT_LIST_DIR}/../gfx4/opengl/glb_stride_blur.cpp
		${CMAKE_CURRENT_LIST_DIR}/../gfx4/opengl/glb_debug_renderer.h
		${CMAKE_CURRENT_LIST_DIR}/../gfx4/opengl/glb_debug_renderer.cpp
		${CMAKE_CURRENT_LIST_DIR}/../gfx4/opengl/glb_particlesystem4.h
		${CMAKE_CURRENT_LIST_DIR}/../gfx4/opengl/glb_particlesystem4.cpp
		${CMAKE_CURRENT_LIST_DIR}/../gfx4/opengl/glb_occlusion_cull.h
		${CMAKE_CURRENT_LIST_DIR}/../gfx4/opengl/glb_occlusion_cull.cpp
		${CMAKE_CURRENT_LIST_DIR}/../gfx4/opengl/glb_lensflare.h
		${CMAKE_CURRENT_LIST_DIR}/../gfx4/opengl/glb_lensflare.cpp
		${CMAKE_CURRENT_LIST_DIR}/../gfx4/opengl/glb_compute_hdr.h
		${CMAKE_CURRENT_LIST_DIR}/../gfx4/opengl/glb_compute_hdr.cpp
		${CMAKE_CURRENT_LIST_DIR}/../gfx4/opengl/gfxb4_mesh.h
		${CMAKE_CURRENT_LIST_DIR}/../gfx4/opengl/gfxb4_mesh.cpp
		${CMAKE_CURRENT_LIST_DIR}/../gfx4/opengl/compute_reduction4.h
		${CMAKE_CURRENT_LIST_DIR}/../gfx4/opengl/compute_reduction4.cpp

		${SRC_KRL}
		${SRC_GUI}
	)

	file (GLOB SRC_LOW_LEVEL_TESTS_4_0 "${CMAKE_CURRENT_LIST_DIR}/low_level_tests/opengl/*.h" "${CMAKE_CURRENT_LIST_DIR}/low_level_tests/opengl/*.cpp")

	list (APPEND SRC_LOW_LEVEL_TESTS_4_0
		${CMAKE_CURRENT_LIST_DIR}/low_level_tests/tess_base.h
		${CMAKE_CURRENT_LIST_DIR}/low_level_tests/tess_base.cpp
	)

elseif ("${OPT_GRAPHICS}" STREQUAL "METAL")
	set(SRC_HIGH_LEVEL_TESTS_4_0
		${SRC_COMMON31}

		${ROOT30}/high_level_tests_3/engine.cpp

		${CMAKE_CURRENT_LIST_DIR}/../gfx4/metal/mtl_scene_40.h
		${CMAKE_CURRENT_LIST_DIR}/../gfx4/metal/mtl_scene_40.mm
		${CMAKE_CURRENT_LIST_DIR}/../gfx4/metal/mtl_scene_40_support.h
		${CMAKE_CURRENT_LIST_DIR}/../gfx4/metal/mtl_scene_40_support.mm
		${CMAKE_CURRENT_LIST_DIR}/../gfx4/metal/mtl_gbuffer.h
		${CMAKE_CURRENT_LIST_DIR}/../gfx4/metal/mtl_gbuffer.mm
		${CMAKE_CURRENT_LIST_DIR}/../gfx4/metal/mtl_compute_motion_blur.h
		${CMAKE_CURRENT_LIST_DIR}/../gfx4/metal/mtl_compute_motion_blur.mm
		${CMAKE_CURRENT_LIST_DIR}/../gfx4/metal/mtl_material4.h
		${CMAKE_CURRENT_LIST_DIR}/../gfx4/metal/mtl_material4.mm
		${CMAKE_CURRENT_LIST_DIR}/../gfx4/metal/mtl_cascaded_shadow_map.h
		${CMAKE_CURRENT_LIST_DIR}/../gfx4/metal/mtl_cascaded_shadow_map.mm
		${CMAKE_CURRENT_LIST_DIR}/../gfx4/metal/mtl_stride_blur.h
		${CMAKE_CURRENT_LIST_DIR}/../gfx4/metal/mtl_stride_blur.mm
		${CMAKE_CURRENT_LIST_DIR}/../gfx4/metal/mtl_particlesystem4.h
		${CMAKE_CURRENT_LIST_DIR}/../gfx4/metal/mtl_particlesystem4.mm
		${CMAKE_CURRENT_LIST_DIR}/../gfx4/metal/mtl_occlusion_cull.h
		${CMAKE_CURRENT_LIST_DIR}/../gfx4/metal/mtl_occlusion_cull.mm
		${CMAKE_CURRENT_LIST_DIR}/../gfx4/metal/mtl_lensflare.h
		${CMAKE_CURRENT_LIST_DIR}/../gfx4/metal/mtl_lensflare.mm
		${CMAKE_CURRENT_LIST_DIR}/../gfx4/metal/mtl_mesh4.h
		${CMAKE_CURRENT_LIST_DIR}/../gfx4/metal/mtl_mesh4.mm
		${CMAKE_CURRENT_LIST_DIR}/../gfx4/metal/mtl_tessellator.h
		${CMAKE_CURRENT_LIST_DIR}/../gfx4/metal/mtl_tessellator.mm
		${CMAKE_CURRENT_LIST_DIR}/../gfx4/metal/mtl_compute_reduction40.h
		${CMAKE_CURRENT_LIST_DIR}/../gfx4/metal/mtl_compute_reduction40.mm
		${CMAKE_CURRENT_LIST_DIR}/../gfx4/metal/mtl_compute_hdr40.h
		${CMAKE_CURRENT_LIST_DIR}/../gfx4/metal/mtl_compute_hdr40.mm

		${SRC_KRL}
		${SRC_GUI}
	)

	if(0)
	set(SRC_LOW_LEVEL_TESTS_4_0
		${CMAKE_CURRENT_LIST_DIR}/low_level_tests/metal/mtl_low_level4.h
		${CMAKE_CURRENT_LIST_DIR}/low_level_tests/metal/mtl_tess.mm

		${CMAKE_CURRENT_LIST_DIR}/low_level_tests/tess_base.h
		${CMAKE_CURRENT_LIST_DIR}/low_level_tests/tess_base.cpp
	)
	endif()
endif()

SOURCE_GROUP("" FILES ${SRC_HIGH_LEVEL_TESTS_4_0})

SOURCE_GROUP("gui" FILES ${SRC_GUI})

SOURCE_GROUP("low_level_tests" FILES ${SRC_LOW_LEVEL_TESTS_4_0})

set(INCL_GFX40
	${CMAKE_CURRENT_LIST_DIR}
	${CMAKE_CURRENT_LIST_DIR}/low_level_tests
	${CMAKE_CURRENT_LIST_DIR}/../common
	${CMAKE_CURRENT_LIST_DIR}/../common_31

	${KCL_PATH_PUBLIC}/src
	${KCL_LIB_PATH_PUBLIC}/libz/src
)

include_directories( ${INCL_GFX40})
