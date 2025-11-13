macro(install_target TARGET_NAME)

	get_target_property(thepostfix ${TARGET_NAME} DEBUG_POSTFIX)
	if(NOT ${thepostfix})
		set_target_properties(${TARGET_NAME} PROPERTIES DEBUG_POSTFIX _d)
	endif()
	if (IOS AND ${CMAKE_GENERATOR} MATCHES "Xcode")
		# cmake bug: INSTALL(TARGETS...) doesn't work for ios with xcode
		# 
		# http://public.kitware.com/Bug/bug_relationship_graph.php?bug_id=12506&graph=dependency
		#
		get_property(target_location_debug TARGET ${TARGET_NAME} PROPERTY LOCATION_DEBUG)
		get_property(target_location_release TARGET ${TARGET_NAME} PROPERTY LOCATION_RELEASE)
		string(REGEX REPLACE "\\$\\(([^)]*)\\)" "\$ENV{\\1}" target_location_ios_xcode_install_debug ${target_location_debug})
		string(REGEX REPLACE "\\$\\(([^)]*)\\)" "\$ENV{\\1}" target_location_ios_xcode_install_release ${target_location_release})
		install (FILES ${target_location_ios_xcode_install_debug} CONFIGURATIONS Debug${CMAKE_XCODE_EFFECTIVE_PLATFORMS} DESTINATION lib)
		install (FILES ${target_location_ios_xcode_install_release} CONFIGURATIONS Release${CMAKE_XCODE_EFFECTIVE_PLATFORMS} DESTINATION lib)
	else()
		install(TARGETS ${TARGET_NAME} DESTINATION lib)
	endif()
endmacro()
