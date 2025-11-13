cmake_minimum_required(VERSION 2.8.12)

project(app_ios)

include(CheckRequiredVars)
include(VersionParse)
include(mklink)
include(printvar)

add_definitions(-DGLES_SILENCE_DEPRECATION=1)

## suppress deprecation error to warning with iOS 10.0 sdk (Xcode 8.0)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-error=deprecated-declarations")

check_required_vars(PRODUCT_ID PRODUCT_VERSION PRODUCT_NAME TFW_PACKAGE_DIR)

if (BENCHMARK_DIR)
    add_subdirectory(${BENCHMARK_DIR} ${PRODUCT_ID}_dir)
endif()

version_parse(BENCHMARK ${PRODUCT_VERSION})

find_host_program(PYTHON python REQUIRED)

string(REPLACE "_" ";" PRODUCT_FAMILY ${PRODUCT_ID})
list(GET PRODUCT_FAMILY 0 PRODUCT_FAMILY)
message(STATUS "PRODUCT_FAMILY: ${PRODUCT_FAMILY} (calculated from PRODUCT_ID)")

# set PACKAGE_NAME, and PRODUCT_TYPE
if (OPT_COMMUNITY_BUILD)
    set(PRODUCT_TYPE "community")
    set(PACKAGE_NAME "net.kishonti.${PRODUCT_ID}")
else()
    set(PRODUCT_TYPE "corporate")
    # for corporate build append VERSION_CODE so different versions can be installed side by side.
    set(PACKAGE_NAME "net.kishonti.${PRODUCT_ID}.v${VERSION_CODE}.corporate")
endif()
# fixup PACKAGE_NAME to be valid reverse-DNS id
string(REPLACE "_" "." PACKAGE_NAME ${PACKAGE_NAME})

# maintain backward compatibility here for specific benchmark versions that are already in the store
if ("${PRODUCT_ID}" STREQUAL "gfxbench_gl" AND OPT_COMMUNITY_BUILD AND "$ENV{DISTRIBUTION_TYPE}" STREQUAL "store")
    set(PACKAGE_NAME "com.glbenchmark.glb27")
endif()
message(STATUS "PACKAGE_NAME (iOS Bundle ID): ${PACKAGE_NAME}")

# BUNDLE_ID is used in iOS-Info.plist.in
set(BUNDLE_ID ${PACKAGE_NAME})

if ("${PRODUCT_ID}" STREQUAL "gfxbench_metal")
    set(METAL_REQUIREMENT "<string>metal</string>")
else()
    set(METAL_REQUIREMENT "")
endif()

if("${PRODUCT_FAMILY}" STREQUAL "compubench")
    file(GLOB PRODUCT_TESTLISTS ${TFW_PACKAGE_DIR}/benchmark_tests.json)
else()
    file(GLOB PRODUCT_TESTLISTS ${TFW_PACKAGE_DIR}/testlist*.json)
endif()
list(APPEND BENCHMARK_RESOURCES ${PRODUCT_TESTLISTS})

foreach(testlist ${PRODUCT_TESTLISTS})
    # checking test resources (images)
    execute_process(COMMAND ${PYTHON} ${CMAKE_CURRENT_LIST_DIR}/../frameworks/testfw/support/benchmark_tests_util.py --image_ids --file ${testlist}
        OUTPUT_VARIABLE image_ids OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_VARIABLE error
        RESULT_VARIABLE result
    )
    if (NOT "${result}" STREQUAL "0")
        message(FATAL_ERROR "${error}")
    endif()
endforeach()

execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/res/${PRODUCT_ID}/images/ ${CMAKE_BINARY_DIR}/${PRODUCT_ID}_images/)
execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/res/${PRODUCT_ID}-${BENCHMARK_VERSION_MAJOR}.${BENCHMARK_VERSION_MINOR}/images/ ${CMAKE_BINARY_DIR}/${PRODUCT_ID}_images/)

list(APPEND BENCHMARK_RESOURCES ${CMAKE_CURRENT_LIST_DIR}/AppIcon.xcassets)

