set(PUBLIC_HEADERS
	test_base0.h
	test_descriptor.h
	printable_result.h
)

set(SRC_COMMON
	testfw_functions.cpp
	newnotificationmanager.cpp
	test_descriptor.cpp
	render_statistics.cpp
	xml_utils.cpp
	printable_result.cpp
	test_base.cpp
)

set(GL_WRAPPER
    	opengl/gl_wrapper/gl_wrapper.h
		opengl/gl_wrapper/gl_wrapper_platform.h
		opengl/gl_wrapper/private/gl_wrapper_private.h
		opengl/gl_wrapper/private/gl_wrapper_private.cpp
		opengl/gl_wrapper/private/gl_wrapper_gl31_gl3platform.h
		opengl/gl_wrapper/private/gl_wrapper_gl31_khrplatform.h
		opengl/gl_wrapper/private/gl_wrapper_gl31_tokens.h
		opengl/gl_wrapper/private/gl_wrapper_gl31_functions.h
		opengl/gl_wrapper/private/gl_wrapper_gl31_impl.cpp
)

SOURCE_GROUP("gl_wrapper" FILES ${GL_WRAPPER})

if ("${OPT_GRAPHICS}" STREQUAL "GL")
	list(APPEND SRC_COMMON
		opengl/bitmask.h
		opengl/bitmask.cpp
		opengl/misc2_opengl.h
		opengl/ext.cpp
		opengl/fbo.cpp
		opengl/glbshader.cpp
		opengl/shader.cpp
		opengl/shader.h
		opengl/glb_shader_common.cpp
		opengl/glb_shader_common.h
		opengl/glwindow.cpp
		opengl/misc2.cpp
		opengl/misc2_opengl.cpp
		opengl/glb_opengl_state_manager.cpp
		opengl/glb_discard_functions.cpp
		opengl/glb_image.cpp
		opengl/glb_texture.cpp
		opengl/notificationman_opengl.cpp
		opengl/offscrman_opengl.cpp
		opengl/texture.cpp
		opengl/newnotificationmanager_opengl.cpp
		opengl/glb_image.h
		opengl/glb_texture.h
		opengl/notificationman.h
		opengl/offscrman.h
		opengl/texture.h
		opengl/glb_discard_functions.h
		opengl/newnotificationmanager_opengl.h
		opengl/glwindow.h
		opengl/glbshader.h
		opengl/fbo.h
		opengl/ext.h
		opengl/glb_opengl_limits.h
		opengl/glb_opengl_limits.cpp
		opengl/glb_opengl_debug_output.h
		opengl/glb_opengl_debug_output.cpp
		${GL_WRAPPER}
		opengl/vbopool.h
		opengl/vbopool.cpp
	)
elseif ("${OPT_GRAPHICS}" STREQUAL "METAL")
	list(APPEND SRC_COMMON
		metal/newnotificationmanager_metal.h
		metal/newnotificationmanager_metal.mm
		metal/offscrman.h
		metal/offscrman_metal.mm
		metal/mtl_texture.h
		metal/mtl_texture.mm
		metal/mtl_globals.h
		metal/fbo.h
		metal/fbo_metal.h
		metal/fbo_metal.mm
		metal/mtl_shader_helper.h
		metal/mtl_shader_helper.mm
		metal/mtl_types.h
		metal/mtl_types.mm
		metal/mtl_dynamic_data_buffer.h
		metal/mtl_dynamic_data_buffer.mm
		metal/mtl_quadBuffer.h
		metal/mtl_quadBuffer.mm
	)
elseif ("${OPT_GRAPHICS}" STREQUAL "DX")
	list(APPEND SRC_COMMON
		d3d11/csvstream.cpp
		d3d11/dxb_buffer.cpp
		d3d11/dxb_image.cpp
		d3d11/dxb_state_manager.cpp
		d3d11/dxb_texture.cpp
		d3d11/fbo3.cpp
		d3d11/misc2.cpp
		d3d11/newnotificationmanager_dx.cpp
		d3d11/notificationman_dx.cpp
		d3d11/offscrman_dx.cpp
		d3d11/texture.cpp
		d3d11/DX.cpp
		d3d11/dxb_base.cpp
		d3d11/results.cpp
		d3d11/resultsconverter.cpp
		d3d11/shader.h
		d3d11/shader.cpp
		d3d11/vbopool.h
		d3d11/vbopool.cpp
	)
