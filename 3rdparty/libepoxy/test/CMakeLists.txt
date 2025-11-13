cmake_minimum_required(VERSION 2.8)

project(epoxy-test)

if(X11_FOUND)
	list(APPEND tests
		glx_beginend
		glx_public_api
		glx_public_api_core
		glx_glxgetprocaddress_nocontext
		glx_has_extension_nocontext
		glx_static
	)
	if (NOT APPLE)
		list(APPEND tests
			glx_gles2
			# glx_alias_prefer_same_name # Makefile.am set the same source files for this target and has compiler errors
		)
	endif()
	add_library(glx_common STATIC glx_common.c glx_common.h)
	add_dependencies(glx_common epoxy)
	set(glx_common_lib glx_common)
endif()

if(EGL_FOUND)
	list(APPEND tests
		egl_has_extension_nocontext
		egl_gl
	)
	add_library(egl_common STATIC egl_common.c egl_common.h)
	add_dependencies(egl_common epoxy)
	set(egl_common_lib egl_common)
endif()

find_package(OpenGL)
if(OPENGL_gl_LIBRARY)
	set(other_libraries ${OPENGL_gl_LIBRARY})
endif()
foreach(testfile ${tests})
	get_filename_component(testcase ${testfile} NAME_WE)
	add_executable(${testcase} ${testfile}.c)
	add_test(${testcase} ${testcase})
	add_definitions(${${testcase}_DEFINITIONS})
	target_link_libraries(${testcase} epoxy ${egl_common_lib} ${glx_common_lib} ${X11_LIBRARIES} ${other_libraries})
endforeach()

add_definitions(-fvisibility=default)
macro(generate_test test)
	add_executable(${test} ${${test}_SOURCES})
	add_test(${test} ${test})
	set_target_properties(${test} PROPERTIES COMPILE_FLAGS "${${test}_CPPFLAGS}")
	target_link_libraries(${test} ${${test}_LDFLAGS} ${${test}_LDADD})
endmacro()

set(egl_gles1_without_glx_CPPFLAGS "-DGLES_VERSION=1")
set(egl_gles1_without_glx_SOURCES egl_without_glx.c)
set(egl_gles1_without_glx_LDADD epoxy egl_common ${DL_LIBRARIES})

set(egl_gles2_without_glx_CPPFLAGS "-DGLES_VERSION=2")
set(egl_gles2_without_glx_SOURCES egl_without_glx.c)
set(egl_gles2_without_glx_LDADD epoxy egl_common ${DL_LIBRARIES})

set(egl_and_glx_different_pointers_egl_SOURCES egl_and_glx_different_pointers.c dlwrap.c dlwrap.h)
set(egl_and_glx_different_pointers_egl_LDADD ${egl_common_lib} glx_common ${DL_LIBRARIES} epoxy ${X11_LIBRARIES})
set(egl_and_glx_different_pointers_egl_LDFLAGS "-rdynamic")
set(egl_and_glx_different_pointers_egl_CPPFLAGS "-DUSE_EGL -fvisibility=default")

set(egl_and_glx_different_pointers_glx_SOURCES egl_and_glx_different_pointers.c dlwrap.c dlwrap.h)
set(egl_and_glx_different_pointers_glx_LDADD ${egl_common_lib} glx_common ${DL_LIBRARIES} epoxy ${X11_LIBRARIES})
set(egl_and_glx_different_pointers_glx_LDFLAGS "-rdynamic")
set(egl_and_glx_different_pointers_glx_CPPFLAGS "-DUSE_GLX -fvisibility=default")

set(egl_and_glx_different_pointers_egl_glx_SOURCES egl_and_glx_different_pointers.c dlwrap.c dlwrap.h)
set(egl_and_glx_different_pointers_egl_glx_LDADD egl_common glx_common ${DL_LIBRARIES} epoxy ${X11_LIBRARIES})
set(egl_and_glx_different_pointers_egl_glx_LDFLAGS "-rdynamic")
set(egl_and_glx_different_pointers_egl_glx_CPPFLAGS "-DUSE_EGL -DUSE_GLX -fvisibility=default")

if (EGL_FOUND)
	generate_test(egl_gles1_without_glx)
	generate_test(egl_gles2_without_glx)
endif()
if (EGL_FOUND AND X11_FOUND AND NOT APPLE)
	generate_test(egl_and_glx_different_pointers_glx)
	generate_test(egl_and_glx_different_pointers_egl)
	generate_test(egl_and_glx_different_pointers_egl_glx)
endif()
