macro(install_includes MODULE_NAME)
	install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/include/${MODULE_NAME} DESTINATION include/NaviGenie)
endmacro()