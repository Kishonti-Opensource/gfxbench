# This file is based off of the Platform/Darwin.cmake and Platform/UnixPaths.cmake
# files which are included with CMake 2.8.4
# It has been altered for iOS development

# Options:
#
# IOS_PLATFORM = OS (default) or SIMULATOR
#   This decides if SDKS will be selected from the iPhoneOS.platform or iPhoneSimulator.platform folders
#   OS - the default, used to build for iPhone and iPad physical devices, which have an arm arch.
#   SIMULATOR - used to build for the Simulator platforms, which have an x86 arch.
#
# CMAKE_IOS_DEVELOPER_ROOT = automatic(default) or /path/to/platform/Developer folder
#   By default this location is automatcially chosen based on the IOS_PLATFORM value above.
#   If set manually, it will override the default location and force the user of a particular Developer Platform
#
# CMAKE_IOS_SDK_ROOT = automatic(default) or /path/to/platform/Developer/SDKs/SDK folder
#   By default this location is automatcially chosen based on the CMAKE_IOS_DEVELOPER_ROOT value.
#   In this case it will always be the most up-to-date SDK found in the CMAKE_IOS_DEVELOPER_ROOT path.
#   If set manually, this will force the use of a specific SDK version

# Macros:
#
# set_xcode_property (TARGET XCODE_PROPERTY XCODE_VALUE)
#  A convenience macro for setting xcode specific properties on targets
#  example: set_xcode_property (myioslib IPHONEOS_DEPLOYMENT_TARGET "3.1")
#
# find_host_package (PROGRAM ARGS)
#  A macro used to find executable programs on the host system, not within the iOS environment.
#  Thanks to the android-cmake project for providing the command


# Standard settings
set (CMAKE_SYSTEM_NAME Darwin)
set (CMAKE_SYSTEM_VERSION 1)
set (UNIX True)
set (APPLE True)
set (IOS True)

# Determine the cmake host system version so we know where to find the iOS SDKs
find_program (CMAKE_UNAME uname /bin /usr/bin /usr/local/bin)
if (CMAKE_UNAME)
	exec_program(uname ARGS -r OUTPUT_VARIABLE CMAKE_HOST_SYSTEM_VERSION)
	string (REGEX REPLACE "^([0-9]+)\\.([0-9]+).*$" "\\1" DARWIN_MAJOR_VERSION "${CMAKE_HOST_SYSTEM_VERSION}")
endif (CMAKE_UNAME)

# Setup iOS developer location unless specified manually with CMAKE_IOS_DEVELOPER_ROOT
find_program(xcode_select "xcode-select")
execute_process(COMMAND ${xcode_select} -p OUTPUT_VARIABLE XCODE_SELECT_PATH OUTPUT_STRIP_TRAILING_WHITESPACE)

# Force the compilers to gcc for iOS
include (CMakeForceCompiler)
set(CMAKE_C_COMPILER ${XCODE_SELECT_PATH}/Toolchains/XcodeDefault.xctoolchain/usr/bin/clang)
set(CMAKE_CXX_COMPILER ${XCODE_SELECT_PATH}/Toolchains/XcodeDefault.xctoolchain/usr/bin/clang++)

# Skip the platform compiler checks for cross compiling
set (CMAKE_CXX_COMPILER_WORKS TRUE)
set (CMAKE_C_COMPILER_WORKS TRUE)

# All iOS/Darwin specific settings - some may be redundant
set (CMAKE_SHARED_LIBRARY_PREFIX "lib")
set (CMAKE_SHARED_LIBRARY_SUFFIX ".dylib")
set (CMAKE_SHARED_MODULE_PREFIX "lib")
set (CMAKE_SHARED_MODULE_SUFFIX ".so")
set (CMAKE_MODULE_EXISTS 1)
set (CMAKE_DL_LIBS "")

set (CMAKE_C_OSX_COMPATIBILITY_VERSION_FLAG "-compatibility_version ")
set (CMAKE_C_OSX_CURRENT_VERSION_FLAG "-current_version ")
set (CMAKE_CXX_OSX_COMPATIBILITY_VERSION_FLAG "${CMAKE_C_OSX_COMPATIBILITY_VERSION_FLAG}")
set (CMAKE_CXX_OSX_CURRENT_VERSION_FLAG "${CMAKE_C_OSX_CURRENT_VERSION_FLAG}")

# Hidden visibilty is required for cxx on iOS 
set (CMAKE_C_FLAGS_DEBUG_INIT "-g")
set (CMAKE_CXX_FLAGS_DEBUG_INIT "-g")
set (CMAKE_C_FLAGS_INIT "")
set (CMAKE_CXX_FLAGS_INIT "-fvisibility=hidden -fvisibility-inlines-hidden")

