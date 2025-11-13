# When called in a find module (Find*.cmake)
# sets FPFH_ARGV to the following string:
#
#     [<version>] [EXACT] [QUIET] [REQUIRED|COMPONENTS] [<component list>]
#
# where the individual components are set according to the
# arguments supplied to the original find_package() call.
#
# Use this to forward the call to another find_package like this:
#
#     find_package(<packagename> ${FPFH_ARGV})
#

macro(find_package_forwarding_helper fpfh_package_name)

	unset(FPFH_ARGV)
	
	list(APPEND FPFH_ARGV ${${fpfh_package_name}_FIND_VERSION})
	
	if(${fpfh_package_name}_FIND_VERSION_EXACT)
		list(APPEND FPFH_ARGV EXACT)
	endif()

	if(${fpfh_package_name}_FIND_QUIETLY)
		list(APPEND FPFH_ARGV QUIET)
	endif()

	if(${fpfh_package_name}_FIND_REQUIRED)
		list(APPEND FPFH_ARGV REQUIRED)
	elseif(${fpfh_package_name}_FIND_COMPONENTS)
		list(APPEND FPFH_ARGV COMPONENTS)
	endif()

	list(APPEND FPFH_ARGV ${${fpfh_package_name}_FIND_COMPONENTS})

endmacro()
