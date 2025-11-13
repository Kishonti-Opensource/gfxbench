# try_find_package_cfg()
#
# This macro should be called from a find module.
# It calls find_package() with the same parameters, except
# - removes any REQUIRED if specified
# - adds QUIET - this has been removed
# - adds NO_MODULE
#
# It does not handle all possible arguments given to the original find_package call,
# only the following ones:
#
# - VERSION
# - EXACT
# - REQUIRED
# - COMPONENTS and component list

macro(try_find_package_cfg)

	get_filename_component(TFPC_FIND_MODULE_FILENAME ${CMAKE_CURRENT_LIST_FILE} NAME)
	string(REGEX MATCH "^Find(.+)[.]cmake$" TFPC_V ${TFPC_FIND_MODULE_FILENAME})
	set(TFPC_PACKAGE_NAME ${CMAKE_MATCH_1})
	string(TOUPPER "${TFPC_PACKAGE_NAME}" TFPC_PACKAGE_NAME_UPPER)
	if(NOT TFPC_PACKAGE_NAME)
		message(FATAL_ERROR "try_find_package_cfg must be called from a find module, it was called from ${TFPC_FIND_MODULE_FILENAME}")
	endif()

	set(TFPC_ARGV ${TFPC_PACKAGE_NAME})

	list(APPEND TFPC_ARGV ${${TFPC_PACKAGE_NAME}_FIND_VERSION})

	if(${TFPC_PACKAGE_NAME}_FIND_VERSION_EXACT)
		list(APPEND TFPC_ARGV EXACT)
	endif()

	if(${TFPC_PACKAGE_NAME}_FIND_COMPONENTS)
		list(APPEND TFPC_ARGV COMPONENTS ${${TFPC_PACKAGE_NAME}_FIND_COMPONENTS})
	endif()

	# call CONFIG mode
	# without NO_CMAKE_BUILDS_PATH finds files like <name-binary-dir>/<name>config.cmake accidentaly in the build directory
	find_package(${TFPC_ARGV} NO_MODULE NO_CMAKE_BUILDS_PATH QUIET)
	if(NOT TFPC_PACKAGE_NAME AND NOT TFPC_PACKAGE_NAME_UPPER AND ${TFPC_PACKAGE_NAME}_DIR)
		# config module was found but reported failure (NOT <package-name>_FOUND)
		# repeat calling without QUIET and with optional REQUIRED to display error messages
		# the reason is that if there's a config module, and it's required we don't
		# want to fall back to the find-module but stop here.
		if(${TFPC_PACKAGE_NAME}_FIND_REQUIRED)
			list(APPEND TFPC_ARGV REQUIRED)
		endif()
		find_package(${TFPC_ARGV} NO_MODULE NO_CMAKE_BUILDS_PATH)
	endif()
endmacro()
