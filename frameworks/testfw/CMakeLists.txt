cmake_minimum_required(VERSION 3.5)

project(testfw)

include(testfw.cmake)

# Optional: if set add each directory to the generated project.
# Useful when developing the module, so application and module get to a single solutions (MSVC, Xcode, Eclipse, ...)
# Each module should call register_testfw_module(...) to static module lib will be linked into the app (see: testfw.cmake)
# message(STATUS "BENCHMARK_DIR: ${BENCHMARK_DIR}")
foreach (module_dir ${BENCHMARK_DIR})
    get_filename_component(last_dir_component ${module_dir} NAME)
    add_subdirectory(${module_dir} ${last_dir_component})
endforeach()

if(UNIX AND ( ${CMAKE_CXX_COMPILER_ID} MATCHES "GNU" OR ${CMAKE_CXX_COMPILER_ID} MATCHES "Clang" ) )
    set(CMAKE_CXX_FLAGS "-fPIC ${CMAKE_CXX_FLAGS}")
    set(CMAKE_C_FLAGS "-fPIC ${CMAKE_C_FLAGS}")
endif()

include(compiler_flags)

include(apply_find_vars)
find_package(ngrtl REQUIRED core)
add_definitions(-DNGLOG_ENABLE=NGLOG_SEVERITY_ANY)
apply_find_vars(NGRTL)

set(SOURCES
    testfw.h
    testfw.cpp
    messagequeue.h
    tfwmessagequeue.h
)
find_package(EGL)
if(EGL_FOUND)
    list(APPEND SOURCES graphics/eglgraphicscontext.cpp graphics/eglgraphicscontext.h)
endif()

if(NOT OPT_SWIG_CSHARP)
	list(APPEND SOURCES
		graphics/vulkangraphicscontext.cpp
		graphics/AndroidVulkanGraphicsContext.cpp
		graphics/ovrgraphicscontext.cpp
	)
endif()


#if(BUILD_OVR_APP)
	if(WITH_OVR_SDK)
		list(APPEND SOURCES graphics/ovrgraphicscontext.h)
	else()
		list(APPEND SOURCES graphics/ovrgraphicscontext_dummy.h)
	endif()
#endif()

if (NOT TARGET tfw_schemas)
    add_subdirectory(schemas)
endif()

if (NOT TARGET tfw_deviceinfo AND NOT ${CMAKE_SYSTEM_NAME} STREQUAL "WindowsPhone" AND NOT ${CMAKE_SYSTEM_NAME} STREQUAL "WindowsStore")
    add_subdirectory(deviceinfo)
endif()

if (NOT TARGET tfw_resourcemanager AND NOT ${CMAKE_SYSTEM_NAME} STREQUAL "WindowsPhone" AND NOT ${CMAKE_SYSTEM_NAME} STREQUAL "WindowsStore")
    add_subdirectory(resourcemanager)
endif()

