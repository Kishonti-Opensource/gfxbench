cmake_minimum_required(VERSION 3.5)
project(systeminfo)

include(compiler_flags)
include(apply_find_vars)
include(EmbedFiles)

if("$ENV{NG_TARGET_PLATFORM}" STREQUAL "v120_wp81-arm")
	set(WP81 1)
elseif("$ENV{NG_TARGET_PLATFORM}" STREQUAL "v120_wp81-win32")
	set(WP81 1)
elseif("$ENV{NG_TARGET_PLATFORM}" STREQUAL "v140_wp81-arm")
	set(WP81 1)
elseif("$ENV{NG_TARGET_PLATFORM}" STREQUAL "v140_wp81-win32")
	set(WP81 1)
elseif("$ENV{NG_TARGET_PLATFORM}" STREQUAL "v120_rt81-arm")
	set(RT 1)
elseif("$ENV{NG_TARGET_PLATFORM}" STREQUAL "v120_rt81-x86")
	set(RT 1)
elseif("$ENV{NG_TARGET_PLATFORM}" STREQUAL "v120_rt81-x64")
	set(RT 1)
endif()

find_package(Poco REQUIRED COMPONENTS Foundation)
find_package(ngrtl REQUIRED core pngio)
apply_find_vars(POCO NGRTL)

set(ENABLE_FIND_VULKAN 1 CACHE STRING "help desc" FORCE)

if(NOT OPT_SWIG_CSHARP)
    if(ENABLE_FIND_VULKAN)
        find_package(Vulkan)
    endif()
endif()

if(VULKAN_FOUND)
	if(ANDROID)
		list(APPEND SOURCE
			src/vulkan_wrapper.cpp
			src/vulkan_wrapper.h)

		set(VULKAN_LIBRARIES "") # remove vulkan.so because we dynamic loading the fn pointers
	endif()

    apply_find_vars(VULKAN)
else()
	message("Vulkan not found, uses nullvulkaninfocollector!!!")
endif()


list(APPEND PUBLIC_HEADERS
    src/properties.h
    src/utils.h
    src/dataformatter.h
    src/FormattedDeviceInfo.h
    src/SystemInfoCommonKeys.h

    src/deviceinfo.h
    src/systeminfo.h
    src/keyvaluevisitor.h
)

list(APPEND HEADERS
    src/semver.h
    src/jsonvisitor.h
)

list(APPEND SOURCE
    src/properties.cpp
    src/utils.cpp
    src/dataformatter.cpp
    src/FormattedDeviceInfo.cpp
    src/semver.cpp
    ../testfw/schemas/apidefinition.cpp
)

if(NOT OPT_SWIG_CSHARP)
    list(APPEND PUBLIC_HEADERS

        src/glinfo.h
        src/glinfocollector.h

        src/metalinfo.h
        src/metalinfocollector.h

        src/vulkaninfo.h
        src/vulkaninfocollector.h

        src/deviceinfocollector.h
        src/nulldeviceinfocollector.h
    )

if(NOT QNX)
    list(APPEND SOURCE
        src/epoxyglinfocollector.cpp
        src/metalinfocollector.cpp
)
endif()

list(APPEND SOURCE
        ../testfw/graphics/eglgraphicscontext.cpp
    )
endif()

if(VULKAN_FOUND)
    list(APPEND SOURCE
        src/vulkaninfocollector.cpp
    )
else()
    list(APPEND SOURCE
        src/nullvulkaninfocollector.cpp
    )
endif()

find_package(epoxy)
if(EPOXY_FOUND)
    apply_find_vars(epoxy)
endif()

find_package(glfw3 QUIET CONFIG)
if (TARGET glfw)
    set_source_files_properties(src/epoxyglinfocollector.cpp PROPERTIES COMPILE_DEFINITIONS HAVE_GLFW3)
    list(APPEND SOURCE
        ../testfw/graphics/glfwgraphicscontext.cpp
        ../testfw/graphics/glfwgraphicswindow.cpp
    )
endif()

find_package(clew QUIET)
if(clew_FOUND)
    apply_find_vars(CLEW)
    list(APPEND PUBLIC_HEADERS
      src/clinfo.h
      src/clinfocollector.h
    )
    list(APPEND SOURCE
      src/clinfocollector.cpp
    )
endif()

find_package(cudaw QUIET)
if(cudaw_FOUND)
    apply_find_vars(CUDAW)
    list(APPEND PUBLIC_HEADERS
      src/cudainfo.h
      src/cudainfocollector.h
    )
    list(APPEND SOURCE
      src/cudainfocollector.cpp
    )
