cmake_minimum_required(VERSION 3.5)
project(gfx3)

include(${CMAKE_CURRENT_LIST_DIR}/../common_31/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/../gfx3_0/CMakeLists.txt)

if(BENCHMARK_GUI)
	add_definitions(-DHAVE_GUI_FOLDER)
	include(${CMAKE_CURRENT_LIST_DIR}/../gui/CMakeLists.txt)

	list(REMOVE_ITEM SRC_GUI "${GUI_ROOT}/gfxgui.h" "${GUI_ROOT}/gfxgui.cpp")

endif()

set(SRC_LOW_LEVEL2_BASE
    ${CMAKE_CURRENT_LIST_DIR}/low_level_tests/alu2_base.h
    ${CMAKE_CURRENT_LIST_DIR}/low_level_tests/alu2_base.cpp
    ${CMAKE_CURRENT_LIST_DIR}/low_level_tests/fill2_base.h
    ${CMAKE_CURRENT_LIST_DIR}/low_level_tests/fill2_base.cpp
)

if ("${OPT_GRAPHICS}" STREQUAL "GL")
	set(SRC_HIGH_LEVEL_TESTS_3_1
		${SRC_COMMON31}
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/opengl/glb_scene_opengl31.h
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/opengl/glb_scene_opengl31.cpp
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/opengl/instancedlightrenderer.h
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/opengl/instancedlightrenderer.cpp
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/opengl/compute_lightning.h
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/opengl/compute_lightning.cpp
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/opengl/ubo_manager31.cpp
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/opengl/compute_hdr31.h
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/opengl/compute_hdr31.cpp
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/opengl/compute_reduction31.h
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/opengl/compute_reduction31.cpp

		${SRC_GUI}
	)
	set(SRC_LOW_LEVEL_TESTS_3_1
		${SRC_LOW_LEVEL_BASE}
		${SRC_LOW_LEVEL2_BASE}
		${SRC_COMMON31}
		${CMAKE_CURRENT_LIST_DIR}/low_level_tests/opengl/gl_alu2.h
		${CMAKE_CURRENT_LIST_DIR}/low_level_tests/opengl/gl_alu2.cpp
		${CMAKE_CURRENT_LIST_DIR}/low_level_tests/opengl/gl_fill2.h
		${CMAKE_CURRENT_LIST_DIR}/low_level_tests/opengl/gl_fill2.cpp
		${CMAKE_CURRENT_LIST_DIR}/low_level_tests/opengl/gl_driver2.h
		${CMAKE_CURRENT_LIST_DIR}/low_level_tests/opengl/gl_driver2.cpp
	)
elseif ("${OPT_GRAPHICS}" STREQUAL "METAL")
	set(SRC_LOW_LEVEL_TESTS_3_1
		${SRC_LOW_LEVEL_BASE}
		${SRC_LOW_LEVEL2_BASE}
		${SRC_COMMON31}
		${CMAKE_CURRENT_LIST_DIR}/low_level_tests/metal/mtl_low_level2.h
		${CMAKE_CURRENT_LIST_DIR}/low_level_tests/metal/mtl_alu2.mm
		${CMAKE_CURRENT_LIST_DIR}/low_level_tests/metal/mtl_driver2.mm
		${CMAKE_CURRENT_LIST_DIR}/low_level_tests/metal/mtl_fill2.mm
	)
	set(SRC_HIGH_LEVEL_TESTS_3_1
		${SRC_COMMON31}
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/metal/mtl_scene_31.h
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/metal/mtl_scene_31.mm
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/metal/mtl_compute_reduction31.h
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/metal/mtl_compute_reduction31.mm
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/metal/mtl_compute_hdr31.h
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/metal/mtl_compute_hdr31.mm
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/metal/mtl_instancedlightrenderer.h
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/metal/mtl_instancedlightrenderer.mm
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/metal/mtl_compute_lightning.h
		${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3/metal/mtl_compute_lightning.mm
	)
else()
	set(SRC_HIGH_LEVEL_TESTS_3_1
		${SRC_HIGH_LEVEL_TESTS_3_0}
		${SRC_KRL}
	)
endif()


SOURCE_GROUP("" FILES ${SRC_HIGH_LEVEL_TESTS_3_1} ${SRC_LOW_LEVEL_TESTS_3_1})
SOURCE_GROUP("gui" FILES ${SRC_GUI})

set(INCL_GFX31
	${CMAKE_CURRENT_LIST_DIR}/../common
	${CMAKE_CURRENT_LIST_DIR}/../common_31
	${CMAKE_CURRENT_LIST_DIR}/high_level_tests_3
	${CMAKE_CURRENT_LIST_DIR}/low_level_tests

	${KCL_PATH_PUBLIC}/src
	${KCL_LIB_PATH_PUBLIC}/libz/src
)

include_directories( ${INCL_GFX31})
