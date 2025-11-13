cmake_minimum_required(VERSION 3.5)
project("ksl_compiler")

# include compiler flags
include(compiler_flags)

# Try to set compiler flags to support C++11/C++0x features
if(${CMAKE_GENERATOR} MATCHES "Xcode")
	set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++11")
	set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++")
	set(XCODE_GENERATOR 1)
endif()
if(${CMAKE_CXX_COMPILER_ID} MATCHES "GNU" AND NOT XCODE_GENERATOR)
	if (${CMAKE_CXX_COMPILER_VERSION} VERSION_LESS "4.3")
		message(FATAL_ERROR "GNU c++ compiler version is less than 4.3 (no c++11 nor c++0x support): (" ${CMAKE_CXX_COMPILER_VERSION} ")")
	else()
		message("GNU c++ compiler version detected: ${CMAKE_CXX_COMPILER_VERSION}. Using gnu++0x and hope for the best")
		message("See: http://gcc.gnu.org/projects/cxx0x.html")
		if(QNX)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wno-unused-parameter -fno-strict-aliasing -Wno-strict-aliasing -Wno-invalid-offsetof")
        else()
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++0x")
		endif()
	endif()
endif()
if(${CMAKE_CXX_COMPILER_ID} MATCHES "Clang" AND NOT XCODE_GENERATOR)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -stdlib=libc++")
endif()

set(SOURCE_FILES
	src/ksl_generator.h
	src/ksl_generator.cpp
	src/ksl_analyzer.h
	src/ksl_analyzer.cpp
	src/ksl_analyzer_error.cpp
	src/ksl_analyzer_forcehighp.cpp
	src/ksl_analyzer_reflection.cpp
	src/ksl_analyzer_tables.cpp
	src/ksl_analyzer_validation.cpp
	src/ksl_ast_tree.h
	src/ksl_ast_tree.cpp
	src/ksl_common.h
	src/ksl_common.cpp
	src/ksl_compiler.h
	src/ksl_compiler.cpp
	src/ksl_tokenizer.h
	src/ksl_tokenizer.cpp
	src/ksl_tokenizer_debug.cpp
	src/ksl_translator.h
	src/ksl_translator.cpp
)


foreach(api ${RENDER_API})

	if( (api STREQUAL ES31) OR (api STREQUAL GL) )
		add_definitions(-DENABLE_GL_GENERATOR)

		list(APPEND SOURCE_FILES
				src/gl/ksl_gl_generator.h
				src/gl/ksl_gl_generator.cpp
				src/gl/ksl_gl_generator_base.h
				src/gl/ksl_gl_generator_base.cpp
				src/gl/ksl_gl_translator.h
				src/gl/ksl_gl_translator.cpp
			)
	endif()

	if(api STREQUAL VULKAN)
		add_definitions(-DENABLE_VULKAN_GENERATOR)

		list(APPEND SOURCE_FILES
				src/gl/ksl_gl_generator_base.h
				src/gl/ksl_gl_generator_base.cpp
				src/gl/ksl_gl_translator.h
				src/gl/ksl_gl_translator.cpp

				src/vulkan/ksl_vk_nodes.h
				src/vulkan/ksl_vk_generator.h
				src/vulkan/ksl_vk_generator.cpp
				src/vulkan/ksl_vk_translator.h
				src/vulkan/ksl_vk_translator.cpp
			)
	endif()

	if( (api STREQUAL D3D11) OR (api STREQUAL D3D12) )
		add_definitions(-DENABLE_D3D_GENERATOR)

		list(APPEND SOURCE_FILES
		        src/d3d/ksl_d3d_nodes.h
				src/d3d/ksl_d3d_generator.h
				src/d3d/ksl_d3d_generator.cpp
				src/d3d/ksl_d3d_generator_texture_access.cpp
				src/d3d/ksl_d3d_translator.h
				src/d3d/ksl_d3d_translator.cpp
			)
	endif()

	if(api STREQUAL Metal)
		add_definitions(-DENABLE_METAL_GENERATOR)

		list(APPEND SOURCE_FILES
				src/metal/ksl_mtl_nodes.h
				src/metal/ksl_mtl_translator.h
				src/metal/ksl_mtl_translator.cpp
				src/metal/ksl_mtl_generator.h
				src/metal/ksl_mtl_generator.cpp
				src/metal/ksl_mtl_generator_texture_access.cpp
			)
	endif()

endforeach()

add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} ngl_api)

target_include_directories(${PROJECT_NAME} PUBLIC
	${CMAKE_CURRENT_LIST_DIR}/src
	../ngl/src
)

