cmake_minimum_required(VERSION 3.5)
project(epoxy)
enable_testing()

if ( NOT BUILD_SHARED_LIBS)
	add_definitions(-DEPOXY_STATIC=1)
endif()

if (NOT MSVC)
	add_definitions(-Wall)
endif()
if (MSVC_VERSION LESS 1800)
	include_directories(msvc)
endif()

if (IOS)
	add_definitions(-DIOS=1)
endif()

if ( (ANDROID OR IOS) AND COMMAND find_host_program)
    find_host_program(python python)
else()
    find_program(python python)
endif()
add_custom_command(OUTPUT ${CMAKE_SOURCE_DIR}/src/gl_generated_dispatch.c ${CMAKE_SOURCE_DIR}/include/epoxy/gl_generated.h
	COMMAND ${python} ${CMAKE_SOURCE_DIR}/src/gen_dispatch.py --dir ${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/registry/gl.xml
	DEPENDS ${CMAKE_SOURCE_DIR}/src/gen_dispatch.py ${CMAKE_SOURCE_DIR}/registry/gl.xml)
add_custom_command(OUTPUT ${CMAKE_SOURCE_DIR}/src/egl_generated_dispatch.c ${CMAKE_SOURCE_DIR}/include/epoxy/egl_generated.h
	COMMAND ${python} ${CMAKE_SOURCE_DIR}/src/gen_dispatch.py --dir ${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/registry/egl.xml
	DEPENDS ${CMAKE_SOURCE_DIR}/src/gen_dispatch.py ${CMAKE_SOURCE_DIR}/registry/egl.xml)
add_custom_command(OUTPUT ${CMAKE_SOURCE_DIR}/src/glx_generated_dispatch.c ${CMAKE_SOURCE_DIR}/include/epoxy/glx_generated.h
	COMMAND ${python} ${CMAKE_SOURCE_DIR}/src/gen_dispatch.py --dir ${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/registry/glx.xml
	DEPENDS ${CMAKE_SOURCE_DIR}/src/gen_dispatch.py ${CMAKE_SOURCE_DIR}/registry/glx.xml)
add_custom_command(OUTPUT ${CMAKE_SOURCE_DIR}/src/wgl_generated_dispatch.c ${CMAKE_SOURCE_DIR}/include/epoxy/wgl_generated.h
	COMMAND ${python} ${CMAKE_SOURCE_DIR}/src/gen_dispatch.py --dir ${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/registry/wgl.xml
	DEPENDS ${CMAKE_SOURCE_DIR}/src/gen_dispatch.py ${CMAKE_SOURCE_DIR}/registry/wgl.xml)
add_custom_target(generate_files DEPENDS
    ${CMAKE_SOURCE_DIR}/include/epoxy/gl_generated.h
    ${CMAKE_SOURCE_DIR}/include/epoxy/egl_generated.h
    ${CMAKE_SOURCE_DIR}/include/epoxy/glx_generated.h
    ${CMAKE_SOURCE_DIR}/include/epoxy/wgl_generated.h
)


set(sources
	include/epoxy/egl.h
	include/epoxy/epoxy_export.h
	include/epoxy/gl.h
	include/epoxy/glx.h
	include/epoxy/wgl.h
	src/dispatch_common.h
	src/dispatch_common.c
	src/gl_generated_dispatch.c

)
include_directories(include)
add_definitions(-DPLATFORM_DETECTED)

set(EGL_FOUND 1)
set(EGL_INCLUDE_DIRS ${CMAKE_CURRENT_LIST_DIR}/khronos/include)
add_definitions(-DPLATFORM_HAS_EGL=1)
include_directories(${EGL_INCLUDE_DIRS})
list(APPEND sources
	src/dispatch_egl.c
	src/egl_generated_dispatch.c
)

find_package(X11)
if (X11_FOUND)
	include_directories(${X11_INCLUDE_DIR})
	add_definitions(-DPLATFORM_HAS_GLX=1)
	list(APPEND sources
		src/dispatch_glx.c
		src/glx_generated_dispatch.c
	)
else()
	add_definitions(-DPLATFORM_HAS_GLX=0)
endif()

if(WIN32)
	add_definitions(-DPLATFORM_HAS_WGL=1)
	list(APPEND sources
		src/dispatch_wgl.c
		src/wgl_generated_dispatch.c
	)
endif()

file(GLOB headers include/epoxy/*.h)

add_library(epoxy
	${headers}
	${sources}
)
set_target_properties(epoxy PROPERTIES DEBUG_POSTFIX "_d")
target_link_libraries(epoxy ${CMAKE_DL_LIBS})
add_dependencies(epoxy generate_files)

if (NOT (MSVC OR IOS))
  #add_subdirectory(test) #disabele because it contains EGL and GL based tests
endif()

configure_file(epoxy-config.cmake.in ${CMAKE_BINARY_DIR}/epoxy-config.cmake @ONLY)
install(TARGETS epoxy DESTINATION lib)
install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/include/epoxy DESTINATION include PATTERN Makefile.am EXCLUDE)
install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/khronos/include/EGL ${CMAKE_CURRENT_LIST_DIR}/khronos/include/KHR DESTINATION include)
if (MSVC_VERSION LESS 1800)
	install(FILES ${CMAKE_CURRENT_LIST_DIR}/msvc/inttypes.h DESTINATION include)
	install(FILES ${CMAKE_CURRENT_LIST_DIR}/msvc/stdbool.h DESTINATION include)
	install(FILES ${CMAKE_CURRENT_LIST_DIR}/msvc/stdint.h DESTINATION include)
endif()
install(FILES ${CMAKE_BINARY_DIR}/epoxy-config.cmake DESTINATION share/epoxy/cmake)
