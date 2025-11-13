# Platform name guessing function: 
#
# platform_tag(out_name)
# 
# This function sets a variable in the calling scope whose name is contained by out_name
# It will contain the name of the current platform.
# Possible values: windows, windows64, macosx, ios, android, cygwin

function(platform_tag out_var)
	unset(tag)
	
	if (MSVC)
		if(CMAKE_CXX_SIZEOF_DATA_PTR EQUAL 4)
			set (tag windows)
		elseif(CMAKE_CXX_SIZEOF_DATA_PTR EQUAL 8)
			set (tag windows64)
		elseif(WINCE)
			set(CMAKE_CXX_SIZEOF_DATA_PTR 4)
			set (tag wince)
		else()
			message(FATAL_ERROR "Invalid sizeof(void*): ${CMAKE_CXX_SIZEOF_DATA_PTR}")
		endif()
	elseif(CYGWIN)
		set (tag cygwin)
	elseif (IOS)
		set (tag ios)
	elseif (ANDROID)
		set(tag android)
	elseif (APPLE)
		set(tag macosx)
	elseif (WIN32 AND CMAKE_COMPILER_IS_GNUCC)
		set(tag mingw)
	else()
		message(FATAL_ERROR "Please extend the Platform.cmake file with your combination of platform/compilerversion")
	endif()
	
	set(${out_var} ${tag} PARENT_SCOPE)

endfunction()