endif()

### Setting platform dependent sources ###
if(WIN32)
    if(NOT OPT_SWIG_CSHARP)
        list(APPEND PUBLIC_HEADERS src/directxinfo.h)
        list(APPEND PUBLIC_HEADERS src/windowsdeviceinfocollector.h src/directxinfocollector.h)
        list(APPEND SOURCE src/windowsdeviceinfocollector.cpp src/directxinfocollector.cpp)
    endif()
elseif(IOS)
    list(APPEND PUBLIC_HEADERS src/iosdeviceinfocollector.h)
    list(APPEND SOURCE src/iosdeviceinfocollector.mm)
    set_source_files_properties(src/epoxyglinfocollector.cpp PROPERTIES COMPILE_FLAGS "-x objective-c++")
    set_source_files_properties(src/metalinfocollector.cpp PROPERTIES COMPILE_FLAGS "-x objective-c++")
elseif(APPLE)
    list(APPEND PUBLIC_HEADERS src/osxdeviceinfocollector.h)
    list(APPEND SOURCE src/osxdeviceinfocollector.mm)
    set_source_files_properties(src/epoxyglinfocollector.cpp PROPERTIES COMPILE_FLAGS "-x objective-c++")
    set_source_files_properties(src/metalinfocollector.cpp PROPERTIES COMPILE_FLAGS "-x objective-c++")
elseif(UNIX AND NOT ANDROID AND NOT QNX)
    list(APPEND PUBLIC_HEADERS src/linuxdeviceinfocollector.h)
    list(APPEND SOURCE src/linuxdeviceinfocollector.cpp)
elseif(WP81 OR RT)
    #replace to dummy cpp#
    list(APPEND SOURCE src/datacollector_null.cpp)
endif()

list(APPEND HEADERS ${PUBLIC_HEADERS})

### Getting external packages ###

if(WINDOWS OR WINDOWS64 OR MACOSX)
    find_package(GLEW)
    if(GLEW_FOUND)
        add_definitions(-DHAVE_GLEW)
        include_directories(${GLEW_INCLUDE_DIRS})
        add_definitions(${GLEW_DEFINITIONS})
    endif()

    if(WIN32)
        include_directories(${CMAKE_CURRENT_LIST_DIR}/windows/nvapi)
        include_directories(${CMAKE_CURRENT_LIST_DIR}/windows/ags/inc)
    endif()

    if(APPLE AND NOT IOS)
        include_directories(${POCO_INCLUDE_DIRS})
    endif()
endif()

### Defining the target ##

if(NOT ANDROID AND NOT WP81 AND NOT RT)
    set(CMAKE_DEBUG_POSTFIX _d)
endif()

add_library(systeminfo STATIC ${HEADERS} ${SOURCE})
#add_executable(systeminfo ${HEADERS} ${SOURCE} src/main.cpp)
if (TARGET glfw)
    target_link_libraries(systeminfo glfw)
    target_compile_definitions(systeminfo INTERFACE HAVE_GLFW3)
endif()

if(NOT WIN32)
    target_compile_options(systeminfo PRIVATE -Wno-deprecated-declarations)
endif()


if(EPOXY_FOUND)
    target_compile_definitions(systeminfo PRIVATE -DHAVE_EPOXY)
endif()
target_include_directories(systeminfo PRIVATE ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_LIST_DIR}/../testfw)

### Setting necessary libraries ##

set(LIBRARIES ${NGRTL_LIBRARIES})

if (MSVC )
    if( WP81 OR RT)
        add_definitions(-DDISABLE_COLLECTORS) #TODO add per api per collector
    else()
        list(APPEND LIBRARIES Psapi.lib)
    endif()
endif()

if(WIN32 AND NOT $ENV{NG_TARGET_PLATFORM} MATCHES "vs20[0-9][0-9]-arm64")
    include_directories(
        ${CMAKE_CURRENT_LIST_DIR}/windows/ags/inc
        ${CMAKE_CURRENT_LIST_DIR}/windows/nvapi
    )
    if(CMAKE_CL_64)
        list(APPEND LIBRARIES ${CMAKE_CURRENT_LIST_DIR}/windows/nvapi/amd64/nvapi64.lib)
        list(APPEND LIBRARIES ${CMAKE_CURRENT_LIST_DIR}/windows/ags/lib/amd_ags_x64.lib)
    endif()

