include(${CMAKE_CURRENT_LIST_DIR}/../../gfx4/CMakeLists.txt)

##
##	GL
##
if(${OPT_GRAPHICS} STREQUAL "GL")


if(${EDITION} STREQUAL "corporate")
	set(FACTORY_ES2_SOURCES "${CMAKE_CURRENT_LIST_DIR}/${EDITION}/create_factory_gfx27_corporate.cpp")
else()
	set(FACTORY_ES2_SOURCES "${CMAKE_CURRENT_LIST_DIR}/${EDITION}/create_factory_gfx27_community.cpp")
endif()

if(FALSE) # (${EDITION} STREQUAL "community" AND NOT IOS)
	set(es2_target_name "gfxbench40_gl_es2")
	add_library(${es2_target_name} ${SHARED_GFXSOURCES} ${FACTORY_ES2_SOURCES} ${SRC_HIGH_LEVEL_TESTS_3_0_ENGINE} ${SRC_LOW_LEVEL_BASE})
	set_target_properties(${es2_target_name} PROPERTIES CXX_STANDARD 11)
	target_compile_definitions(${es2_target_name} PRIVATE OPT_TEST_GFX2 OPT_TEST_GFX30 ${OGLX_DEFINITIONS_ES2})
	target_link_libraries(${es2_target_name} gfxbench30_shared_es2 glb_common_es2 ${ALL_LIBS} ${OGLX_LIBRARIES_ES2})
	register_testfw_module(${es2_target_name} "gl_trex;gl_trex_off;gl_trex_qmatch;gl_trex_qmatch_highp;gl_trex_battery;gl_alu;gl_alu_off;gl_fill;gl_fill_off;gl_driver;gl_driver_off;gl_blending;gl_blending_off")
	include("${CMAKE_CURRENT_LIST_DIR}/../../../setup_importlib_properties")
endif()

if(${EDITION} STREQUAL "corporate")
	list(APPEND FACTORY_SOURCES
		${CMAKE_CURRENT_LIST_DIR}/${EDITION}/create_factory_gfx40_corporate.cpp
		${CMAKE_CURRENT_LIST_DIR}/${EDITION}/create_factory_gfx31_corporate.cpp
		${CMAKE_CURRENT_LIST_DIR}/${EDITION}/create_factory_lowlevel_corporate.cpp
		${CMAKE_CURRENT_LIST_DIR}/${EDITION}/create_factory_trex_corporate.cpp
		${CMAKE_CURRENT_LIST_DIR}/${EDITION}/create_factory_egypt_corporate.cpp)
else()
	list(APPEND FACTORY_SOURCES "${CMAKE_CURRENT_LIST_DIR}/${EDITION}/create_factory_gfx40_community.cpp")
endif()


if(TARGET gfxbench40_gl)
	return()
endif()

set(target_name "gfxbench40_gl")
add_library(${target_name} ${SHARED_GFXSOURCES} ${SRC_HIGH_LEVEL_TESTS_4_0} ${SRC_LOW_LEVEL_TESTS_4_0} ${FACTORY_SOURCES})
set_target_properties(${target_name} PROPERTIES CXX_STANDARD 11)
target_compile_definitions(${target_name} PRIVATE OPT_TEST_GFX2 OPT_TEST_GFX30 OPT_TEST_GFX31 ${OGLX_DEFINITIONS})

if(NOT TARGET gfxbench31_gl)
	add_library(gfxbench31_gl STATIC ${SRC_HIGH_LEVEL_TESTS_3_1} ${SRC_LOW_LEVEL_TESTS_3_1})
	set_target_properties(gfxbench31_gl PROPERTIES CXX_STANDARD 11)
	target_compile_definitions(gfxbench31_gl PRIVATE OPT_TEST_GFX2 OPT_TEST_GFX30 OPT_TEST_GFX31 ${OGLX_DEFINITIONS})
	target_include_directories(gfxbench31_gl PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../../../resources/data/gfx/shaders_40/shaders.31)
	target_link_libraries(${target_name} gfxbench30_shared gfxbench31_gl glb_common_es3 gfxbench30_shared ${ALL_LIBS} ${OGLX_LIBRARIES})
endif()

target_link_libraries(${target_name} gfxbench30_shared ${ALL_LIBS} glb_common_es3)
target_include_directories(${target_name} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../../../resources/data/gfx/shaders_40/shaders.4)
target_compile_definitions(${target_name} PRIVATE OPT_TEST_GFX2 OPT_TEST_GFX30 OPT_TEST_GFX31 OPT_TEST_GFX40)

register_testfw_module(${target_name} "gl_egypt;gl_egypt_off;gl_4;gl_4_off;gl_tess;gl_tess_off;gl_precision;gl_manhattan31;gl_manhattan31_off;gl_manhattan31_battery;gl_manhattan;gl_manhattan_off;gl_trex;gl_trex_off;gl_trex_battery;gl_alu2;gl_alu2_off;gl_fill2;gl_fill2_off;gl_driver2;gl_driver2_off;gl_alu;gl_alu_off;gl_fill;gl_fill_off;gl_driver;gl_driver_off;gl_blending;gl_blending_off")


##
##	Metal
##
elseif(${OPT_GRAPHICS} STREQUAL "METAL")


set(target_name "gfxbench40_metal")
add_definitions(-DOPT_TEST_GFX40)
aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/${EDITION} SOURCES)

add_library(${target_name}  ${SHARED_GFXSOURCES} ${SRC_HIGH_LEVEL_TESTS_4_0} ${SRC_LOW_LEVEL_TESTS_4_0} ${SOURCES})

if(NOT TARGET gfxbench31_metal)
	add_definitions(-DOPT_TEST_GFX2 -DOPT_TEST_GFX30 -DOPT_TEST_GFX31)
	add_library(gfxbench31_metal STATIC ${SRC_HIGH_LEVEL_TESTS_3_1} ${SRC_LOW_LEVEL_TESTS_3_1})
	target_include_directories(gfxbench31_metal PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../../../resources/data/gfx/shaders_mtl/shaders.31)
	target_link_libraries(${target_name} gfxbench30_shared gfxbench31_metal ${ALL_LIBS})
	set_target_properties(gfxbench31_metal PROPERTIES XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC "YES")
endif()

target_link_libraries(${target_name} gfxbench30_shared gfxbench31_metal glb_common_es3 ${ALL_LIBS})

register_testfw_module(${target_name} "metal_4;metal_4_off;metal_tess;metal_tess_off;metal_precision;metal_manhattan31;metal_manhattan31_off;metal_manhattan;metal_manhattan_off;metal_trex;metal_trex_off;metal_trex_battery;metal_alu2;metal_alu2_off;metal_fill2;metal_fill2_off;metal_driver2;metal_driver2_off")

set_target_properties(${target_name} PROPERTIES XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC "YES")
	
if(MACOSX)
	target_link_libraries(${target_name} "-framework Metal")
endif()

include("${CMAKE_CURRENT_LIST_DIR}/../../../setup_importlib_properties")

  
target_include_directories(${target_name} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../../../resources/data/gfx/shaders_mtl/shaders.40)

endif()
