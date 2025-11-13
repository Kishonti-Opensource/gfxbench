# Usage:
#
# Call find_package:
#
#     find_package(Boost REQUIRED thread)
#     find_package(FreeType REQUIRED)
#
# Then call this macro like this:
#
#     apply_find_vars(Boost FREETYPE)
#
# to apply these attributes of the packages
#
# <pkgname>_INCLUDE_DIRS
# <pkgname>_DEFINITIONS
#
# The macro also concats the <pkgname>_LIBRARIES variables to this variable: APPLY_FIND_LIBRARIES
# so you can apply them like this:
#
#     target_link_libraries(<targetName> ${APPLY_FIND_LIBRARIES})
#

macro(apply_find_vars)

foreach(i ${ARGV})

	if(TARGET ${i})
		list(APPEND APPLY_FIND_LIBRARIES ${i})
	else()
		if(NOT ${i}_INCLUDE_DIRS AND NOT ${i}_INCLUDE_DIR AND NOT ${i}_LIBRARIES)
			message(FATAL_ERROR "Nor ${i}_INCLUDE_DIR(S) neither ${i}_LIBRARIES are set, check for possible error.")
		endif()
		if(${i}_INCLUDE_DIRS AND ${i}_INCLUDE_DIR AND NOT "${${i}_INCLUDE_DIRS}" STREQUAL "${${i}_INCLUDE_DIR}")
			message("found both ${i}_INCLUDE_DIRS and ${i}_INCLUDE_DIR")
			message("using ${i}_INCLUDE_DIRS, the other one is ignored")
			message("${i}_INCLUDE_DIR=${${i}_INCLUDE_DIR}")
			message("${i}_INCLUDE_DIRS=${${i}_INCLUDE_DIRS}")
		endif()
		
		if (${i}_INCLUDE_DIRS)
			set(inc_dir "${${i}_INCLUDE_DIRS}")
		elseif (${i}_INCLUDE_DIR)
			set(inc_dir "${${i}_INCLUDE_DIR}")
		endif()
		if (inc_dir)
			include_directories(${inc_dir})
			list(APPEND APPLY_FIND_INCLUDE_DIRS ${inc_dir})
			unset(inc_dir)
		endif()
		
		if(${i}_DEFINITIONS)
			add_definitions(${${i}_DEFINITIONS})
			list(APPEND APPLY_FIND_DEFINITIONS ${${i}_DEFINITIONS}) 
		endif()

		list(APPEND APPLY_FIND_LIBRARIES ${${i}_LIBRARIES})
	endif()
endforeach()

endmacro()
