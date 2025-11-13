cmake_minimum_required(VERSION 3.5)
project( ngl_api)

##################################################################
set(VULKAN_VERSION "v1.0.3")
set(SUPPORTED_RENDER_API "NULL;GL;ES31;D3D11;D3D12;VULKAN;Metal")
set(RENDER_API "" CACHE STRING "help: ${SUPPORTED_RENDER_API}")
##################################################################


if(WIN32)
	#4804 unsafe use of type 'bool'
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4804")
endif()

set(AGILITY_SDK_INCLUDES ${CMAKE_CURRENT_LIST_DIR}/../../../3rdparty/AgilitySDK/Include)

include(choose_display_protocol) # NOTE this is used to set up Vulkan

if(RENDER_API STREQUAL "")
	message(FATAL_ERROR "RENDER_API must be set one of: ${SUPPORTED_RENDER_API}")
endif()

foreach(api ${RENDER_API})

	list(FIND SUPPORTED_RENDER_API ${api} status)
	if(status EQUAL -1)
		message(FATAL_ERROR "RENDER_API contain an invalid API ${api};\nSupported API: ${SUPPORTED_RENDER_API}")
	endif()
endforeach()

set( TARGET_SOURCES
	${CMAKE_CURRENT_LIST_DIR}/ngl.cpp
	${CMAKE_CURRENT_LIST_DIR}/ngl.h
	${CMAKE_CURRENT_LIST_DIR}/ngl_internal.h
	${CMAKE_CURRENT_LIST_DIR}/job_statistic.h
	${CMAKE_CURRENT_LIST_DIR}/parser.cpp
	${CMAKE_CURRENT_LIST_DIR}/parser.h
	${CMAKE_CURRENT_LIST_DIR}/shader_reflection.h)


if(NOT TARGET lib_glsl_pp)
	add_subdirectory(glslpp)
endif()