elseif(MACOSX)
    list(APPEND LIBRARIES "-framework AppKit" ${GLEW_LIBRARIES} ${POCO_LIBRARIES})
elseif(IOS)
    list(APPEND LIBRARIES "-framework AppKit -framework AVFoundation -framework UIKit")
elseif(LINUX)
    list(APPEND LIBRARIES "-ldl")
endif()
list(APPEND LIBRARIES ${APPLY_FIND_LIBRARIES})
target_link_libraries(systeminfo ${LIBRARIES})



if (OPT_SWIG_JAVA)
    if (ANDROID)
        if(COMMAND find_host_package)
            find_host_package(SWIG REQUIRED)
        else()
            find_package(SWIG REQUIRED)
        endif()
        set(CMAKE_SWIG_OUTDIR ${CMAKE_CURRENT_LIST_DIR}/android/systeminfo-lib/src/main/java/net/kishonti/systeminfo/swig)
    else()
        set(CMAKE_SWIG_OUTDIR ${CMAKE_CURRENT_LIST_DIR}/java/net/kishonti/systeminfo/swig)
        find_package(SWIG REQUIRED)
        find_package(JNI REQUIRED)
        include_directories(${JNI_INCLUDE_DIRS})
    endif()

    include_directories(${CMAKE_CURRENT_LIST_DIR}/src)
    include(${SWIG_USE_FILE})
    set(CMAKE_SWIG_FLAGS -package net.kishonti.systeminfo.swig)
    set_source_files_properties(systeminfo.i PROPERTIES CPLUSPLUS ON)
    swig_add_module(systeminfo_jni java systeminfo.i)
    set_target_properties(systeminfo_jni PROPERTIES NO_SONAME OFF)
    swig_link_libraries(systeminfo_jni systeminfo)
    if (ANDROID)
        add_definitions(-DHAVE_GLES -DHAVE_GLES2)
        swig_link_libraries(systeminfo_jni ${APPLY_FIND_LIBRARIES} android GLESv2 EGL)
        add_custom_command(TARGET systeminfo_jni POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:systeminfo_jni> ${LIBRARY_OUTPUT_PATH_ROOT}/libs/${ANDROID_NDK_ABI_NAME}/$<TARGET_FILE_NAME:systeminfo_jni>
            COMMAND ${CMAKE_STRIP} --strip-unneeded $<TARGET_FILE:systeminfo_jni>)
        install(TARGETS systeminfo_jni DESTINATION ${LIBRARY_OUTPUT_PATH_ROOT}/libs/${ANDROID_NDK_ABI_NAME})

    endif()
endif()

if(OPT_SWIG_CSHARP)
   find_package(SWIG REQUIRED)
   include(${SWIG_USE_FILE})
   include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
   set(CMAKE_SWIG_OUTDIR ${CMAKE_BINARY_DIR}/../src-gen/SystemInfo)
   set(CMAKE_SWIG_FLAGS -namespace SystemInfo)
    if( WP81 OR RT)
        list(APPEND CMAKE_SWIG_FLAGS -DDISABLE_COLLECTORS)
    endif()

   set_source_files_properties(systeminfo.i PROPERTIES CPLUSPLUS ON)
   swig_add_module(systeminfo_cs csharp systeminfo.i ${SOURCES})
   swig_link_libraries(systeminfo_cs systeminfo)
endif()

### Install phase ###

install(FILES ${PUBLIC_HEADERS} DESTINATION "include/sysinf")
install(TARGETS systeminfo DESTINATION lib)
set(ARM64 0)
if($ENV{NG_TARGET_PLATFORM} MATCHES "vs20[0-9][0-9]-arm64")
    set(ARM64 1)
endif()

if(WIN32 AND NOT ARM64)
    if(CMAKE_CL_64)
        install(FILES ${CMAKE_CURRENT_LIST_DIR}/windows/nvapi/amd64/nvapi64.lib DESTINATION lib)
        install(FILES ${CMAKE_CURRENT_LIST_DIR}/windows/ags/lib/amd_ags_x64.lib DESTINATION lib)
        install(FILES ${CMAKE_CURRENT_LIST_DIR}/windows/ags/lib/amd_ags_x64.dll DESTINATION bin)
    endif()
endif()

configure_file(sysinfConfig.cmake.in ${CMAKE_BINARY_DIR}/sysinfConfig.cmake @ONLY)
install(FILES ${CMAKE_BINARY_DIR}/sysinfConfig.cmake DESTINATION share/sysinf/cmake)