set (CMAKE_C_LINK_FLAGS "-Wl,-search_paths_first -Wl,-headerpad_max_install_names ${CMAKE_C_LINK_FLAGS}")
set (CMAKE_CXX_LINK_FLAGS "-Wl,-search_paths_first -Wl,-headerpad_max_install_names ${CMAKE_CXX_LINK_FLAGS}")

set (CMAKE_PLATFORM_HAS_INSTALLNAME 1)
set (CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS "-dynamiclib -headerpad_max_install_names")
set (CMAKE_SHARED_MODULE_CREATE_C_FLAGS "-bundle -headerpad_max_install_names")
set (CMAKE_SHARED_MODULE_LOADER_C_FLAG "-Wl,-bundle_loader,")
set (CMAKE_SHARED_MODULE_LOADER_CXX_FLAG "-Wl,-bundle_loader,")
set (CMAKE_FIND_LIBRARY_SUFFIXES ".dylib" ".so" ".a")

# hack: if a new cmake (which uses CMAKE_INSTALL_NAME_TOOL) runs on an old build tree
# (where install_name_tool was hardcoded) and where CMAKE_INSTALL_NAME_TOOL isn't in the cache
# and still cmake didn't fail in CMakeFindBinUtils.cmake (because it isn't rerun)
# hardcode CMAKE_INSTALL_NAME_TOOL here to install_name_tool, so it behaves as it did before, Alex
if (NOT DEFINED CMAKE_INSTALL_NAME_TOOL)
	find_program(CMAKE_INSTALL_NAME_TOOL install_name_tool)
endif (NOT DEFINED CMAKE_INSTALL_NAME_TOOL)

# Setup iOS platform unless specified manually with IOS_PLATFORM
if (NOT DEFINED IOS_PLATFORM)
	set (IOS_PLATFORM "OS")
endif (NOT DEFINED IOS_PLATFORM)
set (IOS_PLATFORM ${IOS_PLATFORM} CACHE STRING "Type of iOS Platform")

# Check the platform selection and setup for developer root
if (${IOS_PLATFORM} STREQUAL "OS")
	set (IOS_PLATFORM_LOCATION "iPhoneOS.platform")

	# This causes the installers to properly locate the output libraries
	set (CMAKE_XCODE_EFFECTIVE_PLATFORMS "-iphoneos")
elseif (${IOS_PLATFORM} STREQUAL "SIMULATOR")
	set (IOS_PLATFORM_LOCATION "iPhoneSimulator.platform")

	# This causes the installers to properly locate the output libraries
	set (CMAKE_XCODE_EFFECTIVE_PLATFORMS "-iphonesimulator")
else (${IOS_PLATFORM} STREQUAL "OS")
	message (FATAL_ERROR "Unsupported IOS_PLATFORM value selected. Please choose OS or SIMULATOR")
endif (${IOS_PLATFORM} STREQUAL "OS")

# Note Xcode 4.3 changed the installation location, choose the most recent one available
set (XCODE_POST_43_ROOT "${XCODE_SELECT_PATH}/Platforms/${IOS_PLATFORM_LOCATION}/Developer")
set (XCODE_PRE_43_ROOT "/Developer/Platforms/${IOS_PLATFORM_LOCATION}/Developer")
if (NOT DEFINED CMAKE_IOS_DEVELOPER_ROOT)
	if (EXISTS ${XCODE_POST_43_ROOT})
		set (CMAKE_IOS_DEVELOPER_ROOT ${XCODE_POST_43_ROOT})
	elseif(EXISTS ${XCODE_PRE_43_ROOT})
		set (CMAKE_IOS_DEVELOPER_ROOT ${XCODE_PRE_43_ROOT})
	endif (EXISTS ${XCODE_POST_43_ROOT})
endif (NOT DEFINED CMAKE_IOS_DEVELOPER_ROOT)
set (CMAKE_IOS_DEVELOPER_ROOT ${CMAKE_IOS_DEVELOPER_ROOT} CACHE PATH "Location of iOS Platform")

