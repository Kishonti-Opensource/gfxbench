cmake_minimum_required(VERSION 3.5)

project(tfw_deviceinfo)

unset(NO_GL CACHE)
if($ENV{NO_GL} AND $ENV{NO_GL} EQUAL 1)
    set(NO_GL YES)
endif()

include(compiler_flags)
include(apply_find_vars)
include(EmbedFiles)


find_package(Poco REQUIRED COMPONENTS Foundation)
find_package(ngrtl REQUIRED core pngio)
apply_find_vars(POCO NGRTL)

include_directories(${CMAKE_CURRENT_LIST_DIR}/..)

embed_files(shaderprecision_shaders shaderprecision.vs shaderprecision.fs)
list(APPEND HEADERS
    chartcollector.h
    runtimeinfo.h
    nullruntimeinfo.h
    platformruntimeinfo.h
    clinfo.h
    cudainfo.h
    glinfo.h
    metalinfo.h
    ${CMAKE_CURRENT_BINARY_DIR}/shaderprecision_shaders.h
    semver.h
    jsonvisitor.h
)


# TODO: use graphics context creation from library (create: tfw_graphics lib)
# TODO: remove schemas dependency (ApiDefinitions)
list(APPEND SOURCE
    chartcollector.cpp
    nullruntimeinfo.cpp
    clinfo.cpp
    glinfo.cpp
    metalinfocollector.cpp
    semver.cpp
    ../schemas/apidefinition.cpp
)



find_package(epoxy)
if(EPOXY_FOUND)
    apply_find_vars(epoxy)
    list(APPEND HEADERS
        shaderprecision.h
    )
    list(APPEND SOURCE
        epoxyglinfocollector.cpp
        shaderprecision.cpp
        shaderprecision.vs
        shaderprecision.fs
        ../graphics/eglgraphicscontext.cpp
        ${CMAKE_CURRENT_BINARY_DIR}/shaderprecision_shaders.cpp
    )
else()
    list(APPEND SOURCE nullglinfocollector.cpp)
endif()

if(NOT NO_GL)
    find_package(glfw3 CONFIG)
endif()

if (TARGET glfw)
    set_source_files_properties(epoxyglinfocollector.cpp PROPERTIES COMPILE_DEFINITIONS HAVE_GLFW3)
    list(APPEND SOURCE
        ../graphics/glfwgraphicscontext.cpp
        ../graphics/glfwgraphicswindow.cpp
    )
endif()

find_package(clew)
if(clew_FOUND)
    list(APPEND SOURCE clinfocollector.cpp)
    apply_find_vars(CLEW)
endif()

find_package(cudaw)
if(cudaw_FOUND)
    list(APPEND SOURCE cudainfocollector.cpp)
    apply_find_vars(CUDAW)
endif()

if(WIN32)
    list(APPEND HEADERS windowsruntimeinfo.h)
    list(APPEND SOURCE windowsruntimeinfo.cpp)
elseif(ANDROID)
    list(APPEND HEADERS linuxruntimeinfo.h)
    list(APPEND SOURCE linuxruntimeinfo.cpp)
elseif(UNIX AND NOT APPLE AND NOT IOS)
    list(APPEND HEADERS linuxruntimeinfo.h)
    list(APPEND SOURCE linuxruntimeinfo.cpp)
elseif(APPLE AND NOT IOS)
    list(APPEND HEADERS osxruntimeinfo.h)
    list(APPEND SOURCE osxruntimeinfo.mm)
elseif(IOS)
    list(APPEND HEADERS iosruntimeinfo.h)
    list(APPEND SOURCE iosruntimeinfo.mm)
endif()


if(IOS)
    set_source_files_properties(epoxyglinfocollector.cpp PROPERTIES COMPILE_FLAGS "-x objective-c++")
    set_source_files_properties(metalinfocollector.cpp PROPERTIES COMPILE_FLAGS "-x objective-c++")
endif()

add_library(tfw_deviceinfo STATIC ${HEADERS} ${SOURCE})

if (TARGET glfw)
    target_link_libraries(tfw_deviceinfo glfw)
    target_compile_definitions(tfw_deviceinfo INTERFACE HAVE_GLFW3=1)
endif()

if(NOT WIN32)
    target_compile_options(tfw_deviceinfo PRIVATE -Wno-deprecated-declarations)
endif()

target_link_libraries(tfw_deviceinfo ${APPLY_FIND_LIBRARIES})

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	target_link_libraries(tfw_deviceinfo dl)
endif()

if(EPOXY_FOUND)
    target_compile_definitions(tfw_deviceinfo PRIVATE -DHAVE_EPOXY)
endif()

target_include_directories(tfw_deviceinfo PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

if(WIN32)
    target_link_libraries(tfw_deviceinfo PowrProf.lib)
endif()

if (OPT_ENABLE_TESTS)
    add_executable(deviceinfo
        main.cpp
    )
    if (ANDROID)
        target_link_libraries(deviceinfo -pie -fPIE)
    endif()
    target_link_libraries(deviceinfo tfw_deviceinfo)
    install(TARGETS deviceinfo DESTINATION bin)
endif()

#set_target_properties(tfw_deviceinfo PROPERTIES XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET "8.0")
