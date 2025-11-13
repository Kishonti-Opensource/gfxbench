# generate_win32_version_file(
#	filename
#	COMPANY_NAME company
#	PRODUCT_NAME product
#	VERSION versiontag
#	VERSION_NUMBERS versionlist
#	[FILE_VERSION fileversiontag] 			# Optional, defaults to versiontag
#	[FILE_VERSION_NUMBERS fileversionlist]  # Optional, defaults to versionlist
#)
#
# Example:
#	generate_version_file(
#       version.rc
#		COMPANY_NAME "Kishonti Informatics Ltd"
#		PRODUCT_NAME "NaviGenie SDK"
#		VERSION 3.0.1-alpha
#		VERSION_NUMBERS 3,0,1,0
#	)

include(CMakeParseArguments)

function(generate_win32_version_file filename)
	cmake_parse_arguments(gvf "" "COMPANY_NAME;PRODUCT_NAME;VERSION;VERSION_NUMBERS;FILE_VERSION;FILE_VERSION_NUMBERS" "" ${ARGN})
	list(LENGTH gvf_UNPARSED_ARGUMENTS unparsed_count)
	if (unparsed_count GREATER 0)
		message(FATAL_ERROR "Unrecognized parameters: ${gvf_UNPARSED_ARGUMENTS}")
	endif()
	if (NOT DEFINED gvf_COMPANY_NAME)
		message(SEND_ERROR " not defined.")
	endif()
	if (NOT DEFINED gvf_PRODUCT_NAME)
		message(SEND_ERROR "PRODUCT_NAME not defined.")
	endif()
	if (NOT DEFINED gvf_VERSION)
		message(SEND_ERROR "VERSION not defined.")
	endif()
	if (NOT DEFINED gvf_VERSION_NUMBERS)
		message(SEND_ERROR "VERSION_NUMBERS not defined.")
	endif()
	if (NOT DEFINED gvf_FILE_VERSION)
		set(gvf_FILE_VERSION ${gvf_VERSION})
	endif()
	if (NOT DEFINED gvf_FILE_VERSION_NUMBERS)
		set(gvf_FILE_VERSION_NUMBERS ${gvf_VERSION_NUMBERS})
	endif()

	set(resourcefile_template "#include <windows.h>

#if defined DEBUG || defined _DEBUG
#define VER_FILEFLAGS 1
#else
#define VER_FILEFLAGS 0
#endif

VS_VERSION_INFO VERSIONINFO
	FILEVERSION ${gvf_FILE_VERSION_NUMBERS}
	PRODUCTVERSION ${gvf_VERSION_NUMBERS}
	FILEFLAGSMASK VS_FFI_FILEFLAGSMASK
	FILEFLAGS VER_FILEFLAGS
	FILEOS VOS__WINDOWS32
	FILETYPE VFT_DLL
	FILESUBTYPE VFT2_UNKNOWN
BEGIN
	BLOCK \"StringFileInfo\"
	BEGIN
		BLOCK \"000004e4\"
		BEGIN
			VALUE \"CompanyName\", \"${gvf_COMPANY_NAME}\"
			VALUE \"FileVersion\", \"${gvf_FILE_VERSION}\"
			VALUE \"ProductName\", \"${gvf_PRODUCT_NAME}\"
			VALUE \"ProductVersion\", \"${gvf_VERSION}\"
		END
	END
	BLOCK \"VarFileInfo\"
	BEGIN
		VALUE \"Translation\", 0x0, 1252
	END
END")

	string(CONFIGURE ${resourcefile_template} resources)
	file(WRITE "${filename}" "${resourcefile_template}")
endfunction()