# Find and use the most recent iOS sdk unless specified manually with CMAKE_IOS_SDK_ROOT
if (NOT DEFINED CMAKE_IOS_SDK_ROOT)
	file (GLOB _CMAKE_IOS_SDKS "${CMAKE_IOS_DEVELOPER_ROOT}/SDKs/*")
	if (_CMAKE_IOS_SDKS) 
		list (SORT _CMAKE_IOS_SDKS)
		list (REVERSE _CMAKE_IOS_SDKS)
		list (GET _CMAKE_IOS_SDKS 0 CMAKE_IOS_SDK_ROOT)
	else (_CMAKE_IOS_SDKS)
		message (FATAL_ERROR "No iOS SDK's found in default seach path ${CMAKE_IOS_DEVELOPER_ROOT}. Manually set CMAKE_IOS_SDK_ROOT or install the iOS SDK.")
	endif (_CMAKE_IOS_SDKS)
	message (STATUS "Toolchain using default iOS SDK: ${CMAKE_IOS_SDK_ROOT}")
endif (NOT DEFINED CMAKE_IOS_SDK_ROOT)
set (CMAKE_IOS_SDK_ROOT ${CMAKE_IOS_SDK_ROOT} CACHE PATH "Location of the selected iOS SDK")

# Set the sysroot default to the most recent SDK
set (CMAKE_OSX_SYSROOT ${CMAKE_IOS_SDK_ROOT} CACHE PATH "Sysroot used for iOS support")

# set the architecture for iOS 
# NOTE: Currently both ARCHS_STANDARD_32_BIT and ARCHS_UNIVERSAL_IPHONE_OS set armv7 only, so set both manually
if (${IOS_PLATFORM} STREQUAL "OS")
    set(VERSION 1.0)
    string(FIND "${CMAKE_IOS_SDK_ROOT}" "/" VERSION_POS REVERSE)
    string(SUBSTRING "${CMAKE_IOS_SDK_ROOT}" ${VERSION_POS} -1 VERSION)

    string(REGEX REPLACE "([0-9]+[.]?)+.sdk" "__VERSION__\\0" VERSION_REGEXED ${VERSION})
    string(FIND "${VERSION_REGEXED}" "__VERSION__" VERSION_POS_2 REVERSE)
    string(FIND "${VERSION_REGEXED}" ".sdk" VERSION_POS_3 REVERSE)
    MATH(EXPR VERSION_LENGTH "${VERSION_POS_3}-${VERSION_POS_2} - 11" )
    MATH(EXPR VERSION_POS_FINAL "${VERSION_POS_2} + 11" )
    string(SUBSTRING "${VERSION_REGEXED}" ${VERSION_POS_FINAL} ${VERSION_LENGTH} VERSION)

    message(STATUS "iOS SDK version = ${VERSION}")

    if (NOT VERSION VERSION_LESS 11.0)
        set (IOS_ARCH arm64)
        message(STATUS "Architecture set to [arm64]")
    elseif (NOT VERSION VERSION_LESS 7.0)
        set (IOS_ARCH armv7s armv7 arm64)
        message(STATUS "Architecture set to [armv7s armv7 arm64]")
    else (NOT VERSION VERSION_LESS 7.0)
        set (IOS_ARCH armv7s armv7)
        message(STATUS "Architecture set to [armv7s armv7]")
    endif()
else()
	set (IOS_ARCH i386)
endif()

set (CMAKE_OSX_ARCHITECTURES ${IOS_ARCH} CACHE STRING "Build architecture for iOS")

# Set the find root to the iOS developer roots and to user defined paths
set (CMAKE_FIND_ROOT_PATH ${CMAKE_IOS_DEVELOPER_ROOT} ${CMAKE_IOS_SDK_ROOT} ${CMAKE_PREFIX_PATH} CACHE STRING "iOS find search path root")

# default to searching for frameworks first
set (CMAKE_FIND_FRAMEWORK FIRST)

# set up the default search directories for frameworks
set (CMAKE_SYSTEM_FRAMEWORK_PATH
	${CMAKE_IOS_SDK_ROOT}/System/Library/Frameworks
	${CMAKE_IOS_SDK_ROOT}/System/Library/PrivateFrameworks
	${CMAKE_IOS_SDK_ROOT}/Developer/Library/Frameworks
)

