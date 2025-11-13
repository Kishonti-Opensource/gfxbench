cmake_minimum_required(VERSION 3.4)

include(VersionParse)
version_parse(BENCHMARK ${PRODUCT_VERSION})


function(build_gfx4_gl)
	set(PRODUCT_ID gfxbench_gl)
	set(PRODUCT_VERSION 4.0.0)
	add_subdirectory(../gfxbench40 gfxbench40_bindir)
endfunction()

function(build_gfx4_dx)
	set(PRODUCT_ID gfxbench_dx)
	set(PRODUCT_VERSION 3.0.0)
	add_subdirectory(../gfxbench40 gfxbench40_bindir_dx)
endfunction()

function(build_gfx4_metal)
	set(PRODUCT_ID gfxbench_metal)
	set(PRODUCT_VERSION 4.0.0)
	add_subdirectory(../gfxbench40 gfxbench40_bindir_metal)
endfunction()


if(NOT ${APPLICATION_TYPE} STREQUAL "gui")
	if(NOT ANDROID)
		add_definitions(-DDISABLE_OGLES_FUNCTIONS=1)
	endif()
endif()

if(NOT DISABLE_GFX4)
    if (NOT ${BENCHMARK_VERSION_MAJOR} LESS 4)
        if(PRODUCT_ID AND ${PRODUCT_ID} STREQUAL "gfxbench_gl" )
            build_gfx4_gl()
        elseif(PRODUCT_ID AND ${PRODUCT_ID} STREQUAL "gfxbench_vulkan")
            build_gfx4_gl()
        elseif(PRODUCT_ID AND ${PRODUCT_ID} STREQUAL "gfxbench_dx" AND WIN32)
            build_gfx4_dx()
        elseif(PRODUCT_ID AND ${PRODUCT_ID} STREQUAL "gfxbench_metal")
            build_gfx4_metal()
        elseif(PRODUCT_ID AND ${PRODUCT_ID} STREQUAL "gfxbench" )
            build_gfx4_gl()
            if (NOT ANDROID)
                build_gfx4_dx()
            endif()
        endif()
    else()
        add_subdirectory(../gfxbench40 gfxbench40_bindir)
    endif()
endif()


set(TMP_LIBS ${TESTFW_LIBRARIES})

if (NOT ${BENCHMARK_VERSION_MAJOR} LESS 5)
    set(TEST_MODULE scene5)
    add_subdirectory(../gfxbench50 gfxbench50_bindir) #TODO check build only DX
endif()


if(IOS)
	list(APPEND TESTFW_LIBRARIES ${TMP_LIBS})
	set(TESTFW_LIBRARIES "${TESTFW_LIBRARIES}" CACHE STRING "" FORCE)
endif()


if(TFW_PACKAGE_DIR)
	include(tfw_package.cmake)
endif()