file(GLOB PRODUCT_IMAGES ${CMAKE_BINARY_DIR}/${PRODUCT_ID}_images/*.png)
list(APPEND BENCHMARK_RESOURCES ${PRODUCT_IMAGES})
foreach(image_id ${image_ids})
    if(EXISTS ${TFW_PACKAGE_DIR}/images/${image_id}.png)
        list(APPEND BENCHMARK_RESOURCES ${TFW_PACKAGE_DIR}/images/${image_id}.png)
    else()
        message(FATAL_ERROR "Drawable resource not found: ${image_id}")
    endif()
endforeach()
list(APPEND BENCHMARK_RESOURCES ${TFW_PACKAGE_DIR}/config ${TFW_PACKAGE_DIR}/localization)
if (NOT COMMUNITY_BUILD AND BUNDLE_DATA)
    list(APPEND BENCHMARK_RESOURCES ${TFW_PACKAGE_DIR}/data)
endif()

set(THEME_NAME "Sky_Blue")
if("${PRODUCT_FAMILY}" STREQUAL "compubench")
    set(THEME_NAME "Grass_Green")
endif()
execute_process(COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_LIST_DIR}/res/themes/${THEME_NAME}.xml" "${CMAKE_BINARY_DIR}/theme/${THEME_NAME}.xml")
list(APPEND BENCHMARK_RESOURCES "${CMAKE_BINARY_DIR}/theme")


if (NOT OPT_COMMUNITY_BUILD)
    # add poco for command line options
    find_package(Poco REQUIRED COMPONENTS Util Foundation)
    add_definitions(${POCO_DEFINITIONS})

endif()


find_package(clew QUIET)
if(clew_FOUND)
    apply_find_vars(CLEW)
endif()

find_package(cudaw QUIET)
if(cudaw_FOUND)
    apply_find_vars(CUDAW)
endif()

find_package(epoxy QUIET)
if(EPOXY_FOUND)
    apply_find_vars(epoxy)
endif()

# find dependent libraries
include(apply_find_vars)
if (OPT_COMMUNITY_BUILD)
    find_package(syncdir REQUIRED)
    find_package(netman REQUIRED)
    apply_find_vars(NETMAN SYNCDIR)
    add_definitions(-DIS_CORPORATE=0)
    set(ENABLE_FILE_SHARING "false")
else()
    add_definitions(-DIS_CORPORATE=1)
    set(ENABLE_FILE_SHARING "true")
endif()
find_package(ngrtl REQUIRED core pngio)
find_package(sysinf REQUIRED)
# find_package(benchmarkservice REQUIRED)
apply_find_vars(SYSINF NGRTL )


# add source files and create filter in the project
# basic views and controllers
file(GLOB SRC_COMMON "src/Common/*")
source_group( "Common" FILES ${SRC_COMMON} )

# add utils
file(GLOB SRC_UTILS "src/Utils/*")
source_group( "Utilities" FILES ${SRC_UTILS} )

file(GLOB SRC_PAGES "src/Pages/*")
source_group( "Pages" FILES ${SRC_PAGES} )

file(GLOB SRC_CELLS "src/Cells/*")
source_group( "Cells" FILES ${SRC_CELLS} )

file(GLOB SRC_CONTROLS "src/Controls/*")
source_group( "Controls" FILES ${SRC_CONTROLS} )

file(GLOB SRC_CHARTS "src/Charts/*")
source_group( "Charts" FILES ${SRC_CHARTS} )


list(APPEND TESTFWDIR
        ../frameworks/testfw/schemas/jsonutils.h
        ../frameworks/testfw/messagequeue.h
        ../frameworks/testfw/testfw.cpp
        ../frameworks/testfw/testfw.h
        ../frameworks/testfw/graphics/graphicscontext.h
        ../frameworks/testfw/graphics/eaglgraphicscontext.h
        ../frameworks/testfw/graphics/eaglgraphicscontext.mm
        ../frameworks/testfw/ios/TfwEAGLView.h
        ../frameworks/testfw/ios/TfwEAGLView.mm
        ../frameworks/testfw/ios/NUIResultDetailView.h
        ../frameworks/testfw/ios/NUIResultDetailView.mm
        ../frameworks/testfw/graphics/metalgraphicscontext.h
        ../frameworks/testfw/graphics/metalgraphicscontext.mm
)
source_group( "TestFWDir" FILES ${TESTFWDIR} )



file(GLOB APP_IMAGES res/img/*.png)
file(GLOB XIBS src/Xibs/*)
list(APPEND RESOURCE
            ${XIBS}
            ${APP_IMAGES}
            ${BENCHMARK_RESOURCES}

            # raw and config (simple folder path to create a "blue" linked folder in Xcode)
            data
            image
            compare
        )
source_group("Assets" FILES ${RESOURCE} )

# Set include directories to allow the files to see each other
include_directories(
    ../frameworks/testfw
    ../frameworks/testfw/schemas
    ../frameworks/testfw/graphics
    ../frameworks/testfw/ios
    ../frameworks/benchmark_service/src
    ${CMAKE_CURRENT_BINARY_DIR}/benchmark_service

    src

    ${APPLY_FIND_VARS}
    ${POCO_INCLUDE_DIRS}
    ${epoxy_INCLUDE_DIRS}
)

# Add the source groups to the project
add_executable(${PROJECT_NAME}
            MACOSX_BUNDLE
            ${SRC_CELLS}
            ${SRC_CHARTS}
            ${SRC_COMMON}
            ${SRC_CONTROLS}
            ${SRC_PAGES}
            ${SRC_UTILS}

            ${TESTFWDIR}

            ${RESOURCE}
)

if("$ENV{STORE_VERSION}" STREQUAL "true")
    add_definitions(-DSTORE_VERSION=1)
endif()


# Disabling ARC for some files
set_source_files_properties(
    src/Pages/NUITestViewController.mm
    src/Utils/TestContextProvider.mm
    ../frameworks/testfw/ios/TfwEAGLView.mm
    PROPERTIES COMPILE_FLAGS "-fno-objc-arc")


if(NOT TARGET tfw_schemas)
    add_subdirectory(../frameworks/testfw/schemas ${CMAKE_BINARY_DIR}/tfw_schemas)
endif()
if(NOT TARGET tfw_deviceinfo)
    add_subdirectory(../frameworks/testfw/deviceinfo ${CMAKE_BINARY_DIR}/tfw_deviceinfo)
endif()
if(NOT TARGET benchmarkservice)
    add_subdirectory(../frameworks/benchmark_service ${CMAKE_BINARY_DIR}/benchmark_service)
endif()

target_link_libraries(${PROJECT_NAME}
        tfw_schemas
        ${TESTFW_LIBRARIES}
        ${APPLY_FIND_LIBRARIES}
        "-framework WebKit -framework UIKit -framework QuartzCore -framework AVFoundation -framework Security -framework CoreMedia -framework CoreText -framework CoreGraphics -framework CoreImage -framework OpenGLES -framework SystemConfiguration"
        "-weak_framework Metal"
        ${POCO_LIBRARIES}
        ${epoxy_LIBRARIES}
        tfw_deviceinfo
        benchmarkservice
        )

# Add resources as project resources also (this causes duplication inside the project but we can't do it other way)
set_target_properties(${PROJECT_NAME} PROPERTIES RESOURCE "${RESOURCE}")

set (CODESIGN_ID "iPhone Developer")

set_property(TARGET ${PROJECT_NAME} PROPERTY XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER ${PACKAGE_NAME})

#XCode8
if(DEFINED ENV{GFXBENCH_STORE_PROVISIONING_PROFILE_SPECIFIER} AND DEFINED ENV{GFXBENCH_PROVISIONING_PROFILE_SPECIFIER})

if (STORE_VERSION)
    set_target_properties(${PROJECT_NAME} PROPERTIES XCODE_ATTRIBUTE_PROVISIONING_PROFILE_SPECIFIER "$ENV{GFXBENCH_STORE_PROVISIONING_PROFILE_SPECIFIER}")
else()
    set_target_properties(${PROJECT_NAME} PROPERTIES XCODE_ATTRIBUTE_PROVISIONING_PROFILE_SPECIFIER "$ENV{GFXBENCH_PROVISIONING_PROFILE_SPECIFIER}")
endif()

if ( "$ENV{DISTRIBUTION_TYPE}" STREQUAL "adhoc" OR "$ENV{DISTRIBUTION_TYPE}" STREQUAL "media")
    set (CODESIGN_ID "iPhone Distribution")
    set_target_properties(${PROJECT_NAME} PROPERTIES XCODE_ATTRIBUTE_PROVISIONING_PROFILE_SPECIFIER "$ENV{GFXBENCH_STORE_PROVISIONING_PROFILE_SPECIFIER}")
endif()

endif()


if(DEFINED ENV{GFXBENCH_DEVELOPMENT_TEAM})
	set_target_properties(${PROJECT_NAME} PROPERTIES XCODE_ATTRIBUTE_DEVELOPMENT_TEAM "$ENV{GFXBENCH_DEVELOPMENT_TEAM}")
endif()
#!Xcode8


message(STATUS "CODESIGN_ID = " ${CODESIGN_ID})
set_target_properties(${PROJECT_NAME} PROPERTIES XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY ${CODESIGN_ID})

# Switch off dead code stripping to preserve factory methods
set_target_properties(${PROJECT_NAME} PROPERTIES XCODE_ATTRIBUTE_DEAD_CODE_STRIPPING "No")

# Enable arc for the entire project
set_target_properties(${PROJECT_NAME} PROPERTIES XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC "YES")

# Set the compiler explicitly to match the one used under testfw
# set_target_properties(${PROJECT_NAME} PROPERTIES XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libstdc++")

# Architecture settings - the tool chain set these
#set_target_properties(${PROJECT_NAME} PROPERTIES XCODE_ATTRIBUTE_IPHONEOS_ARCHS "armv7s armv7")
#set_target_properties(${PROJECT_NAME} PROPERTIES XCODE_ATTRIBUTE_IPHONEOS_VALID_ARCHS "armv7s armv7")

if(NOT STORE_VERSION)
    set (${PROJECT_NAME} PROPERTIES XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH "YES")
endif()

# Set iPhone and iPad target devices
set_target_properties(${PROJECT_NAME} PROPERTIES XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY "1,2")

# Set deployment target
set_target_properties(${PROJECT_NAME} PROPERTIES XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET "15.4")

# Asset catalog usage
set_target_properties(${PROJECT_NAME} PROPERTIES XCODE_ATTRIBUTE_ASSETCATALOG_COMPILER_APPICON_NAME "AppIcon")
set_target_properties(${PROJECT_NAME} PROPERTIES XCODE_ATTRIBUTE_ASSETCATALOG_COMPILER_LAUNCHIMAGE_NAME "LaunchImage")

# Keep debug symbols for crash report
set_target_properties(${PROJECT_NAME} PROPERTIES XCODE_ATTRIBUTE_GCC_GENERATE_DEBUGGING_SYMBOLS "YES")

# At last but not least add the info.plist to the project (see below what it contains)
set(BENCHMARK_BUILD_NUMBER $ENV{BUILD_NUMBER})

if($ENV{COMMUNITY_BUILD})
    set(PRODUCT_VERSION_META_POSTFIX "+community")
else()
    set(PRODUCT_VERSION_META_POSTFIX "+corporate")
endif()

#
# Get the benchmark name from the variables set up in buld.sh
#
set(ARCHIVE_NAME "${BUNDLE_ID}-$ENV{PRODUCT_VERSION}${PRODUCT_VERSION_META_POSTFIX}")
set(BUIVERSION "5.0.5")

set_target_properties(${PROJECT_NAME} PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_LIST_DIR}/iOS-Info.plist.in)

# To workaround the archiving problem
set_target_properties(${PROJECT_NAME} PROPERTIES
    XCODE_ATTRIBUTE_INSTALL_PATH "$(LOCAL_APPS_DIR)"
    XCODE_ATTRIBUTE_SKIP_INSTALL "No"
)

# Factory method keepeing
set_target_properties(${PROJECT_NAME} PROPERTIES
    XCODE_ATTRIBUTE_COPY_PHASE_STRIP "NO"
    XCODE_ATTRIBUTE_STRIP_INSTALLED_PRODUCT "NO"
    XCODE_ATTRIBUTE_STRIP_STYLE "debugging"
)

# Bitcode daisabled
set_target_properties(${PROJECT_NAME} PROPERTIES
    XCODE_ATTRIBUTE_ENABLE_BITCODE "NO"
)

include(generate_xcarchive)
generate_xcarchive(${PROJECT_NAME})
