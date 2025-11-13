# Build tag generator function: 
#
# BUILD_TAG( <tag> <tag_debug>)
# 
# This function returns strings in the 'tag' and 'tag_debug' parameters which describes
# the current platform/compilerversion with a similar encoding like the boost library naming.
# the add_library command.
#
# Example:
#     tag = vc71-mt
#     tag_debug = vc71-mt-gd
#
# tag rules:
# - if msvc append vcXX for visual studio compiler with version number
# - append -64 for 64-bit buid
# - append -s for using static runtime libs
# - append -gd for debug runtime and debug build

FUNCTION(build_tag bt_tag_out bt_tag_debug_out)

	unset(tag)
	unset(multithreaded)
	unset(static_runtime_libs)
	unset(build64)
	unset(rtltag)
	unset(rtltag_debug)
	unset(tag)
	unset(tag_debug)

	#the default 64-bit build tag, can be changed if needed (for example Itanium needs another tag)
	set(build64_tag 64)
	
	if (MSVC)
		if (MSVC_VERSION EQUAL 1200)
			set(tag vc60)
		elseif (MSVC_VERSION EQUAL 1300)
			set(tag vc70)
		elseif (MSVC_VERSION EQUAL 1310)
			set(tag vc71)
		elseif (MSVC_VERSION EQUAL 1400)
			set(tag vc80)
		elseif (MSVC_VERSION EQUAL 1500)
			set(tag vc90)
		elseif (MSVC_VERSION EQUAL 1600)
			set(tag vc100)
		elseif (MSVC_VERSION EQUAL 1700)
			set(tag vc110)
		elseif (MSVC_VERSION EQUAL 1800)
			set(tag vc120)
		elseif (MSVC_VERSION EQUAL 1900)
			set(tag vc130)	
		else()
			message(FATAL_ERROR "Please extend the BuildTag.cmake file with your MSVC version (${MSVC_VERSION})")
		endif()
		
		if (WINCE)
			set(tag "${tag}ce$ENV{CEVer}")
			set(CMAKE_CXX_SIZEOF_DATA_PTR 4)
		endif()
		
		#cmake-msvc uses multithreaded DLL runtime always
		set(multithreaded 1)
		set(static_runtime_libs "${OPT_USE_STATIC_RUNTIME}")
	elseif(CYGWIN)
		set(multithreaded 1)
		if ( OPT_USE_STATIC_RUNTIME )
			message(FATAL_ERROR "OPT_USE_STATIC_RUNTIME is no supported on cygwin")
		endif()
		set(static_runtime_libs OFF)
		set(tag cygwin)
	elseif (IOS)
		if("${IOS_PLATFORM}" STREQUAL "OS")
			set(tag ios)
		elseif("${IOS_PLATFORM}" STREQUAL "SIMULATOR")
			set(tag iossim)
		endif()
		set(multithreaded 0)
		set(static_runtime_libs OFF)
		set(CMAKE_CXX_SIZEOF_DATA_PTR 4)
	elseif (ANDROID)
		set(tag android)
		set(multithreaded 0)
		set(static_runtime_libs OFF)
		set(CMAKE_CXX_SIZEOF_DATA_PTR 4)
	elseif (APPLE)
		set(tag macosx)
		set(multithreaded 1)
		set(static_runtime_libs OFF)
	elseif (WIN32 AND CMAKE_COMPILER_IS_GNUCC)
			set(multithreaded 1)
			set(static_runtime_libs OFF)
			set(tag mingw)
	elseif (UNIX)
		set(tag linux)
		set(multithreaded 1)
		set(static_runtime_libs OFF)
	else()
		message(FATAL_ERROR "Please extend the BuildTag.cmake file with your combination of platform/compilerversion")
	endif()
	
	if(NOT DEFINED multithreaded)
		message(FATAL_ERROR "Please set the multithreaded variable on your platform")
	endif()
	
	if(NOT DEFINED static_runtime_libs)
		message(FATAL_ERROR "Please set the static_runtime_libs variable on your platform")
	endif()
	
	if(NOT DEFINED CMAKE_CXX_SIZEOF_DATA_PTR)
		set(CMAKE_CXX_SIZEOF_DATA_PTR ${CMAKE_C_SIZEOF_DATA_PTR})
	endif()
	if(CMAKE_CXX_SIZEOF_DATA_PTR EQUAL 4)
		set(build64 0)
	elseif(CMAKE_CXX_SIZEOF_DATA_PTR EQUAL 8)
		set(build64 1)
	else()
		message(FATAL_ERROR "Invalid sizeof(void*): ${CMAKE_CXX_SIZEOF_DATA_PTR}")
	endif()
	
	if(build64)
		set(tag "${tag}-${build64_tag}")
	endif()
	
	if(multithreaded)
		set(tag "${tag}-mt")
	endif()
	
	if(static_runtime_libs)
		set(rtltag "s")
		set(rtltag_debug "sgd")
	else()
		set(rtltag "")
		set(rtltag_debug "gd")
	endif()
	
	if(rtltag_debug)
		set(tag_debug "${tag}-${rtltag_debug}")
	else()
		message(FATAL_ERROR "rtltag_debug is empty, no difference between debug and release tags")
	endif()

	if (rtltag)
		set(tag "${tag}-${rtltag}")
	endif()
	
	set(${bt_tag_out} ${tag} PARENT_SCOPE)
	if(DEFINED bt_tag_debug_out)
		set(${bt_tag_debug_out} ${tag_debug} PARENT_SCOPE)
	endif()

ENDFUNCTION()
