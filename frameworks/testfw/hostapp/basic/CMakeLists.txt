cmake_minimum_required(VERSION 3.5)
project(testfw_app)

unset(NO_GL CACHE)
if(DEFINED ENV{NO_GL} AND "$ENV{NO_GL}" STREQUAL "1")
    set(NO_GL YES CACHE BOOL "help msg")
endif()

set(CMAKE_MACOSX_RPATH 1)
include(apply_find_vars)

option(OPT_FIXUP_BUNDLE "Set to ON|OFF (default is ON) to fixup" ON)

if(MSVC)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /DELAYLOAD:vulkan-1.dll" )
endif()

# find dependencies

find_package(epoxy)
if (EPOXY_FOUND)
    apply_find_vars(epoxy)
    add_definitions(-DHAVE_EPOXY=1)
endif()

if (SILENCE_PLUGIN_LOAD_WARNINGS)
    add_definitions(-DSILENCE_PLUGIN_LOAD_WARNINGS=1)
endif()

if (NOT APPLE AND NOT ("$ENV{USE_WAYLAND}" STREQUAL "1"))
    find_package(X11)
endif()

#on desktop Linux where there is native EGL (e.g.: NVidia driver version larger than 334.xx )
#we have to skip finding the GLFW library because the hostapp is loading the libGL.so and
#the test load the libGLESv2.so too that mismatch it together and leads to crash
if(NOT ( "$ENV{OGLX_DRIVER}" STREQUAL "ES3" AND "$ENV{OGLX_VARIANT}" STREQUAL "custom"))

  if(NOT NO_GL)
    find_package(glfw CONFIG)
  endif()
  if (TARGET glfw AND NOT ("$ENV{USE_WAYLAND}" STREQUAL "1"))
      add_definitions(-DHAVE_GLFW3=1)
  endif()

endif()

find_package(Poco REQUIRED Util XML JSON Foundation)
find_package(ngrtl REQUIRED core)
apply_find_vars(NGRTL POCO)

if (NOT TARGET tfw_schemas)
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../../schemas schemas)
endif()
if (NOT TARGET tfw_deviceinfo)
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../../deviceinfo deviceinfo)
endif()

# add source
set(dir ${CMAKE_CURRENT_LIST_DIR}/../..)

set(sources
    ${dir}/graphics/nullgraphicscontext.h
    ${dir}/testfw.cpp
)

#if($ENV{NG_TARGET_PLATFORM} MATCHES "^(linux_cross|linux_arm|linux_arm64)$")
#    add_definitions("-DGLFW_INCLUDE_ES2")
#endif()

if($ENV{NG_TARGET_PLATFORM} MATCHES "^(linux_cross|linux_arm|linux_arm64)$")
    set(LINUX_ARM_TARGET TRUE)
endif()

if(NOT APPLE AND "${EXECUTABLE_NAME}" MATCHES "ADAS|Compu|adas|compu")
    if(LINUX_ARM_TARGET AND ${CMAKE_HOST_SYSTEM_PROCESSOR} MATCHES x86*)
        set(X_LIBS xcb Xau Xdmcp)
        foreach(lib IN LISTS X_LIBS)
        find_library(${lib}_PATH NAMES "${lib}" "lib${lib}.so")
        list(APPEND X_LIBRARIES ${${lib}_PATH})
        endforeach()
        list(APPEND platform_libraries ${X_LIBRARIES})
	endif()

    find_package(GStreamer REQUIRED)
    if (GSTREAMER_FOUND)
    	if(NOT (ANDROID OR LINUX_ARM_TARGET))
            list(APPEND GSTREAMER_LIBRARIES ${GSTREAMER_BASE_LIBRARIES} ${GSTREAMER_APP_LIBRARIES} -lgobject-2.0 -lglib-2.0)
        endif()
        if(LINUX_ARM_TARGET AND NOT (${CMAKE_HOST_SYSTEM_PROCESSOR} MATCHES x86*))
            find_package(PkgConfig REQUIRED)
            pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
            pkg_check_modules(DRM REQUIRED libdrm)
            message(STATUS ">>GTK3_INCLUDE_DIRS ${GTK3_INCLUDE_DIRS} GTK3_LIBRARIES ${GTK3_LIBRARIES} DRM_INCLUDE_DIRS ${DRM_INCLUDE_DIRS} DRM_LIBRARIES ${DRM_LIBRARIES}")
            list(APPEND GSTREAMER_LIBRARIES ${GTK3_LIBRARIES} ${DRM_LIBRARIES})
        endif()
        apply_find_vars(GSTREAMER)
        add_definitions(-DHAVE_GSTREAMER)
        list(APPEND sources ${dir}/videostream_gstreamer.cpp)
    endif()