foreach(api ${RENDER_API})

	if(api STREQUAL NULL)
		#have to check null implementation because ADAS_Dashboard has a
		#missing OGLX header?
		list(APPEND TARGET_SOURCES
			${CMAKE_CURRENT_LIST_DIR}/null/ngl_null.h
			${CMAKE_CURRENT_LIST_DIR}/null/ngl_null.cpp)
		list(APPEND TARGET_DEFINITIONS -DENABLE_NULL_IMPLEMENTATION)
	endif()

	if(api STREQUAL ES31)
		find_package(OGLX REQUIRED)
		message(STATUS "OGLX_headers: ${OGLX_INCLUDE_DIRS}")
		message(STATUS "OGLX_libs: ${OGLX_LIBRARIES}")

		list(APPEND TARGET_DEFINITIONS -DENABLE_GL_IMPLEMENTATION)
		list(APPEND TARGET_LIBRARIES oglx)
		list(APPEND TARGET_SOURCES
			${CMAKE_CURRENT_LIST_DIR}/ogl_gles31_functions.cpp
			${CMAKE_CURRENT_LIST_DIR}/ogl_gles31_functions.h
			${CMAKE_CURRENT_LIST_DIR}/ogl_gles30_functions.cpp
			${CMAKE_CURRENT_LIST_DIR}/ogl_gles30_functions.h
			${CMAKE_CURRENT_LIST_DIR}/ogl.cpp
			${CMAKE_CURRENT_LIST_DIR}/ogl.h)
	endif()

	if(api STREQUAL GL)
		find_package(OGLX REQUIRED)
		message(STATUS "OGLX_headers: ${OGLX_INCLUDE_DIRS}")
		message(STATUS "OGLX_libs: ${OGLX_LIBRARIES}")

		if(WIN32)
			list(APPEND TARGET_LIBRARIES opengl32)
		endif()

		list(APPEND TARGET_DEFINITIONS -DENABLE_GL_IMPLEMENTATION)
		list(APPEND TARGET_LIBRARIES oglx)
		list(APPEND TARGET_SOURCES
			${CMAKE_CURRENT_LIST_DIR}/ogl_gles31_functions.cpp
			${CMAKE_CURRENT_LIST_DIR}/ogl_gles31_functions.h
			${CMAKE_CURRENT_LIST_DIR}/ogl_gles30_functions.cpp
			${CMAKE_CURRENT_LIST_DIR}/ogl_gles30_functions.h
			${CMAKE_CURRENT_LIST_DIR}/ogl.cpp
			${CMAKE_CURRENT_LIST_DIR}/ogl.h
			${CMAKE_CURRENT_LIST_DIR}/ngl_gl_adapter_interface.h)
	endif()

	if(api STREQUAL D3D11)
		list(APPEND TARGET_LIBRARIES
			 d3d11.lib
			 dxgi.lib
			 d3dcompiler.lib
			 dxguid.lib)

		list(APPEND TARGET_DEFINITIONS -DENABLE_D3D11_IMPLEMENTATION)
		list(APPEND TARGET_SOURCES
			${CMAKE_CURRENT_LIST_DIR}/d3d11.cpp
			${CMAKE_CURRENT_LIST_DIR}/d3d11.h
			)
	endif()

	if(api STREQUAL D3D12)
		list(APPEND TARGET_LIBRARIES
			d3d11.lib
			d3dcompiler.lib
			dxguid.lib
			d3d12.lib
			dxgi.lib)

		list(APPEND TARGET_DEFINITIONS -DENABLE_D3D12_IMPLEMENTATION)

		list(APPEND TARGET_SOURCES
			${CMAKE_CURRENT_LIST_DIR}/d3d12/ngl_d3d12.h
			${CMAKE_CURRENT_LIST_DIR}/d3d12/ngl_d3d12.cpp
			${CMAKE_CURRENT_LIST_DIR}/d3d12/ngl_d3d12_renderer.h
			${CMAKE_CURRENT_LIST_DIR}/d3d12/ngl_d3d12_renderer.cpp
			${CMAKE_CURRENT_LIST_DIR}/d3d12/ngl_d3d12_command.h
			${CMAKE_CURRENT_LIST_DIR}/d3d12/ngl_d3d12_command.cpp
			${CMAKE_CURRENT_LIST_DIR}/d3d12/ngl_d3d12_bind.h
			${CMAKE_CURRENT_LIST_DIR}/d3d12/ngl_d3d12_bind.cpp
			${CMAKE_CURRENT_LIST_DIR}/d3d12/ngl_d3d12_resource.h
			${CMAKE_CURRENT_LIST_DIR}/d3d12/ngl_d3d12_resource.cpp
			${CMAKE_CURRENT_LIST_DIR}/d3d12/ngl_d3d12_job.h
			${CMAKE_CURRENT_LIST_DIR}/d3d12/ngl_d3d12_job.cpp
			${CMAKE_CURRENT_LIST_DIR}/d3d12/ngl_d3d12_util.h
			${CMAKE_CURRENT_LIST_DIR}/d3d12/ngl_d3d12_util.cpp
			${CMAKE_CURRENT_LIST_DIR}/d3d12/ngl_d3d12_memory.h
			${CMAKE_CURRENT_LIST_DIR}/d3d12/ngl_d3d12_memory.cpp
			${CMAKE_CURRENT_LIST_DIR}/d3d12/ngl_d3d12_define.h
			${CMAKE_CURRENT_LIST_DIR}/d3d12/ngl_d3d12_action.h
			${CMAKE_CURRENT_LIST_DIR}/d3d12/ngl_d3d12_init.cpp
			)
	endif()
	if(api STREQUAL VULKAN)

		if(VK_NULL)
			list(APPEND TARGET_SOURCES ${VULKAN_VERSION}/nulldrv/nulldrv.c)
		else()
			if(WIN32)
				find_package(Vulkan REQUIRED)

				set(VULKAN_ARCH windows)
				set(VULKAN_LIBS ${VULKAN_LIBRARY})
			elseif (ANDROID)
				set(VULKAN_ARCH android)
				list(APPEND TARGET_SOURCES vulkan_wrapper.cpp)
			elseif(QNX)
				find_package(Vulkan)
				set(VULKAN_ARCH qnx)
				set(VULKAN_LIBS ${VULKAN_LIBRARIES})
				message("Found vulkan lib: ${VULKAN_LIBS}")
				list (APPEND TARGET_HEADERS
					${VULKAN_INCLUDE_DIRS}
					)
			else()

				if(DEFINED ENV{VULKAN_LIB_PATH})
					find_library(VULKAN_LIBS libvulkan.so
						PATHS $ENV{VULKAN_LIB_PATH}
						NO_CMAKE_FIND_ROOT_PATH)
					message("Found vulkan lib: ${VULKAN_LIBS}")
				endif()

				if("${VULKAN_LIBS}" STREQUAL "")
					find_package(Vulkan REQUIRED)
					set(VULKAN_LIBS ${VULKAN_LIBRARY})
				endif()

				set(VULKAN_ARCH linux)
				if("${VULKAN_LIBS}" STREQUAL "")
					set(VULKAN_LIBS "${CMAKE_CURRENT_LIST_DIR}/${VULKAN_VERSION}/${VULKAN_ARCH}/lib/libvulkan.so")
				endif()
			endif()
		endif()

		add_subdirectory(glslang_spirv0x10000.3)
		list(APPEND TARGET_HEADERS
			${GLSLANG_INCLUDE_DIRS}
			${VULKAN_VERSION}
			)

		list(APPEND TARGET_DEFINITIONS -DENABLE_VULKAN_IMPLEMENTATION)
		list(APPEND TARGET_LIBRARIES ${VULKAN_LIBS} ${GLSLANG_LIBRARIES})
		list(APPEND TARGET_SOURCES
			glsl_compiler.cpp
			glsl_compiler.h
			${VULKAN_VERSION}/vk.cpp
			${VULKAN_VERSION}/vk.h
			)
	endif()

	if(api STREQUAL Metal)

		list(APPEND TARGET_DEFINITIONS -DENABLE_METAL_IMPLEMENTATION)

		list(APPEND TARGET_SOURCES
			${CMAKE_CURRENT_LIST_DIR}/metal/ngl_metal.mm
			${CMAKE_CURRENT_LIST_DIR}/metal/ngl_metal.h
			${CMAKE_CURRENT_LIST_DIR}/metal/ngl_metal_misc.mm
			${CMAKE_CURRENT_LIST_DIR}/metal/ngl_metal_resources.mm
			${CMAKE_CURRENT_LIST_DIR}/metal/ngl_metal_renderer.mm
			${CMAKE_CURRENT_LIST_DIR}/metal/ngl_metal_adapter_interface.h
			)
	endif()
