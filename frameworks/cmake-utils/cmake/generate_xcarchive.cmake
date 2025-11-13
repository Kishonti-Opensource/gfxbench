set(GENERATE_XCARCHIVE_DIR ${CMAKE_CURRENT_LIST_DIR})

function(generate_xcarchive target)

	get_target_property(IS_BUNDLE_APP ${target} MACOSX_BUNDLE)
	if (NOT IS_BUNDLE_APP)
		message(FATAL_ERROR "${target} is not an application bundle")
	endif()

	set(ARCHIVE_ROOT "$ENV{ARCHIVE_ROOT}")
    if(NOT ARCHIVE_ROOT)
    	set(ARCHIVE_ROOT "${CMAKE_CURRENT_LIST_DIR}")
    endif()

	add_custom_target(${target}_xcarchive
		COMMAND ${CMAKE_COMMAND}
			-DGENERATE_XCARCHIVE_APP_PATH:PATH=$<TARGET_FILE:${target}>
         	-DGENERATE_XCARCHIVE_OUTPUT_DIR:PATH=${ARCHIVE_ROOT}
            -DARCHIVE_NAME:STRING=${ARCHIVE_NAME}
			-P ${GENERATE_XCARCHIVE_DIR}/generate_xcarchive_worker.cmake
	)
	if (NOT TARGET xcarchive)
		add_custom_target(xcarchive DEPENDS ${target}_xcarchive)
	endif()
endfunction()