endif()


if(NOT NO_GL)
    set(sources ${sources}
                ${dir}/graphics/eglgraphicscontext.h
                ${dir}/graphics/eglgraphicscontext.cpp
                ${dir}/graphics/windowfactory.h
                ${dir}/graphics/windowfactory.cpp
    )
endif()


list(APPEND sources
    ${dir}/graphics/vulkangraphicscontext.h
    ${dir}/graphics/vulkangraphicscontext.cpp
    )

if (WIN32)
    list(APPEND sources
        ${dir}/graphics/wglgraphicscontext.h
        ${dir}/graphics/wglgraphicscontext.cpp
        ${dir}/graphics/hwndgraphicswindow.h
        ${dir}/graphics/hwndgraphicswindow.cpp
        ${dir}/graphics/hwndmessagequeue.h
        ${dir}/graphics/hwndmessagequeue.cpp
    )

    include(CheckIncludeFiles)
    check_include_files(d3d11_1.h HAVE_DX11_H)

    if (HAVE_DX11_H)
        message("Enabling DXGraphicsContext")
        list(APPEND sources
            ${dir}/graphics/dxwin32graphicscontext.h
            ${dir}/graphics/dxwin32graphicscontext.cpp)
        set_property(SOURCE ${dir}/graphics/windowfactory.cpp PROPERTY COMPILE_DEFINITIONS "HAVE_D3D11")
    endif()
endif()
if (X11_FOUND)
    list(APPEND sources
        ${dir}/graphics/xgraphicswindow.h
        ${dir}/graphics/xgraphicswindow.cpp
    )
    include_directories(${X11_INCLUDE_DIR})
    add_definitions(-DHAVE_X11)
    list(APPEND platform_libraries ${X11_LIBRARIES})
endif()


find_package(EGL QUIET)
if(EGL_FOUND)
	add_definitions(-DHAVE_EGL)
	list(APPEND platform_libraries ${EGL_LIBRARIES})
endif()

include(choose_display_protocol)

if (APPLE AND NOT IOS AND NOT XCODE_VERSION VERSION_LESS 7.0)
    list(APPEND sources
        ${dir}/graphics/metalgraphicscontext.h
        ${dir}/graphics/metalgraphicscontext.mm
        ${dir}/graphics/metalgraphicswindow.h
        ${dir}/graphics/metalgraphicswindow.mm
        ${dir}/graphics/metalmessagequeue.h
        ${dir}/graphics/metalmessagequeue.mm
    )
endif()

if(QNX)
    list(APPEND sources
        ${dir}/graphics/qnxgraphicswindow.h
        ${dir}/graphics/qnxgraphicswindow.cpp
    )
    list(APPEND platform_libraries -lscreen)
endif()

# nougat==nougat-mr2.3-release
set(OS_ANDROID_VERSION "nougat" CACHE STRING "Currently supported: nougat,4.4.4_r1")
if (ANDROID)
    list(APPEND sources
        ${dir}/graphics/anwgraphicswindow.h
        ${dir}/graphics/anwgraphicswindow.cpp
    )
    include_directories(
        ${dir}/android/aosp/${OS_ANDROID_VERSION}/include
        ${dir}/android/aosp/${OS_ANDROID_VERSION}/include/hardware/libhardware/include
        ${dir}/android/aosp/${OS_ANDROID_VERSION}/include/frameworks/native/include
        ${dir}/android/aosp/${OS_ANDROID_VERSION}/include/system/core/include
    )

    list(APPEND platform_libraries -Wl,--hash-style=both -L${dir}/android/aosp/${OS_ANDROID_VERSION}/lib/${ANDROID_NDK_ABI_NAME} -lgui -lutils)
    set_property(SOURCE ${dir}/graphics/windowfactory.cpp PROPERTY COMPILE_DEFINITIONS "ANDROID_ENABLE_ANW=1")
    message("ANW is enabled. Uses private C++ API from AOSP, and might not be compatible with all devices.")

    # create postion independent code (required Android-L, doesn't work on pre Android-16)
    # https://code.google.com/p/android-developer-preview/issues/detail?id=888
    # http://vinsol.com/blog/2014/08/19/compiling-native-libraries-for-android-l/
    list(APPEND platform_libraries android -pie -fPIE)
