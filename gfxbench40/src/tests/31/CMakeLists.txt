include(${CMAKE_CURRENT_LIST_DIR}/../../gfx3_1/CMakeLists.txt)

if(${OPT_GRAPHICS} STREQUAL "GL")
	set(target_name "gfxbench31_gl")
	set(TESTFW_TESTS_LIST "gl_manhattan31;gl_manhattan31_off;gl_manhattan31_battery;gl_manhattan;gl_manhattan_off;gl_trex;gl_trex_off;gl_trex_battery;gl_trex_qmatch;gl_trex_qmatch_highp;gl_alu2;gl_alu2_off;gl_fill2;gl_fill2_off;gl_driver2;gl_driver2_off")
elseif(${OPT_GRAPHICS} STREQUAL "METAL")
	set(target_name "gfxbench31_metal")
	set(TESTFW_TESTS_LIST "metal_manhattan31;metal_manhattan31_off;metal_manhattan31_wqhd_off;metal_manhattan31_battery;metal_manhattan;metal_manhattan_off;metal_trex;metal_trex_off;metal_trex_battery;metal_alu2;metal_alu2_off;metal_fill2;metal_fill2_off;metal_driver2;metal_driver2_off")
elseif(${OPT_GRAPHICS} STREQUAL "DX")
	message(FATAL_ERROR "directx not supported")
endif()

if ("${OPT_GRAPHICS}" STREQUAL "METAL")
add_definitions(-DOPT_TEST_GFX2 -DOPT_TEST_GFX30 -DOPT_TEST_GFX31)
set(OGLX_LIBB )
else()
set(OGLX_LIBB oglx)
add_definitions(-DOPT_TEST_GFX2 -DOPT_TEST_GFX30 -DOPT_TEST_GFX31 ${OGLX_DEFINITIONS})
endif()


aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/${EDITION} SOURCES)
add_library(${target_name}  ${SHARED_GFXSOURCES} ${SRC_HIGH_LEVEL_TESTS_3_1} ${SRC_LOW_LEVEL_TESTS_3_1} ${SRC_HIGH_LEVEL_TESTS_3_0_ENGINE} ${SOURCES})
target_link_libraries(${target_name} gfxbench30_shared ${ALL_LIBS} gfxbench30_shared glb_common_es3 ${OGLX_LIBB})

register_testfw_module(${target_name} "${TESTFW_TESTS_LIST}")
register_testfw_module2(${target_name} "${TESTFW_TESTS_LIST}")

if ("${OPT_GRAPHICS}" STREQUAL "METAL")
	set_target_properties(${target_name} PROPERTIES XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC "YES")
	
	if(MACOSX)
		target_link_libraries(${target_name} "-framework Metal")
	endif()
endif()

include("${CMAKE_CURRENT_LIST_DIR}/../../../setup_importlib_properties")

if ("${OPT_GRAPHICS}" STREQUAL "GL")
	target_include_directories(${target_name} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../../../resources/data/gfx/shaders_40/shaders.31)
elseif ("${OPT_GRAPHICS}" STREQUAL "METAL")
	target_include_directories(${target_name} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../../../resources/data/gfx/shaders_mtl/shaders.31)
endif()
