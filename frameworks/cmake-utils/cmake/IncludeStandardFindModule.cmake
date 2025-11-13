# Include_standard_find_module includes a find module
# which is installed in the cmake root directory
#
#     include_standard_find_module()
#
# The macro must be called from a find module.
# It will include the find module of the same name from the cmake
# module directory ${CMAKE_ROOT}/Modules

macro(include_standard_find_module)
	get_filename_component(FPF_FILENAME ${CMAKE_CURRENT_LIST_FILE} NAME)
	include("${CMAKE_ROOT}/Modules/${FPF_FILENAME}")
endmacro()