endif()
if(TARGET glfw AND NOT ("$ENV{USE_WAYLAND}" STREQUAL "1"))

find_library(GLAPI glapi)
if(GLAPI)
    list(APPEND TESTFW_LIBRARIES ${GLAPI})
endif()

    list(APPEND sources
        ${dir}/graphics/glfwgraphicscontext.h
        ${dir}/graphics/glfwgraphicscontext.cpp
        ${dir}/graphics/glfwgraphicswindow.h
        ${dir}/graphics/glfwgraphicswindow.cpp
    )
else()
    find_package(OGLX)
    if(OGLX_FOUND)
        list(APPEND platform_libraries "${OGLX_LIBRARIES}")
    endif()
endif()

include_directories(${dir})
if(NOT EXECUTABLE_NAME)
    set(EXECUTABLE_NAME testfw_app)
endif()

if(NO_GL)
    set(main_cpp main_windowless.cpp)
else()
    set(main_cpp main.cpp)
endif()

source_group( "Source Files" FILES ${sources} )

# libdrm is provided by screen
if(NOT QNX)
find_package(DRM QUIET)
endif()

add_executable(${EXECUTABLE_NAME}
    ${main_cpp}
    ${sources}
)
set_target_properties(${EXECUTABLE_NAME} PROPERTIES ENABLE_EXPORTS ON)

if("$ENV{USE_WAYLAND}" STREQUAL "1" OR "$ENV{USE_WAYLAND}" STREQUAL "true")

		include(CheckIncludeFiles)
		include(CheckFunctionExists)

		find_program(WAYLAND_SCANNER_EXECUTABLE NAMES wayland-scanner)

		include(FindPkgConfig)
		pkg_check_modules(WAYLAND_PROTOCOLS REQUIRED wayland-protocols>=1.15)
		pkg_get_variable(WAYLAND_PROTOCOLS_BASE wayland-protocols pkgdatadir)
		pkg_get_variable(WAYLAND_CLIENT_PKGDATADIR wayland-client pkgdatadir)

		macro(wayland_generate protocol_file output_file)
			add_custom_command(OUTPUT "${output_file}.h"
				COMMAND "${WAYLAND_SCANNER_EXECUTABLE}" client-header "${protocol_file}" "${output_file}.h"
				DEPENDS "${protocol_file}"
				VERBATIM)

			add_custom_command(OUTPUT "${output_file}.c"
				COMMAND "${WAYLAND_SCANNER_EXECUTABLE}" private-code "${protocol_file}" "${output_file}.c"
				DEPENDS "${protocol_file}"
				VERBATIM)

			target_sources(${EXECUTABLE_NAME} PRIVATE "${output_file}.h" "${output_file}.c")
		endmacro()

		wayland_generate(
			"${WAYLAND_PROTOCOLS_BASE}/stable/xdg-shell/xdg-shell.xml"
			"${CMAKE_BINARY_DIR}/xdg-shell-client-protocol")


target_include_directories(${EXECUTABLE_NAME} PRIVATE ${CMAKE_BINARY_DIR})

endif()

if (TARGET glfw)
    target_link_libraries(${EXECUTABLE_NAME} glfw)
    target_compile_definitions(${EXECUTABLE_NAME} INTERFACE HAVE_GLFW3=1)
endif()

