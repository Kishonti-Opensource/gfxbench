# read_version_info(
#	versiontag                     # output variable, recieves version tag: e.g. 3.0.1-rc1+5
#	[FROM_FILE name]               # default: VERSION
#	[VERSION_NUMBERS versionlist]  # output variable, recieves version number list: e.g. 3,0,1,0
#)
#
include(CMakeParseArguments)

function(read_version_info versiontag)
	cmake_parse_arguments(rvi "" "FROM_FILE;VERSION_NUMBERS" "" ${ARGN})
	list(LENGTH rvi_UNPARSED_ARGUMENTS unparsed_count)
	if (unparsed_count GREATER 0)
		message(FATAL_ERROR "Unrecognized parameters: ${rvi_UNPARSED_ARGUMENTS}")
	endif()

	if(DEFINED rvi_FROM_FILE)
		set(version_file "${rvi_FROM_FILE}")
	else()
		set(version_file VERSION)
	endif()

	set(build_number "$ENV{BUILD_NUMBER}")
	if(build_number STREQUAL "")
		set(build_number 0)
		message(WARNING "BUILD_NUMBER was not set, using 0 as default value")
	endif()

	file(READ "${version_file}" version)
	string(STRIP "${version}" version)
	if (DEFINED rvi_VERSION_NUMBERS)
		string(REGEX MATCHALL [0-9]+ version_numbers ${version})
		list(APPEND version_numbers "${build_number}")
		string(REPLACE ";" "," version_numbers "${version_numbers}")
		set(${rvi_VERSION_NUMBERS} "${version_numbers}" PARENT_SCOPE)
	endif()

	string(REGEX REPLACE "[ \n\t]"  "" version_changecount ${version}+${build_number})
	set(${versiontag} "${version_changecount}" PARENT_SCOPE)

endfunction()