if(OPT_SWIG_JAVA)
    set(SWIG_MODULE_testfw_jni_EXTRA_DEPS
        testfw.h
        graphics/graphicscontext.h
        graphics/vulkangraphicscontext.h
        graphics/AndroidVulkanGraphicsContext.h
        graphics/eglgraphicscontext.h
        graphics/ovrgraphicscontext.h
        messagequeue.h
        tfwmessagequeue.h
        videostream.h

        deviceinfo/runtimeinfo.h
        deviceinfo/linuxruntimeinfo.h
        deviceinfo/clinfo.h
        deviceinfo/glinfo.h
        deviceinfo/cudainfo.h

        schemas/descriptors.h
        schemas/compareitem.h
        schemas/compareresult.h
        schemas/configuration.h
        schemas/resultitem.h
        schemas/serializable.h
        schemas/session.h
        schemas/testinfo.h
        schemas/testitem.h
        schemas/testrepository.h
        schemas/chart.h
        schemas/computeresult.h
        schemas/gfxresult.h
        schemas/result.h
        schemas/graphics.h
        schemas/compute.h
        schemas/environment.h
    )
    list(APPEND SOURCES signal_handler.cpp signal_handler.h)

    if (ANDROID)
        if(COMMAND find_host_package)
            find_host_package(SWIG REQUIRED)
        else()
            find_package(SWIG REQUIRED)
        endif()
        set(CMAKE_SWIG_OUTDIR ${CMAKE_CURRENT_LIST_DIR}/android/testfw-lib/src/main/java/net/kishonti/swig)
    else()
        set(CMAKE_SWIG_OUTDIR ${CMAKE_CURRENT_LIST_DIR}/java/net/kishonti/swig)
        find_package(SWIG REQUIRED)
        find_package(JNI REQUIRED)
        include_directories(${JNI_INCLUDE_DIRS})
    endif()
    set(CMAKE_SWIG_FLAGS -package net.kishonti.swig)
    if (EGL_FOUND)
        if(NOT OPT_SWIG_CSHARP)
            list(APPEND CMAKE_SWIG_FLAGS -DHAVE_EGL)
        endif()
    endif()

	if(${WITH_OVR_SDK})
		list(APPEND CMAKE_SWIG_FLAGS -DWITH_OVR_SDK)
	endif()

	if(OVR_BENCHMARK_MODE)
		list(APPEND CMAKE_SWIG_FLAGS -DOVR_BENCHMARK_MODE)
	endif()

	if(BUILD_OVR_APP)
		list(APPEND CMAKE_SWIG_FLAGS -DBUILD_OVR_APP)
	endif()

	include(${SWIG_USE_FILE})
	set_source_files_properties(testfw.i PROPERTIES CPLUSPLUS ON)
	swig_add_module(testfw_jni java testfw.i ${SOURCES})
	target_include_directories(testfw_jni PUBLIC ${CMAKE_CURRENT_LIST_DIR} ${NGRTL_INCLUDE_DIRS} ${EGL_INCLUDE_DIRS})
	set_target_properties(testfw_jni PROPERTIES NO_SONAME OFF)

    if (ANDROID AND WITH_OVR_SDK)
        list(APPEND TESTFW_LIBRARIES ${CMAKE_CURRENT_LIST_DIR}/ovr/Libs/Android/armeabi-v7a/libvrapi.so)
    endif()

    swig_link_libraries(testfw_jni ${TESTFW_LIBRARIES})
    if ("${CXX_COMPILER_ID_UPPER}" STREQUAL "GNU")
        set_source_files_properties(${swig_generated_file_fullname} PROPERTIES COMPILE_FLAGS -fno-strict-aliasing)
    endif()
    if (ANDROID)
        add_definitions(-DHAVE_GLES -DHAVE_GLES2)
		if(WITH_OVR_SDK)
			add_definitions(-DWITH_OVR_SDK)
		endif()

		if(OVR_BENCHMARK_MODE)
			add_definitions(-DOVR_BENCHMARK_MODE)
		endif()

		if(BUILD_OVR_APP)
			add_definitions(-DBUILD_OVR_APP)
		endif()
        #swig_link_libraries(testfw_jni ${APPLY_FIND_LIBRARIES} tfw_schemas tfw_deviceinfo android ${CMAKE_CURRENT_LIST_DIR}/libNvidia_gfx_debugger_stub.a)
		swig_link_libraries(testfw_jni ${APPLY_FIND_LIBRARIES} tfw_schemas tfw_deviceinfo android GLESv2 EGL)
        find_file(GDBSERVER gdbserver PATHS
            ${ANDROID_TOOLCHAIN_ROOT}/..
            ${ANDROID_NDK}/prebuilt/android-arm/gdbserver
            ${ANDROID_NDK}/prebuilt/android-arm64/gdbserver
            NO_CMAKE_FIND_ROOT_PATH)

        string(REPLACE "-clang" "" ANDROID_TOOLCHAIN_NAME_WO_CLANG ${ANDROID_TOOLCHAIN_NAME})
        if(${ANDROID_NDK} MATCHES "28\\.0")
            file(COPY ${ANDROID_NDK}/toolchains/llvm/prebuilt/${ANDROID_HOST_TAG}/sysroot/usr/lib/${ANDROID_TOOLCHAIN_NAME_WO_CLANG}/libc++_shared.so DESTINATION ${LIBRARY_OUTPUT_PATH_ROOT}/libs/${ANDROID_NDK_ABI_NAME})
        else()
            file(COPY ${ANDROID_NDK}/sources/cxx-stl/llvm-libc++/libs/${ANDROID_ABI}/libc++_shared.so DESTINATION ${LIBRARY_OUTPUT_PATH_ROOT}/libs/${ANDROID_NDK_ABI_NAME})
        endif()

        #configure_file (${CMAKE_CURRENT_LIST_DIR}/support/Application.mk.in ${LIBRARY_OUTPUT_PATH_ROOT}/jni/Application.mk)
        #configure_file (${CMAKE_CURRENT_LIST_DIR}/support/Android.mk.in ${LIBRARY_OUTPUT_PATH_ROOT}/jni/Android.mk)
        #if(GDBSERVER)
            #configure_file (${CMAKE_CURRENT_LIST_DIR}/support/gdb.setup.in ${LIBRARY_OUTPUT_PATH_ROOT}/libs/${ANDROID_NDK_ABI_NAME}/gdb.setup)
            #configure_file (${GDBSERVER} ${LIBRARY_OUTPUT_PATH_ROOT}/libs/${ANDROID_NDK_ABI_NAME} COPYONLY)
        #endif()
        add_custom_command(TARGET testfw_jni POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:testfw_jni> ${LIBRARY_OUTPUT_PATH_ROOT}/libs/${ANDROID_NDK_ABI_NAME}/$<TARGET_FILE_NAME:testfw_jni>
            COMMAND ${CMAKE_STRIP} --strip-unneeded $<TARGET_FILE:testfw_jni>)
        install(TARGETS testfw_jni DESTINATION ${LIBRARY_OUTPUT_PATH_ROOT}/libs/${ANDROID_NDK_ABI_NAME})

        if(WITH_OVR_SDK)
            add_custom_command(TARGET testfw_jni POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_LIST_DIR}/ovr/Libs/Android/armeabi-v7a/libvrapi.so ${LIBRARY_OUTPUT_PATH_ROOT}/libs/${ANDROID_NDK_ABI_NAME}/libvrapi.so)
            add_custom_command(TARGET testfw_jni POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_LIST_DIR}/ovr/Libs/Android/VrApi.jar ${LIBRARY_OUTPUT_PATH_ROOT}/libs/VrApi.jar)
            install(FILES ${CMAKE_CURRENT_LIST_DIR}/ovr/Libs/Android/armeabi-v7a/libvrapi.so DESTINATION libs/${ANDROID_NDK_ABI_NAME})
        endif()

        install(TARGETS testfw_jni DESTINATION libs/${ANDROID_NDK_ABI_NAME})
    endif()