# only search the iOS sdks, not the remainder of the host filesystem
set (CMAKE_FIND_ROOT_PATH_MODE_PROGRAM ONLY)
set (CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set (CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)


# This little macro lets you set any XCode specific property
macro (set_xcode_property TARGET XCODE_PROPERTY XCODE_VALUE)
	set_property (TARGET ${TARGET} PROPERTY XCODE_ATTRIBUTE_${XCODE_PROPERTY} ${XCODE_VALUE})
endmacro (set_xcode_property)


# This macro lets you find executable programs on the host system
macro (find_host_package)
	set (CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
	set (CMAKE_FIND_ROOT_PATH_MODE_LIBRARY NEVER)
	set (CMAKE_FIND_ROOT_PATH_MODE_INCLUDE NEVER)
	set (IOS FALSE)

	find_package(${ARGN})

	set (IOS TRUE)
	set (CMAKE_FIND_ROOT_PATH_MODE_PROGRAM ONLY)
	set (CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
	set (CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
endmacro (find_host_package)

macro (find_host_program)
	set (CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
	set (CMAKE_FIND_ROOT_PATH_MODE_LIBRARY NEVER)
	set (CMAKE_FIND_ROOT_PATH_MODE_INCLUDE NEVER)
	set (IOS FALSE)

	set (APPLE 1)
	set (UNIX)

	find_program(${ARGN})

	set (APPLE)
	set (UNIX 1)
	set (IOS TRUE)
	set (CMAKE_FIND_ROOT_PATH_MODE_PROGRAM ONLY)
	set (CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
	set (CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
endmacro (find_host_program)

if (IOS_STL)
	set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY ${IOS_STL})
endif()

if(NOT IOSINSTALL_INITIALIZED)
	set(IOSINSTALL_INITIALIZED 1)
	set(IOSINSTALL_ALL_INSTALL_KEYWORDS_EXCEPT_TARGETS
		OPTIONAL NAMELINK_ONLY NAMELINK_SKIP DESTINATION COMPONENT INCLUDES PERMISSIONS CONFIGURATIONS
		EXPORT ARCHIVE LIBRARY RUNTIME FRAMEWORK BUNDLE PRIVATE_HEADER PUBLIC_HEADER RESOURCE)
	string(REPLACE ";" "|" IOSINSTALL_ALL_INSTALL_KEYWORDS_EXCEPT_TARGETS_REGEX "${IOSINSTALL_ALL_INSTALL_KEYWORDS_EXCEPT_TARGETS}")
	set(IOSINSTALL_ALL_INSTALL_KEYWORDS_EXCEPT_TARGETS_REGEX "^(${IOSINSTALL_ALL_INSTALL_KEYWORDS_EXCEPT_TARGETS_REGEX})$")

	include(CMakeParseArguments)
	# Known limitations:
	# 1. Setting the archive output dir will break this function:
	#     set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
	# in that case the lib will go to the specd dir/${Config}
	function(install)
		# in all cases we call the original install command
		# to perform all the things except installing
		# libs from the hardcoded xcode directory

		# but even calling for non-targets we need to fix the CONFIGURATIONS values
		# we must be careful not to break existing code which fixes this with in if(IOS)
		# hacks.

		unset(a)
		unset(after_config)
		foreach(i ${ARGN})
			if(NOT after_config)
				if(i MATCHES "^CONFIGURATIONS$")
					set(after_config 1)
				endif()
			else()
				if(i MATCHES "${IOSINSTALL_ALL_INSTALL_KEYWORDS_EXCEPT_TARGETS_REGEX}")
					set(after_config 0)
				else()
					string(FIND "${i}" "${CMAKE_XCODE_EFFECTIVE_PLATFORMS}" idx)
					if(idx EQUAL -1)
						set(i "${i}${CMAKE_XCODE_EFFECTIVE_PLATFORMS}")
					endif()
				endif()
			endif()
			list(APPEND a "${i}")
		endforeach()
		set(ARGN ${a})

		_install(${ARGN})
		# re-escape the $ for the hack that has been in use to fix this ios install bug
		#unset(_IOSINSTALL_ARGN)
		#foreach(_IOSINSTALL_i ${ARGN})
		#	message(STATUS "i:${_IOSINSTALL_i}")
		#	string(REPLACE "$" "\\$" _IOSINSTALL_j "${_IOSINSTALL_i}")
		#	message(STATUS "j:${_IOSINSTALL_j}")
		#	list(APPEND _IOSINSTALL_ARGN "${_IOSINSTALL_j}")
		#endforeach()
		#_install(${_IOSINSTALL_ARGN})
		if("${ARGV0}" MATCHES "^TARGETS$")
			# find the first word after the last target
			unset(targets)
			list(REMOVE_AT ARGN 0) # remove the word 'TARGETS'
			while(1)
				if(NOT ARGN)
					break()
				endif()
				list(GET ARGN 0 head)
				list(FIND IOSINSTALL_ALL_INSTALL_KEYWORDS_EXCEPT_TARGETS "${head}" f)
				if(f EQUAL -1)
					# it's a name of a target
					list(APPEND targets "${head}")
					list(REMOVE_AT ARGN 0)
				else()
					# it's past the last target
					break()
				endif()
			endwhile()
			# now we have the target names in the variable 'targets'
			# and ARGN contains the rest of the args
			cmake_parse_arguments(
				IOSINSTALL
				""
				"EXPORT"
				"ARCHIVE;LIBRARY;RUNTIME;FRAMEWORK;BUNDLE;PRIVATE_HEADER;PUBLIC_HEADER;RESOURCE"
				${ARGN}
			)
			# we're not interested in all kinds of targets:
			# - RUNTIME (executable, n/a with cmake/IOS)
			# - ARCHIVE (*.a)
			# - MODULE (module lib, n/a on IOS)
			# - LIBRARY (*.so, n/a on IOS)
			# so it's only ARCHIVE for now
			# and also, if none of the above has been specified
			# the unparsed arguments can be interpreted
			# for all kinds of targets (ARCHIVE..) at the same time

			unset(kinds_to_process)
			foreach(i ARCHIVE UNPARSED_ARGUMENTS)
				if(IOSINSTALL_${i})
					list(APPEND kinds_to_process ${i})
				endif()
			endforeach()

			foreach(i ${kinds_to_process})
				cmake_parse_arguments(
					IOSINSTALLKIND
					"OPTIONAL;NAMELINK_ONLY;NAMELINK_SKIP"
					"DESTINATION;COMPONENT"
					"INCLUDES;PERMISSIONS;CONFIGURATIONS"
					${IOSINSTALL_${i}}
				)
				# collect arguments for the install directory call
				unset(extra_args)
				if(IOSINSTALLKIND_PERMISSIONS)
					list(APPEND extra_args FILE_PERMISSIONS ${IOSINSTALLKIND_PERMISSIONS})
				endif()
				if(IOSINSTALLKIND_OPTIONAL)
					list(APPEND extra_args OPTIONAL)
				endif()
				if(IOSINSTALLKIND_CONFIGURATIONS)
					list(APPEND extra_args CONFIGURATIONS ${IOSINSTALLKIND_CONFIGURATIONS})
				endif()
				if(IOSINSTALLKIND_COMPONENT)
					list(APPEND extra_args COMPONENT ${IOSINSTALLKIND_COMPONENT})
				endif()
				foreach(j NAMELINK_ONLY NAMELINK_SKIP)
					if(IOSINSTALLKIND_${j})
						message(WARNING "Keyword not supported in ios toolchain's install(): ${j}")
					endif()
				endforeach()
#				message(STATUS "_install ${CMAKE_CURRENT_BINARY_DIR}/\$ENV{CONFIGURATION}\$ENV{EFFECTIVE_PLATFORM_NAME}/ -> ${IOSINSTALLKIND_DESTINATION} ${extra_args}")
				_install(
					DIRECTORY
						${CMAKE_CURRENT_BINARY_DIR}/\$ENV{CONFIGURATION}\$ENV{EFFECTIVE_PLATFORM_NAME}/
					DESTINATION "${IOSINSTALLKIND_DESTINATION}"
					${extra_args}
					FILES_MATCHING PATTERN "*.a")
			endforeach()
        elseif("${ARGV0}" MATCHES "^EXPORT$")
            cmake_parse_arguments(_IOSARG
                "EXPORT_LINK_INTERFACE_LIBRARIES"
                "EXPORT;DESTINATION;NAMESPACE;FILE;COMPONENT"
                "PERMISSIONS;CONFIGURATIONS"
                ${ARGN})
            if (NOT _IOSARG_FILE)
                set(_IOSARG_FILE ${_IOSARG_EXPORT}.cmake)
            endif()
            if (NOT _IOSARG_CONFIGURATIONS)
                set(_IOSARG_CONFIGURATIONS ${CMAKE_CONFIGURATION_TYPES})
            endif()
            # handle absolute DESTINATION (replace first '/' with '_')
            string(REGEX REPLACE "^/" "_" _IOSARG_DESTINATION_BUILD ${_IOSARG_DESTINATION})
            string(REGEX REPLACE "\\.cmake$" "" _IOSARG_FILEWE "${_IOSARG_FILE}")
            string(TOLOWER "${_IOSARG_CONFIGURATIONS}" _CONFIGURATIONS_LOWER)
            foreach(_ios_config ${_CONFIGURATIONS_LOWER})
                # if CONFIGURATIONS was set in install(EXPORT...) -iphoneos added. remove now.
                string(REGEX REPLACE "-.*" "" _ios_config ${_ios_config})
                install(FILES ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/Export/${_IOSARG_DESTINATION_BUILD}/${_IOSARG_FILEWE}-${_ios_config}.cmake DESTINATION ${_IOSARG_DESTINATION} CONFIGURATIONS ${_ios_config})
	        endforeach()
		endif()
	endfunction()
endif()
