cmake_minimum_required(VERSION 3.5)
project(gfxbench_data)

if (NOT TFW_PACKAGE_DIR)
	message(FATAL_ERROR "TFW_PACKAGE_DIR not set")
endif()

include(VersionParse)
version_parse(BENCHMARK "${PRODUCT_VERSION}")

if("$ENV{BUNDLE_DATA}")

	if("${PRODUCT_ID}" STREQUAL "gfxbench_ngl")
		set(TESTS "ngl")
	elseif("${PRODUCT_ID}" STREQUAL "gfxbench_vr")
		set(TESTS "scene5_ovr")
		if( ${PLATFORM} MATCHES "android" )
			set(ALL_FORMATS DXT5 )
		else()
			set(ALL_FORMATS ETC2)
		endif()
	else()
		if("${BENCHMARK_VERSION_MAJOR}${BENCHMARK_VERSION_MINOR}" STREQUAL "41")
			set(TESTS "scene41")
		endif()
		if(${BENCHMARK_VERSION_MAJOR} STREQUAL "5")
			set(TESTS "scene5_normal" "scene5_high")
			if( ${PLATFORM} MATCHES "android" )
				set(ALL_FORMATS DXT5 888)
			else()
				set(ALL_FORMATS 888)
			endif()
		endif()
	endif()

	if ("${PRODUCT_ID}" STREQUAL "gfxbench_metal")

	    set(ALL_FORMATS 888 888_fast)
        if(APPLE AND NOT IOS)
            set(ALL_FORMATS "ETC2;ETC2_fast;${ALL_FORMATS}")
        endif()

	elseif ("${PRODUCT_ID}" STREQUAL "gfxbench_dx")

		set(ALL_FORMATS ASTC ETC2)

	elseif ("${PRODUCT_ID}" STREQUAL "gfxbench_vulkan" AND ${PLATFORM} MATCHES "android" )

	endif()

	foreach(format ${ALL_FORMATS})
		set(EXCLUDES ${EXCLUDES} PATTERN images_${format} EXCLUDE)
	endforeach()

	foreach(test ${TESTS})
		file(COPY "${CMAKE_CURRENT_LIST_DIR}/data/gfx/${test}"
			NO_SOURCE_PERMISSIONS
			DESTINATION ${TFW_PACKAGE_DIR}/data/gfx
			${EXCLUDES})
	endforeach()

else()
	message(STATUS "message(\"ENV{BUNDLE_DATA} was not set, no need to install.\")")
endif()


install(CODE "message(\"Installation complete.\")")
