# Function embed_file()
# 	Create C++ source files from the given source files. 
#
# Example:
#   include(EmbedFile)
#   embed_file(files ngdata data/file1.txt data/file2.txt)
#   add_executable(test main.cpp ${files_SOURCES})
# 
# Usage:
#   embed_file(
# 	  target_name
# 	  namespace
# 	  source_file1 source_file2 ...
#   )
#
# Variables set:
#   ${target_name}_SOURCES

set(EMBED_FILE_TEMPLATE_DIR "${CMAKE_CURRENT_LIST_DIR}/private/embedfile" CACHE INTERNAL "Directory containing template files")
 
function(embed_file target_name namespace)
	set(target_dir ${CMAKE_CURRENT_BINARY_DIR})
	foreach(source_path ${ARGN})
		get_filename_component(source_name "${source_path}" NAME_WE)
		file(READ "${source_path}" source_content)
		string(REPLACE "\"" "\\\"" source_content "${source_content}")
		string(REPLACE "\n" "\\n\"\n\"" source_content "${source_content}")
		set(target_path "${target_dir}/${source_name}.cpp")
		list(APPEND ${target_name}_SOURCES "${target_path}")
		configure_file("${EMBED_FILE_TEMPLATE_DIR}/embed.cpp.in" "${target_path}" @ONLY)
		set(embed_decl "${embed_decl}\n\textern const char *${source_name};")
	endforeach()
	configure_file("${EMBED_FILE_TEMPLATE_DIR}/embed.h.in" "${target_dir}/${target_name}.h" @ONLY ESCAPE_QUOTES)
	list(APPEND ${target_name}_SOURCES "${target_dir}/${target_name}.h")
	set(${target_name}_SOURCES "${${target_name}_SOURCES}" PARENT_SCOPE)
	include_directories(${CMAKE_CURRENT_BINARY_DIR})
	source_group("Embedded Files" FILES ${${target_name}_SOURCES})
endfunction()
