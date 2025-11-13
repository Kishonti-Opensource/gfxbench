cmake_minimum_required(VERSION 2.8)

set(CMAKE_DEBUG_POSTFIX _d)

set (PROJECT_NAME "testfw_app")
project(${PROJECT_NAME})

# set the bundle identifier
set (MACOSX_BUNDLE_GUI_IDENTIFIER "net.kishonti.testfw-app")
set (MACOSX_BUNDLE_INFO_STRING "")
set (MACOSX_BUNDLE_BUNDLE_VERSION "1.0")
set (MACOSX_BUNDLE_LONG_VERSION_STRING "1.0")
set (MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0")
set (BUNDLE MACOSX_BUNDLE)

#add ngrtl libraries
find_package(ngrtl REQUIRED core)

# add source files and create filter in the project
# basic views and controllers

set (TESTFW_DIR "${CMAKE_CURRENT_LIST_DIR}/../..")

list(APPEND APP_SOURCE
	AppDelegate.h
	AppDelegate.mm
	NSFileManagerLocations.h
	NSFileManagerLocations.mm
	PickerViewController.h
	PickerViewController.mm
	ViewController.h
	ViewController.mm
	GlViewController.h
	GlViewController.mm
	METLViewController.h
	METLViewController.mm
	main.m
)
source_group("testfw-app" FILES ${APP_SOURCE})

list(APPEND TESTFW_SOURCE
	${TESTFW_DIR}/messagequeue.h
	${TESTFW_DIR}/testfw.cpp
	${TESTFW_DIR}/testfw.h
	${TESTFW_DIR}/tfwmessagequeue.h
	${TESTFW_DIR}/graphics/eaglgraphicscontext.h
	${TESTFW_DIR}/graphics/eaglgraphicscontext.mm
	${TESTFW_DIR}/graphics/graphicscontext.h
	${TESTFW_DIR}/graphics/metalgraphicscontext.h
	${TESTFW_DIR}/graphics/metalgraphicscontext.mm
    ${TESTFW_DIR}/ios/TfwEAGLView.h
    ${TESTFW_DIR}/ios/TfwEAGLView.mm
    ${TESTFW_DIR}/ios/NUIResultDetailView.h
    ${TESTFW_DIR}/ios/NUIResultDetailView.mm
)

source_group("testfw" FILES ${TESTFW_SOURCE})

file(GLOB app_images "*.png")
list(APPEND RESOURCE
	${app_images}
	${TESTFW_DIR}/resultdetail
	${TESTFW_APP_EXTRA_RESOURCE_DIRS}
	LaunchScreen.xib
	MainStoryboard_iPad.storyboard
	MainStoryboard_iPhone.storyboard
)
source_group("assets" FILES ${RESOURCE})

include_directories(
	${TESTFW_DIR}
	${TESTFW_DIR}/ios
	${NGRTL_INCLUDE_DIRS}
)

# Add the source groups to the project
add_executable(${PROJECT_NAME}
	${BUNDLE}
	${APP_SOURCE}
	${TESTFW_SOURCE}
	${SCHEMA_SOURCE}
	${RESOURCE}
)
target_link_libraries(${PROJECT_NAME}
	tfw_schemas
	tfw_deviceinfo
	${NGRTL_LIBRARIES}
	${TESTFW_LIBRARIES}
	"-weak_framework Metal"
	"-framework OpenGLES"
	"-framework UIKit"
	"-framework QuartzCore"
)

set_source_files_properties(
	METLViewController.mm
	GlViewController.mm
	${TESTFW_DIR}/graphics/eaglgraphicscontext.mm
	${TESTFW_DIR}/ios/TfwEAGLView.mm
	PROPERTIES COMPILE_FLAGS "-fno-objc-arc"
)

# Add resources as project resources also (this causes duplication inside the project but we can't do it other way)
set_target_properties(${PROJECT_NAME} PROPERTIES RESOURCE "${RESOURCE}")

# Add a code signing identity
set (CODESIGN_ID "iPhone Developer")

set_target_properties(${PROJECT_NAME} PROPERTIES XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY ${CODESIGN_ID})

# Switch off dead code stripping to preserve factory methods
set_target_properties(${PROJECT_NAME} PROPERTIES XCODE_ATTRIBUTE_DEAD_CODE_STRIPPING "No")
# Enable arc for the entire project
set_target_properties(${PROJECT_NAME} PROPERTIES XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC "YES")

# Set the compiler explicitly to match the one used under testfw
# set_target_properties(${PROJECT_NAME} PROPERTIES XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++")

# Architecture settings - the tool chain set these
set_target_properties(${PROJECT_NAME} PROPERTIES XCODE_ATTRIBUTE_IPHONEOS_ARCHS "armv7")
set_target_properties(${PROJECT_NAME} PROPERTIES XCODE_ATTRIBUTE_IPHONEOS_VALID_ARCHS "armv7")

# Set iPhone and iPad target devices
set_target_properties(${PROJECT_NAME} PROPERTIES XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY "1,2")

# Set deployment target
#set_target_properties(${PROJECT_NAME} PROPERTIES XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET "7.0")

# Asset catalog usage
set_target_properties(${PROJECT_NAME} PROPERTIES XCODE_ATTRIBUTE_ASSETCATALOG_COMPILER_APPICON_NAME "AppIcon")
set_target_properties(${PROJECT_NAME} PROPERTIES XCODE_ATTRIBUTE_ASSETCATALOG_COMPILER_LAUNCHIMAGE_NAME "LaunchImage")

# Keep debug symbols for crash report
set_target_properties(${PROJECT_NAME} PROPERTIES XCODE_ATTRIBUTE_GCC_GENERATE_DEBUGGING_SYMBOLS "YES")

# At last but not least add the info.plist to the project (see below what it contains)
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
set_target_properties(${PROJECT_NAME} PROPERTIES
	XCODE_ATTRIBUTE_COPY_PHASE_STRIP "NO"
	XCODE_ATTRIBUTE_STRIP_INSTALLED_PRODUCT "NO"
	XCODE_ATTRIBUTE_STRIP_STYLE "debugging"
)

include(generate_ipa)
generate_ipa(${PROJECT_NAME})
foreach(cfg ${CMAKE_CONFIGURATION_TYPES})
    install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${cfg}${CMAKE_XCODE_EFFECTIVE_PLATFORMS}/${PROJECT_NAME}.app DESTINATION bin CONFIGURATIONS ${cfg})
endforeach()