elseif(OPT_SWIG_CSHARP)
    find_package(SWIG REQUIRED)
    include(${SWIG_USE_FILE})
    set(CMAKE_SWIG_OUTDIR ${CMAKE_BINARY_DIR}/../src-gen/TestFw)
    set(CMAKE_SWIG_FLAGS -namespace TestFw)
    set_source_files_properties(testfw.i PROPERTIES CPLUSPLUS ON)
    swig_add_module(testfw_cs csharp testfw.i ${SOURCES})
    target_include_directories(testfw_cs PRIVATE ${CMAKE_CURRENT_LIST_DIR} ${NGRTL_INCLUDE_DIRS})
    set_target_properties(testfw_cs PROPERTIES DEBUG_POSTFIX "")
    swig_link_libraries(testfw_cs ${TESTFW_LIBRARIES} tfw_schemas ${NGRTL_LIBRARIES})
endif()

# add "developer" applications
if (NOT (IOS OR OPT_SWIG_CSHARP))
    add_subdirectory(hostapp/basic)
endif()
if (IOS)
    add_subdirectory(ios/testfw-app)
endif()
if (ANDROID AND OPT_SWIG_JAVA AND "${APPLICATION_TYPE}" STREQUAL "developer")
    if(${WITH_OVR_SDK})
        add_subdirectory(android/ovr-testfw-app)
    else()
        add_subdirectory(android/testfw-app)
    endif()
endif()
if(${CMAKE_SYSTEM_NAME} STREQUAL "WindowsPhone" AND "${APPLICATION_TYPE}" STREQUAL "developer")
    add_subdirectory(wp/testfw-app)
endif()
