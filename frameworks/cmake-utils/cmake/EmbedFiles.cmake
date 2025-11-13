# Function embed_files()
# 	Create C++ source files from the given source files. 
#
# Examples:
#   include(EmbedFiles)
#   embed_files(files ngdata data/file1.txt data/file2.txt)
#   add_executable(test main.cpp ${files_SOURCES})
#   
#   embed_files(globogl_get_shader building.vs building.fs)
#   add_library(mytarget ... glbogl_get_shader.c)
#   
#   set(embedded_src ${CURRENT_BINARY_DIR}/something.c)
#   embed_files(globogl_get_shader OUTPUT ${embedded_src} RELATIVE ${CMAKE_SOURCE_DIR}/data FILES shaders/building.vs shaders/building.fs)
#   add_library(mytarget ... ${embedded_src})
#      
# Usage:
#  embed_files(
#    <name>
#    <file1> [ <file2> ... ]
#  )
#
#  embed_files(
#    <name>
#    [ RELATIVE <rel-path> ]
#    FILES <file1> [ <file2> ... ]
#    [ OUTPUT <out-file> ]
#  )
# 
# Parameters:
#  <name>: Name of the generated C++ function
#  OUTPUT <out-file>: Path to the generated C++ source
#    If <out-file> is a relative path then the source files are generated under ${CMAKE_CURRENT_BINARY_DIR}
#    Defaults to ${CMAKE_CURRENT_BINARY_DIR}/<name>.c
#  RELATIVE <rel-path>: files' paths will be stored relative to this path
# 	Defaults to ${CMAKE_CURRENT_SOURCE_DIR}
#  FILES file1...: Files included in the generated source file
#    Relative paths refer to ${CMAKE_CURRENT_SOURCE_DIR}
#    Absolute paths must not be used unless RELATIVE parameter is given.
# 
# Description
#  embed_files generates a C++ source and header file which contains the function <name> declared as:
#    const char* <name>(const char* path, int *length);
#  <name> returns the contents and length of the file named path or NULL if the path refers to a missing file.
#  The path will be relative to <rel-path>. Path separator must be a slash on all platforms.

include(CMakeParseArguments)

set(EMBED_FILE_TEMPLATE_DIR "${CMAKE_CURRENT_LIST_DIR}/private/embedfiles" CACHE INTERNAL "Directory containing template files")
 
function(embed_files function_name)
	set(options "")
	set(oneValueArgs OUTPUT RELATIVE)
	set(multiValueArgs FILES)
	cmake_parse_arguments(embed "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

	if(embed_OUTPUT OR embed_RELATIVE OR embed_FILES)
	#
	# Advanced mode
	#
		if(embed_UNPARSED_ARGUMENTS)
			message(FATAL_ERROR "Unexpected arguments: ${embed_UNPARSED_ARGUMENTS}")
		endif()
		foreach(file_name ${embed_FILES})
			if(NOT embed_RELATIVE AND IS_ABSOLUTE "${file_name}")
				message(FATAL_ERROR "File path is absoulte and no RELATIVE option given.")
			endif()
		endforeach()
		if(IS_ABSOLUTE "${embed_RELATIVE}")
			set(root_dir "${embed_RELATIVE}")
		else()
			set(root_dir "${CMAKE_CURRENT_SOURCE_DIR}/${embed_RELATIVE}")
		endif()
		foreach(file_name ${embed_FILES})
			list(APPEND files "${file_name}")
		endforeach()
		if(embed_OUTPUT)
			set(source_filename "${embed_OUTPUT}")
			if(NOT IS_ABSOLUTE "${source_filename}")
				set(source_filename "${CMAKE_CURRENT_BINARY_DIR}/${source_filename}")
			endif()
			get_filename_component(source_path "${source_filename}" PATH)
			get_filename_component(source_name_we "${source_filename}" NAME_WE)
			set(header_filename "${source_path}/${source_name_we}.h")
		else()
			set(path_with_name "${function_name}")
			set(source_filename "${path_with_name}.cpp")
			set(header_filename "${path_with_name}.h")
		endif()
	else()
	#
	# Simple mode
	#
		foreach(file_name ${embed_UNPARSED_ARGUMENTS})
			if(IS_ABSOLUTE "${file_name}")
				message(FATAL_ERROR "File path is absolute. Only relative pathes can be used in simple mode.")
			endif()
		endforeach()
		set(root_dir "${CMAKE_CURRENT_SOURCE_DIR}")
		foreach(file_name ${embed_UNPARSED_ARGUMENTS})
			list(APPEND files "${file_name}")
		endforeach()
		set(source_filename "${function_name}.cpp")
		set(header_filename "${function_name}.h")
	endif()
	if(NOT files)
		message(FATAL_ERROR "Missing file list.")
	endif()

	set(array_index 0)
	get_filename_component(header_name "${header_filename}" NAME_WE)
	string(TOUPPER "${header_name}_h_included" header_guard)
	foreach(file_name ${files})
		if(IS_ABSOLUTE "${file_name}")
			set(abs_file_name "${file_name}")
		else()
			set(abs_file_name "${CMAKE_CURRENT_SOURCE_DIR}/${file_name}")
		endif()
		if(NOT EXISTS "${abs_file_name}")
			message(FATAL_ERROR "File not found: ${abs_file_name}")
		endif()
		file(READ ${file_name} file_content HEX)
		string(LENGTH "${file_content}" length)
		math(EXPR length "${length}/2")
		string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1," hex_content "${file_content}")
		file(RELATIVE_PATH rel_file_name "${root_dir}" "${abs_file_name}")
		math(EXPR array_index "${array_index}+1")
		set(array_name "FILE${array_index}")
		set(arrays "${arrays}\n\tconst unsigned char ${array_name}[] = {${hex_content}0x00};")
		set(all_contents "${all_contents}\n\t\t{\"${rel_file_name}\", reinterpret_cast<const char*>(${array_name}), ${length}},") # Add NULL terminator, but don't modifiy length
	endforeach()

	configure_file("${EMBED_FILE_TEMPLATE_DIR}/embed_files.h.in" "${header_filename}")
	configure_file("${EMBED_FILE_TEMPLATE_DIR}/embed_files.cpp.in" "${source_filename}")
	source_group("Embedded Files" FILES "${header_filename}" "${source_filename}")
endfunction()
