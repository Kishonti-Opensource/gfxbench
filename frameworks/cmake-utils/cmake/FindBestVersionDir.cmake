# FindBestVersionDir
#
# find_best_version_dir(<name> <dir> <prefix> <best_version>)
#
# The function helps finding a directory with
# the best matching version number in FindXXX modules
# The function can be used when the directories are named like this
#
# vc100-mt-gd-1_2_6
# vc100-mt-1_2_6
# vc100-mt-gd-1_1
# vc100-mt-1_1
# vc71-mt-gd_1_2_5
# vc71-mt-1_2_5
#
# So we're looking for a directory with a given prefix. Among those directories we
# need a good one with a version number that fits the requested one at the
# find_package command line.
#
# <name> - the first argument of the find_package command
# <dir> - is the directory where we're searching for the subdirectories
# <prefix> - is the prefix of the subdirs we're interested in
# <best_version> - is a variable where the best version is stored (like 1_2_6)
#
# The function selects the best version based on the XXX_FIND_VERSION* variables
# which will be determined by the version request on the find_package command line.

FUNCTION(find_available_dir_versions _dir _prefix fadv_version_list_out)
	unset(file_list)
	unset(k)
	unset(j)
	unset(t)
	
	file(GLOB file_list RELATIVE ${_dir} "${_dir}/${_prefix}-*")
	
	#remove trailing prefix
	string(LENGTH "${_prefix}-" k)
	foreach(i IN LISTS file_list)
		if ( IS_DIRECTORY ${_dir}/${i} )
			string(SUBSTRING ${i} ${k} -1 j)
			string(REGEX REPLACE "[0-9_]" "" t ${j})
			if(NOT t)
				list(APPEND v ${j})
			else()
			endif()
		endif()
	endforeach()
	
	set(${fadv_version_list_out} ${v} PARENT_SCOPE)
	
ENDFUNCTION()

FUNCTION(is_good_version _name _version igv_is_good_out)

	unset(my_version)
	unset(find_version)
	unset(is_good)
	
	string(REPLACE "_" "." my_version ${_version})
	set(find_version ${${_name}_FIND_VERSION})
	
	if ( ${${_name}_FIND_VERSION_EXACT} )
		if ( my_version VERSION_EQUAL find_version )
			set(is_good 1)
		else()
			set(is_good 0)
		endif()
	else()
		#not exact
		if ( find_version VERSION_GREATER my_version )
			set(is_good 0)
		else()
			set(is_good 1)
		endif()
	endif()
	
	set(${igv_is_good_out} ${is_good} PARENT_SCOPE)
ENDFUNCTION()

FUNCTION(find_matching_version _name _version_list fmv_good_version_out)
	unset(is_good)
	unset(good_version_list)
	unset(good_version)
	unset(good_version_dot)
	if(${_name}_FIND_VERSION)
		#filter the _version_list with the given version
		foreach(i IN LISTS ${_version_list})
			is_good_version(${_name} ${i} is_good)
			if ( is_good )
				list(APPEND good_version_list ${i})
			endif()
		endforeach()
	else()
		set(good_version_list ${${_version_list}})
	endif()
	#select the best from the good_version_list
	foreach(i IN LISTS good_version_list)
		if(NOT DEFINED good_version)
			set(good_version ${i})
		else()
			string(REPLACE "_" "." i2 ${i})
			string(REPLACE "_" "." good_version_dot ${good_version})
			if (i2 VERSION_GREATER good_version_dot)
				set(good_version ${i})
			endif()
		endif()
	endforeach()
	set(${fmv_good_version_out} ${good_version} PARENT_SCOPE)
ENDFUNCTION()

FUNCTION(find_best_version_dir _name _dir _prefix fbvd_good_version_out)
	unset(version_list)
	unset(good_version)
	find_available_dir_versions(${_dir} ${_prefix} version_list)
	find_matching_version(${_name} version_list good_version)
	set(${fbvd_good_version_out} ${good_version} PARENT_SCOPE)
ENDFUNCTION()
