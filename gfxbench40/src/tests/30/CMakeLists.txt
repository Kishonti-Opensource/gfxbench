include(${CMAKE_CURRENT_LIST_DIR}/../../gfx3_0/CMakeLists.txt)

if(${OPT_GRAPHICS} STREQUAL "GL")
	set(target_name "gfxbench30_gl")
	set(TESTFW_TESTS_LIST "gl_manhattan;gl_manhattan_off;gl_trex;gl_trex_off;gl_trex_battery;gl_trex_qmatch;gl_trex_qmatch_highp;gl_fill;gl_fill_off;gl_driver;gl_driver_off;gl_alu;gl_alu_off;gl_blending;gl_blending_off")
elseif(${OPT_GRAPHICS} STREQUAL "METAL")
	set(target_name "gfxbench30_metal")
	set(TESTFW_TESTS_LIST "metal_alu;metal_alu_off;metal_blending;metal_blending_off;metal_driver;metal_driver_off;metal_fill;metal_fill_off;metal_manhattan;metal_manhattan_off;metal_trex;metal_trex_off;metal_trex_battery")
elseif(${OPT_GRAPHICS} STREQUAL "DX")
	set(target_name "gfxbench30_dx")
	set(TESTFW_TESTS_LIST "dx_manhattan;dx_manhattan_off;dx_trex;dx_trex_off;dx_trex_battery;dx_trex_qmatch;dx_trex_qmatch_highp;dx_fill;dx_fill_off;dx_driver;dx_driver_off;dx_alu;dx_alu_off;dx_blending;dx_blending_off")
endif()


set(COMMON_LIB_NAME glb_common_es3)
set(SHARED_LIB_NAME gfxbench30_shared)
if ("${OPT_GRAPHICS}" STREQUAL "DX")
	set(COMMON_LIB_NAME glb_common_dx3)
	set(SHARED_LIB_NAME gfxshared30_dx)
endif()

add_definitions(-DOPT_TEST_GFX2 -DOPT_TEST_GFX30)
aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/${EDITION} SOURCES)
add_library(${target_name} ${SHARED_GFXSOURCES} ${SRC_HIGH_LEVEL_TESTS_3_0_ENGINE} ${SRC_LOW_LEVEL_BASE} ${SOURCES})
target_link_libraries(${target_name} ${SHARED_LIB_NAME} ${COMMON_LIB_NAME} ${ALL_LIBS})

register_testfw_module(${target_name} "${TESTFW_TESTS_LIST}")

if ("${OPT_GRAPHICS}" STREQUAL "METAL")
	set_target_properties(${target_name} PROPERTIES XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC "YES")
		
	if(MACOSX)
		target_link_libraries(${target_name} "-framework Metal")
	endif()
endif()

include("${CMAKE_CURRENT_LIST_DIR}/../../../setup_importlib_properties")
