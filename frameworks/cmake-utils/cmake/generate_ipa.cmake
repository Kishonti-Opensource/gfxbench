set(GENERATE_IPA_DIR ${CMAKE_CURRENT_LIST_DIR})

function(generate_ipa target)
	get_target_property(IS_BUNDLE_APP ${target} MACOSX_BUNDLE)
	if (NOT IS_BUNDLE_APP)
		message(FATAL_ERROR "${target} is not an application bundle")
	endif()

	set(ARCHIVE_ROOT "$ENV{ARCHIVE_ROOT}")
    if(NOT ARCHIVE_ROOT)
    	set(ARCHIVE_ROOT "${CMAKE_CURRENT_LIST_DIR}")
    endif()

	add_custom_target(${target}_ipa
		COMMAND ${CMAKE_COMMAND}
			-DGENERATE_IPA_APP_PATH:PATH=$<TARGET_FILE:${target}>
         	-DGENERATE_IPA_OUTPUT_DIR:PATH=${ARCHIVE_ROOT}
			-P ${GENERATE_IPA_DIR}/generate_ipa_worker.cmake
	)
	if (NOT TARGET ipa)
		add_custom_target(ipa DEPENDS ${target}_ipa)
	endif()
endfunction()