endif()

set(SRC_COMMON_headers
	${PUBLIC_HEADERS}
	test_base.h
	glb_kcl_adapter.h
	misc2.h
	nulldriver.h
	render_statistics.h
	render_statistics_defines.h
)

if (${OPT_GRAPHICS} STREQUAL "GL")
	list(APPEND SRC_COMMON_headers
		opengl/glb_image.h
		opengl/glb_texture.h
		opengl/notificationman.h
		opengl/offscrman.h
		opengl/texture.h
	)
elseif (${OPT_GRAPHICS} STREQUAL "METAL")
	list(APPEND SRC_COMMON_headers)
elseif (${OPT_GRAPHICS} STREQUAL "DX")
	list(APPEND SRC_COMMON_headers
		d3d11/dxb_buffer.h
		d3d11/dxb_image.h
		d3d11/dxb_state_manager.h
		d3d11/DX.h
	)
endif()

SOURCE_GROUP("" FILES ${SRC_COMMON} ${SRC_COMMON_headers})


set(INCL_COMMON
	${KCL_LIB_PATH_PUBLIC}/libz/src
	${KCL_LIB_PATH_PUBLIC}/libpng/src
	${KCL_LIB_PATH_PUBLIC}/libetc1/src
	${KCL_LIB_PATH_PUBLIC}/tinyxml/src
	${KCL_PATH_PUBLIC}/src

	${PROJECT_SOURCE_DIR}/src
	${PROJECT_SOURCE_DIR}/src/common

	${PROJECT_SOURCE_DIR}/../frameworks/testfw
)

if ("${OPT_GRAPHICS}" STREQUAL "METAL")
	list(APPEND INCL_COMMON
		${PROJECT_SOURCE_DIR}/src/common/metal
		${PROJECT_SOURCE_DIR}/../frameworks/testfw/schemas
	)
endif()

include_directories(
	${INCL_COMMON}
	${CMAKE_CURRENT_LIST_DIR}/../../../frameworks/krl)

if(FALSE) # (${EDITION} STREQUAL "community" AND NOT IOS)

set(LIB_NAME glb_common_es2)
add_library(${LIB_NAME} STATIC ${SRC_COMMON} ${SRC_COMMON_headers})
set_target_properties(${LIB_NAME} PROPERTIES CXX_STANDARD 11)
target_compile_definitions(${LIB_NAME} PRIVATE ${OGLX_DEFINITIONS_ES2})

if ("${OPT_GRAPHICS}" STREQUAL "GL" OR "${OPT_GRAPHICS}" STREQUAL "METAL")
	set_target_properties(${LIB_NAME} PROPERTIES XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC "YES")
endif()

endif()#community

if(CMAKE_GENERATOR STREQUAL Xcode)
    if ("${OPT_GRAPHICS}" STREQUAL "GL" OR "${OPT_GRAPHICS}" STREQUAL "METAL")
        set_source_files_properties(test_descriptor.cpp PROPERTIES COMPILE_FLAGS "-x objective-c++")
    endif()
endif()

set(LIB_NAME glb_common_es3)
if ("${OPT_GRAPHICS}" STREQUAL "DX")
	set(LIB_NAME glb_common_dx3)
endif()

add_library(${LIB_NAME} STATIC ${SRC_COMMON} ${SRC_COMMON_headers})
set_target_properties(${LIB_NAME} PROPERTIES CXX_STANDARD 11)
target_compile_definitions(${LIB_NAME} PRIVATE ${OGLX_DEFINITIONS})


if ("${OPT_GRAPHICS}" STREQUAL "GL" OR "${OPT_GRAPHICS}" STREQUAL "METAL")
	set_target_properties(${LIB_NAME} PROPERTIES XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC "YES")
endif()
