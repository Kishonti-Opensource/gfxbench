# Renames common variables set by former find_package calls
#
# Example:
#   wrap_variables(CURL NG)
# result:
#   CURL_INCLUDE_DIRS -> NGCURL_INCLUDE_DIRS
#   CURL_LIBRARIES    -> NGCURL_LIBRARIES
#   ...
#
function(print_var VAR)
	message(STATUS "${VAR}=${${VAR}}")
endfunction()

macro(wrap_variable NAME TAG PREFIX)
	set(${PREFIX}${NAME}_${TAG} ${${NAME}_${TAG}})
	#print_var(${PREFIX}${NAME}_${TAG})
	unset(${NAME}_${TAG})
endmacro()

macro(wrap_variables NAME PREFIX)
	foreach(tag FOUND DEFINITIONS INCLUDE_DIRS LIBRARIES)
		wrap_variable(${NAME} ${tag} ${PREFIX})
	endforeach()
endmacro()
