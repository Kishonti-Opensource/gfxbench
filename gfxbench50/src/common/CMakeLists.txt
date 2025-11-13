cmake_minimum_required(VERSION 2.8)
project(gfxb_common)

set(COMMON_SRC
	gfxb_tools.h
	gfxb_tools.cpp
	gfxb_cascaded_shadow_map.cpp
	gfxb_cascaded_shadow_map.h
	gfxb_shadow_map.h
	gfxb_shadow_map.cpp
	gfxb_compute_motion_blur.cpp
	gfxb_compute_motion_blur.h
	gfxb_cubemap.cpp
	gfxb_cubemap.h
	gfxb_debug_renderer.cpp
	gfxb_debug_renderer.h
	gfxb_environment.cpp
	gfxb_environment.h
	gfxb_factories.cpp
	gfxb_factories.h
	gfxb_frustum_cull.cpp
	gfxb_frustum_cull.h
	gfxb_dof.cpp
	gfxb_dof.h
	gfxb_hdr.cpp
	gfxb_hdr.h
	gfxb_light.cpp
	gfxb_light.h
	gfxb_lightshaft.cpp
	gfxb_lightshaft.h
	gfxb_material.cpp
	gfxb_material.h
	gfxb_mesh.h
	gfxb_mesh.cpp
	gfxb_mesh_shape.cpp
	gfxb_mesh_shape.h
	gfxb_particlesystem.cpp
	gfxb_particlesystem.h
	gfxb_texture_flipbook.cpp
	gfxb_texture_flipbook.h
	gfxb_scene_base.cpp
	gfxb_scene_base.h
	gfxb_scheduler.cpp
	gfxb_scheduler.h
	gfxb_shader.cpp
	gfxb_shader.h
	gfxb_shapes.cpp
	gfxb_shapes.h
	gfxb_shot_handler.cpp
	gfxb_shot_handler.h
	gfxb_texture.cpp
	gfxb_texture.h
	gfxb_scene_test_base.cpp
	gfxb_scene_test_base.h
	gfxb_separable_blur.cpp
	gfxb_separable_blur.h
	gfxb_fragment_blur.cpp
	gfxb_fragment_blur.h
	gfxb_gauss_blur_helper.cpp
	gfxb_gauss_blur_helper.h
	gfxb_gen_mipmaps.cpp
	gfxb_gen_mipmaps.h
	gfxb_bloom.h
	gfxb_bloom.cpp
	gfxb_warmup_helper.h
	gfxb_warmup_helper.cpp

	gfxb_bilateral_fragment_blur.cpp
	gfxb_bilateral_fragment_blur.h
	gfxb_bloom_mobile.h
	gfxb_bloom_mobile.cpp
	gfxb_bloom_downsample_blur.h
	gfxb_bloom_downsample_blur.cpp
	gfxb_bloom_upsample_blur.h
	gfxb_bloom_upsample_blur.cpp
	)

source_group( "" FILES ${COMMON_SRC} )

find_package(Poco REQUIRED Foundation)
apply_find_vars(POCO)

add_library(gfxb_common STATIC ${COMMON_SRC})

if(NOT TARGET ksl_compiler)
	add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../../../frameworks/ksl_compiler ksl_compiler)
endif()

if(NOT TARGET KCL)
	find_package(KCL REQUIRED)
	apply_find_vars(KCL)
	add_library(KCL INTERFACE)
endif()
target_link_libraries(gfxb_common ${POCO_LIBRARIES} KCL ngl_api ksl_compiler)