if(MSVC)
    target_link_libraries(${EXECUTABLE_NAME} "delayimp")
    target_link_libraries(${EXECUTABLE_NAME} "advapi32.lib;gdi32.lib;shell32.lib")
endif()

install(TARGETS ${EXECUTABLE_NAME} DESTINATION bin)
if(APPLE AND NOT IOS)
    if(OPT_FIXUP_BUNDLE)
        # set target MACOSX_BUNDLE because fixup_bundle doesn't work for standalone executable on the mac
        set_target_properties(${EXECUTABLE_NAME} PROPERTIES MACOSX_BUNDLE 1)
    endif()

    if (NOT XCODE_VERSION VERSION_LESS 7.0)
        set_target_properties(${EXECUTABLE_NAME} PROPERTIES XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC "YES")
        target_link_libraries(${EXECUTABLE_NAME} "-weak_framework QuartzCore")
        target_link_libraries(${EXECUTABLE_NAME} "-weak_framework Metal")
    endif()
endif()

if((ANDROID OR UNIX) AND NOT QNX)
   list(APPEND platform_libraries -ldl)
endif()

if (MSVC)
    set_target_properties(${EXECUTABLE_NAME} PROPERTIES LINK_FLAGS "${TESTFW_MSVC_INCLUDE_SYMBOLS}")
endif()
if (MSVC10)
    target_link_libraries(${EXECUTABLE_NAME} Psapi)
endif()

target_link_libraries(${EXECUTABLE_NAME}
    tfw_schemas
    tfw_deviceinfo
    ${TESTFW_LIBRARIES}
    ${APPLY_FIND_LIBRARIES}
    ${PNG_LIBRARIES}
    ${platform_libraries}
    ${APPLY_FIND_LIBRARIES}
    ${PNG_LIBRARIES}
    ${ZLIB_LIBRARIES}
)
if(DRM_FOUND AND NOT QNX)
	target_link_libraries(${EXECUTABLE_NAME} ${DRM_LIBRARY} )
endif()

foreach(module ${TESTFW_MODULES})
    add_dependencies(${EXECUTABLE_NAME} ${module})
endforeach()

if (ANDROID)
    find_library(epoxy_LIB libepoxy.so)
    find_library(epoxy_LIBD libepoxy_d.so)
    if (epoxy_LIB)
        install(FILES ${epoxy_LIB} DESTINATION ${TFW_PACKAGE_DIR} CONFIGURATIONS Release RelWithDebInfo MinSizeRel)
    endif()
    if (epoxy_LIBD)
        install(FILES ${epoxy_LIBD} DESTINATION ${TFW_PACKAGE_DIR} CONFIGURATIONS Debug)
    endif()
