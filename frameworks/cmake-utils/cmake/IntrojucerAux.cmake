# Function to help with Introjucer + CMake integration
#
# This file defines the following public function:
#
#     introjucer_package_helper(ARGS)
#
# where ARGS is a list of package names. Example:
#
#     introjucer_package_helper(ZLIB Boost GDAL)
#
# which collects the information from the variables
#
# - <package>_INCLUDE_DIRS, <package>_INCLUDE_DIR
# - <package>_DEFINTIONS 
# - <package>_LIBRARIES
#
# and reports it reformatted to enter it in the
# Introjucer project configuration settings.
#
# As Introjucer does not support separate list
# of debug and release libraries, the function does
# some preprocessing:
#
# - moves debug, release and general libraries to
#   appropriate directories (in the cmake binary dir)
# - renames general libraries to ensure uniqueness
# - renames debug and release libraries to ensure same
#   name for each release and debug lib


function(select_grd_libraries libraries gg rr dd)
	unset(g)
	unset(r)
	unset(d)
	foreach(f ${${libraries}})
		if (${f} MATCHES "general")
			set(state "undef")
		elseif(${f} MATCHES "debug")
			set(state "debug")
		elseif(${f} MATCHES "optimized")
			set(state "optimized")
		else()
			if (${state} MATCHES "debug")
				list(APPEND d "${f}")
			elseif (${state} MATCHES "optimized")
				list(APPEND r "${f}")
			else()
				list(APPEND g "${f}")
			endif()
			unset(state)
		endif()
	endforeach()
	if(g)
		list(REMOVE_DUPLICATES g)
	endif()
	if(r)
		list(REMOVE_DUPLICATES r)
	endif()
	if(d)
		list(REMOVE_DUPLICATES d)
	endif()
	set(${gg} "${g}" PARENT_SCOPE)
	set(${rr} "${r}" PARENT_SCOPE)
	set(${dd} "${d}" PARENT_SCOPE)
endfunction()

function(introjucer_package_helper)

foreach(i ${ARGV})
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
		list(APPEND all_include_dirs "${${i}_INCLUDE_DIRS}")
	elseif (${i}_INCLUDE_DIR)
		list(APPEND all_include_dirs "${${i}_INCLUDE_DIR}")
	endif()
	
	if(${i}_DEFINITIONS)
		foreach(j ${${i}_DEFINITIONS})
			string(SUBSTRING ${j} 2 -1 s)
			list(APPEND all_definitions "${s}")
		endforeach()
	endif()

	if(${i}_LIBRARIES)
		list(APPEND all_libraries "${${i}_LIBRARIES}")
	endif()
endforeach()

select_grd_libraries(all_libraries g r d)

if(g)
	list(SORT g)
endif()
if(r)
	list(SORT r)
endif()
if(d)
	list(SORT d)
endif()

if(all_definitions)
	list(REMOVE_DUPLICATES all_definitions)
endif()
if(all_include_dirs)
	list(REMOVE_DUPLICATES all_include_dirs)
endif()


set(bg "${CMAKE_BINARY_DIR}/ext_libs_general")
set(bd "${CMAKE_BINARY_DIR}/ext_libs_debug")
set(br "${CMAKE_BINARY_DIR}/ext_libs_release")
file(REMOVE_RECURSE "${bg}")
file(REMOVE_RECURSE "${bd}")
file(REMOVE_RECURSE "${br}")
file(MAKE_DIRECTORY "${bg}")
file(MAKE_DIRECTORY "${bd}")
file(MAKE_DIRECTORY "${br}")

unset(all_library_files)
set(counter 0)

list(LENGTH g gsize)
list(LENGTH r rsize)
list(LENGTH d dsize)
if(NOT(${rsize} EQUAL ${dsize}))
	message(FATAL_ERROR "The number of debug and release libraries differ")
endif()

if(${rsize} GREATER 0)
	math(EXPR rsize_minus_1 "${rsize}-1")
	foreach(counter RANGE 0 ${rsize_minus_1})
		list(GET r ${counter} ritem)
		list(GET d ${counter} ditem)
		file(COPY ${ditem} DESTINATION "${bd}")
		file(COPY ${ritem} DESTINATION "${br}")
		get_filename_component(dn ${ditem} NAME)
		get_filename_component(dnwe ${ditem} NAME_WE)
		get_filename_component(de ${ditem} EXT)
		get_filename_component(rn ${ritem} NAME)
		get_filename_component(rnwe ${ritem} NAME_WE)
		get_filename_component(re ${ritem} EXT)
		if(NOT(${de} STREQUAL ${re}))
			message(FATAL_ERROR "Tried to match libs ${dn} and ${rn} but the extension differs")
		endif()
		set(newfilename "${counter}_${rnwe}_or_${dnwe}${re}")
		list(APPEND all_library_files "${newfilename}")
		file(RENAME "${bd}/${dn}" "${bd}/${newfilename}")
		file(RENAME "${br}/${rn}" "${br}/${newfilename}")
	endforeach()
endif()

set(counter ${rsize})
foreach(i ${g})
	file(COPY ${i} DESTINATION "${bg}")
	get_filename_component(n ${i} NAME)
	get_filename_component(e ${i} EXT)
	set(newfilename "${counter}_${n}")
	list(APPEND all_library_files "${newfilename}")
	math(EXPR counter "${counter}+1")
	file(RENAME "${bg}/${n}" "${bg}/${newfilename}")
endforeach()


message("")

message("** Extra preprocessor Definitions **")
foreach(i ${all_definitions})
	message(${i})
endforeach()

message("")

message("** External Libraries To Link **")
foreach(i ${all_library_files})
	message(${i})
endforeach()

message("")

message("** Header Search Paths (Debug and Release) **")
foreach(i ${all_include_dirs})
	message(${i})
endforeach()

message("")

message("** Extra Library Search Paths (Debug) **")
if(${gsize} GREATER 0)
	message("${bg}")
endif()
if(${dsize} GREATER 0)
	message("${bd}")
endif()

message("")

message("** Extra Library Search Paths (Release) **")
if(${gsize} GREATER 0)
	message("${bg}")
endif()
if(${rsize} GREATER 0)
	message("${br}")
endif()

message("")

endfunction()
