# This file provides a helper function to construct csc command line to call csc from cmake.
# It also supports calling csc for WinCE/.Net Compact Framework.
#
#     csc_generate_command_line(
#         <out-var>
#         [DEBUG|OPTIMIZED]
#         ARGS <arg1> <arg2> ...
#         REFERENCES [debug|optimized|general] <ref1> ...)
#
# This function
# - determines the csc command to call
# - adds sets the /optimize, /debug and /define:DEBUG options according to the build type
# - adds the specified args and transform paths to native path
# - adds the specified references observing the debug/optimized/general keywords
#
# <out-var> is the name of the variable that receives the command line.
#
# Example usage:
#
#     csc_generate_command_line(cc ARGS foo.cs /out:foo.dll /target:library)
#     add_custom_target(<target-name> [ALL]
#         COMMAND ${cc}
#         WORKING_DIRECTORY <dir>
#         COMMENT <comment>
#         VERBATIM)
#
# [DEBUG|OPTIMIZED] are optional keywords and controls the setting of
# the /optimize, /debug and /define:DEBUG options.
# If omitted, the CMAKE_BUILD_TYPE must be set and that will be used.
#
# Arguments after ARGS will be copied to the command line. Paths will be transformed
# to native path.
#
# Arguments after REFERENCES will be added with /r:<path>. The path will be transformed
# to native path. Just like for target_link_libraries, you can use the debug/optimized/general
# words to control which reference goes to which build type.
# Restriction: you cannot use <file list> for parameters like /reference:<file list>, only
# single files.
#
# The function sets up the csc command line correctly for .net Compact Framework, too.
# see http://msdn.microsoft.com/en-us/library/ms172492(v=vs.90).aspx
#
# /platform options will be added by the following rule:
# - on desktop, if CMAKE_CL_64 is not set, adds /platform:x86
#
# /define:TRACE will be added.
include(CMakeParseArguments)

function(csc_generate_command_line out_var)
	cmake_parse_arguments(
		CO
		"DEBUG;OPTIMIZED"
		""
		"ARGS;REFERENCES"
		${ARGN})

	if(CO_DEBUG AND CO_OPTIMIZED)
		message(FATAL_ERROR "Both DEBUG and RELEASE are specified")
	endif()

	if(NOT CO_DEBUG AND NOT CO_OPTIMIZED)
		if(NOT CMAKE_BUILD_TYPE)
			message(FATAL_ERROR "Neither DEBUG nor OPTIMIZED was specified and CMAKE_BUILD_TYPES is not set.")
		endif()
		if(CMAKE_BUILD_TYPE MATCHES "^Debug$")
			set(CO_DEBUG 1)
		else()
			set(CO_OPTIMIZED 1)
		endif()
	endif()

	if(CO_DEBUG)
		list(APPEND CO_ARGS /debug /define:DEBUG)
	elseif(CO_OPTIMIZED)
		list(APPEND CO_ARGS /optimize)
	else()
		message(FATAL_ERROR "Internal error, neither CO_DEBUG nor CO_OPTIMIZED are set.")
	endif()

	if(WINCE)
		list(APPEND CO_ARGS /noconfig /nostdlib /define:WindowsCE)
		set(netcf_path "c:/Program Files (x86)/Microsoft.NET/SDK/CompactFramework/v3.5/WindowsCE")
		if(NOT IS_DIRECTORY ${netcf_path})
			message(FATAL_ERROR "NETCF_PATH not found: ${netcf_path}")
		endif()
		set(cf_dlls
			MsCorlib.dll
			System.Core.dll
			System.Data.dll
			System.dll
			System.Drawing.dll
			System.Messaging.dll
			System.Net.IrDA.dll
			System.Web.Services.dll
			System.Windows.Forms.dll
			System.ServiceModel.dll
			System.Runtime.Serialization.dll
			System.Xml.dll
			System.Xml.Linq.dll
			Microsoft.WindowsCE.Forms.dll
			Microsoft.WindowsMobile.DirectX.dll
			Microsoft.ServiceModel.Channels.Mail.dll
			Microsoft.ServiceModel.Channels.Mail.WindowsMobile.dll
		)
		foreach(i ${cf_dlls})
			set(p ${netcf_path}/${i})
			if(NOT EXISTS ${p})
				message(FATAL_ERROR "Missing dll for .Net CF build: ${p}")
			endif()
			list(APPEND CO_REFERENCES ${p})
		endforeach()
	endif()
	
	list(APPEND CO_ARGS /define:TRACE)

	unset(state)
	foreach(i ${CO_REFERENCES})
		if(NOT DEFINED state)
			if(i MATCHES "^(debug|optimized|general)$")
				set(state ${i})
			else()
				list(APPEND CO_ARGS /r:${i})
			endif()
		elseif(state MATCHES "^general$")
			list(APPEND CO_ARGS /r:${i})
			unset(state)
		elseif(state MATCHES "^debug$")
			if(CO_DEBUG)
				list(APPEND CO_ARGS /r:${i})
			endif()
			unset(state)
		elseif(state MATCHES "^optimized$")
			if(CO_OPTIMIZED)
				list(APPEND CO_ARGS /r:${i})
			endif()
			unset(state)
		else()
			message(FATAL_ERROR "Internal error, state: ${state}")
		endif()
	endforeach()

	unset(CO_ARGS2)
	foreach(i ${CO_ARGS})
		if(i MATCHES "^(/(r|reference):[^=]*=)(.*)$")
			set(kw ${CMAKE_MATCH_1})
			set(f ${CMAKE_MATCH_3})
		elseif(i MATCHES "^/(out|doc|reference|r|resource|addmodule|link|l|win32res|win32icon|win32manifest|keyfile|bugreport|pdb|appconfig):(.*)$")
			set(kw /${CMAKE_MATCH_1}:)
			set(f ${CMAKE_MATCH_2})
		elseif(i MATCHES "^@(.*)$")
			set(kw @)
			set(f ${CMAKE_MATCH_1})
		elseif(i MATCHES "^/.*$")
			set(kw ${i})
			unset(f)
		else()
			unset(kw)
			set(f ${i})
		endif()
		if(f)
			file(TO_NATIVE_PATH "${f}" f)
		endif()
		list(APPEND CO_ARGS2 ${kw}${f})
	endforeach()

	if(WINCE)
		set(csc_exe "C:/Windows/Microsoft.NET/Framework/v3.5/csc.exe")
		if(NOT EXISTS "${csc_exe}")
			message(FATAL_ERROR "csc compiler not found: ${csc_exe}")
		endif()
	else()
		set(csc_exe csc)
	endif()

	set(${out_var} ${csc_exe} ${CO_ARGS2} PARENT_SCOPE)
endfunction()