else()
    set(TFW_DEVELOPMENT_DIR ${CMAKE_BINARY_DIR}/tfw-dev)
    # Do not create subdirectories for every build type
    set_target_properties(${EXECUTABLE_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_DEBUG ${TFW_DEVELOPMENT_DIR}
        RUNTIME_OUTPUT_DIRECTORY_RELEASE ${TFW_DEVELOPMENT_DIR}
        RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${TFW_DEVELOPMENT_DIR}
        RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${TFW_DEVELOPMENT_DIR})

    if (OPT_FIXUP_BUNDLE AND NOT ($ENV{NG_TARGET_PLATFORM} MATCHES "linux_arm"))
        set(DIRS ${CMAKE_INSTALL_PREFIX}/lib)
        foreach(dir ${CMAKE_PREFIX_PATH})
            list(APPEND DIRS ${dir}/lib)
        endforeach()
        if(NOT APPLE)
            set_target_properties(${EXECUTABLE_NAME} PROPERTIES
                INSTALL_RPATH "$ORIGIN"
                SKIP_BUILD_RPATH false
                BUILD_WITH_INSTALL_RPATH true)
        endif()
        add_custom_target(fixup ALL DEPENDS ${EXECUTABLE_NAME})
        add_custom_command(TARGET fixup POST_BUILD
            COMMAND ${CMAKE_COMMAND} "\"-DSRC=$<TARGET_FILE:${EXECUTABLE_NAME}>\"" "\"-DDST=${TFW_PACKAGE_DIR}/bin\"" "\"-DDIRS=${DIRS}\"" "-DOPT_CONFIG=${OPT_CONFIG}" -P ${CMAKE_CURRENT_LIST_DIR}/../../../cmake-utils/cmake/fixup.cmake
            COMMAND ${CMAKE_COMMAND} -E copy_directory ${TFW_PACKAGE_DIR}/bin ${TFW_DEVELOPMENT_DIR})
    endif()
    if($ENV{NG_TARGET_PLATFORM} MATCHES "linux_arm" AND ${EXECUTABLE_NAME} MATCHES "Compu|ADAS")
    set_target_properties(${EXECUTABLE_NAME} PROPERTIES
                INSTALL_RPATH "$ENV{NG_INSTALL_ROOT}/$ENV{NG_TARGET_PLATFORM}/lib"
                SKIP_BUILD_RPATH false
                BUILD_WITH_INSTALL_RPATH true)
        add_custom_command(TARGET ${EXECUTABLE_NAME} POST_BUILD
            COMMAND mkdir ARGS -p ${TFW_PACKAGE_DIR}/bin
            COMMAND cp ARGS -RL ${TFW_DEVELOPMENT_DIR}/${EXECUTABLE_NAME} ${TFW_PACKAGE_DIR}/bin
            COMMAND cp ARGS -RL $ENV{NG_INSTALL_ROOT}/$ENV{NG_TARGET_PLATFORM}/lib/libngrtl*.so ${TFW_PACKAGE_DIR}/bin)
    endif()
endif()


if(NOT "$ENV{ARCHIVE_ROOT}" STREQUAL "" AND NOT ANDROID)

    # Installation

    set(CPACK_PACKAGE_FILE_NAME "${PRODUCT_ID}-$ENV{NG_TARGET_PLATFORM}-${PRODUCT_VERSION}+corporate")

    if(APPLE)
        set(APP_BUNDLE ${EXECUTABLE_NAME}.app)
        set(APP_CONTENTS_DIR ${APP_BUNDLE}/Contents)
        install(DIRECTORY ${TFW_PACKAGE_DIR}/config DESTINATION ${APP_CONTENTS_DIR}/Resources/)
        install(DIRECTORY ${TFW_PACKAGE_DIR}/data DESTINATION ${APP_CONTENTS_DIR}/Resources/)
        install(DIRECTORY ${TFW_PACKAGE_DIR}/plugins DESTINATION ${APP_CONTENTS_DIR}/Resources/)
        install(TARGETS ${EXECUTABLE_NAME} BUNDLE DESTINATION .)
        set(APPS \${CMAKE_INSTALL_PREFIX}/${APP_BUNDLE})
    else()
        install(DIRECTORY ${TFW_PACKAGE_DIR}/bin DESTINATION "." USE_SOURCE_PERMISSIONS)
        install(DIRECTORY ${TFW_PACKAGE_DIR}/config DESTINATION ".")
        install(DIRECTORY ${TFW_PACKAGE_DIR}/data DESTINATION ".")
        install(DIRECTORY ${TFW_PACKAGE_DIR}/plugins DESTINATION ".")
    endif()

    # Packaging


    set(CPACK_PACKAGE_VENDOR "Kishonti Ltd.")
    set(CPACK_PACKAGE_NAME "${PRODUCT_NAME}")
    set(CPACK_PACKAGE_VERSION_MAJOR "${BENCHMARK_VERSION_MAJOR}")
    set(CPACK_PACKAGE_VERSION_MINOR "${BENCHMARK_VERSION_MINOR}")
    set(CPACK_PACKAGE_VERSION_PATCH "${BENCHMARK_VERSION_PATCH}")
    set(CPACK_PACKAGE_VERSION "${PRODUCT_VERSION}")
    set(CPACK_OUTPUT_FILE_PREFIX "$ENV{ARCHIVE_ROOT}")


    if(WIN32)
        set(CPACK_GENERATOR ZIP)
        include(CPack)
    else()
        set(CPACK_GENERATOR TGZ)
        include(CPack)
    endif()
else()
    install(CODE "message(\"empty install\")")
endif()
