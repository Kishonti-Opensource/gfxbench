# install one target with IOS fix
macro(install_target_fixed TARGET DESTINATION CONFIG)
	if (IOS AND ${CMAKE_GENERATOR} MATCHES "Xcode")
		# cmake bug: INSTALL(TARGETS...) doesn't work for ios with xcode
		# 
		# http://public.kitware.com/Bug/bug_relationship_graph.php?bug_id=12506&graph=dependency
		#
		get_property(target_location TARGET ${TARGET} PROPERTY LOCATION_${CONFIG})
		string(REGEX REPLACE "\\$\\(([^)]*)\\)" "\$ENV{\\1}" target_location_ios_xcode_install ${target_location})
		install (FILES ${target_location_ios_xcode_install} CONFIGURATIONS ${CONFIG}${CMAKE_XCODE_EFFECTIVE_PLATFORMS} DESTINATION ${DESTINATION})
		
		# should be set for each target. would be nice if we could set this in the toolchain file
		# we call install_target for all targets so we set all xcode properties here
		set(IPHONEOS_DEPLOYMENT_TARGET "8.0")
		message(STATUS "${TARGET_NAME}: IPHONEOS_DEPLOYMENT_TARGET set to ${IPHONEOS_DEPLOYMENT_TARGET}")
		set_xcode_property(${TARGET} IPHONEOS_DEPLOYMENT_TARGET ${IPHONEOS_DEPLOYMENT_TARGET})
	else()
		install(TARGETS ${TARGET} DESTINATION ${DESTINATION} CONFIGURATIONS ${CONFIG})
	endif()
endmacro()