endforeach()

if(CMAKE_GENERATOR STREQUAL Xcode)
	set_source_files_properties(${TARGET_SOURCES} PROPERTIES COMPILE_FLAGS "-x objective-c++")
endif()

source_group( "src" FILES ${TARGET_SOURCES} )

include(compiler_flags)

if(TARGET ngl_api)
	return()
endif()

add_library( ngl_api STATIC ${TARGET_SOURCES} )

foreach(api ${RENDER_API})
	if(api STREQUAL Metal)
		set_target_properties(ngl_api PROPERTIES XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC "YES")
		if (APPLE AND NOT IOS)
			target_link_libraries(ngl_api PUBLIC "-framework QuartzCore -framework Metal")
		endif()
		break()
	endif()
endforeach()

message(STATUS "Agility_headers: ${AGILITY_SDK_INCLUDES}")

target_include_directories(ngl_api PUBLIC
	${CMAKE_CURRENT_LIST_DIR}
	${AGILITY_SDK_INCLUDES}
	${OGLX_INCLUDE_DIRS}
	${TARGET_HEADERS})
target_compile_definitions(ngl_api PUBLIC -DGLEW_NO_GLU ${OGLX_DEFINITIONS} ${TARGET_DEFINITIONS})
target_link_libraries(ngl_api PUBLIC lib_glsl_pp ${TARGET_LIBRARIES} ${OGLX_LIBRARIES})
