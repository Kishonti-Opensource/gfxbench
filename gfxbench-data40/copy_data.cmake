cmake_minimum_required(VERSION 3.5)

if (NOT TFW_PACKAGE_DIR)
    message(FATAL_ERROR "TFW_PACKAGE_DIR not set")
endif()

include(VersionParse)
version_parse(BENCHMARK "${PRODUCT_VERSION}")

set(BENCHMARK_VERSION_MAJOR_MINOR "${BENCHMARK_VERSION_MAJOR}${BENCHMARK_VERSION_MINOR}")

if( OPT_COMMUNITY_BUILD AND (NOT "${PRODUCT_VERSION}" VERSION_LESS "4.0.0") )
        execute_process(COMMAND rm -rf ${CMAKE_CURRENT_LIST_DIR}/data/gfx/car_chase/images_ETC2)
endif()

if("$ENV{BUNDLE_DATA}")
    if ("${PRODUCT_ID}" STREQUAL "gfxbench_dx")
        set(TESTS "fill;trex;manhattan;common")
    elseif ("${PRODUCT_ID}" STREQUAL "gfxbench_metal")
        if (DEFINED ENV{GFX_TESTS} AND "$ENV{GFX_TESTS}" STREQUAL "all")
            set(TESTS "fill2;alu2;manhattan31;fill;trex;manhattan;common")
        else()
        	if(${BENCHMARK_VERSION_MAJOR_MINOR} STREQUAL "30" )
                set(TESTS "fill;trex;manhattan;common")
            elseif(${BENCHMARK_VERSION_MAJOR_MINOR} STREQUAL "31" )
                set(TESTS "fill2;alu2;trex;manhattan;manhattan31;common")
            elseif(${BENCHMARK_VERSION_MAJOR_MINOR} STREQUAL "40" OR ${BENCHMARK_VERSION_MAJOR} STREQUAL "5" )
                set(TESTS "car_chase;tess;fill2;fill;alu2;trex;manhattan;manhattan31;common")
            endif()
        endif()
    else() #gl, vulkan:
        if (DEFINED ENV{GFX_TESTS} AND "$ENV{GFX_TESTS}" STREQUAL "all")
            set(TESTS "trex;fill;fill2;alu2;trex;manhattan;manhattan31;car_chase;tess;common")
        else()

            if(${BENCHMARK_VERSION_MAJOR_MINOR} STREQUAL "30" )
                set(TESTS "fill;trex;manhattan;common")
            elseif(${BENCHMARK_VERSION_MAJOR_MINOR} STREQUAL "31" )
                set(TESTS "fill2;alu2;trex;manhattan;manhattan31;common")
            elseif(${BENCHMARK_VERSION_MAJOR_MINOR} STREQUAL "40" OR ${BENCHMARK_VERSION_MAJOR} STREQUAL "5")
                set(TESTS "car_chase;tess;common;egypt;fill;fill2;alu2;trex;manhattan;manhattan31")
            endif()

        endif()
    endif()

    if(ANDROID AND OPT_COMMUNITY_BUILD)
        # On Android we want to include the ES2 tests in the Store APK, so the app will sync less data.
        # We have to be carefull with the APK size. It must be under 50MB in the Play Store.
        set(TESTS "fill;trex;common")
    endif()

    if(DEFINED ENV{GFX_FORMATS} AND "$ENV{GFX_FORMATS}" STREQUAL "all")
        set(FORMATS "888;ETC1;ETC2;PVRTC4;ASTC;DXT1")

    elseif(IOS)
        set(FORMATS "ETC2;PVRTC4")

    elseif(ANDROID OR OPT_USE_GLES)
        set(FORMATS "ETC2;ETC1")

    elseif(OPT_WINSTORE)
        set(FORMATS "DXT1")

    elseif(WIN32 OR MACOSX OR UNIX)
        set(FORMATS "PVRTC4;ETC1;ETC2;DXT1")#add ETC2 because of emulator

    endif()

    set(ALL_FORMATS "888;ETC1;ETC2;PVRTC4;ASTC;DXT1")
    if(${BENCHMARK_VERSION_MAJOR_MINOR} STREQUAL "31" )
        if(ANDROID)
            if(NOT OPT_COMMUNITY_BUILD)
                set(FORMATS "${FORMATS};ASTC")
            endif()
        elseif(IOS)
        	if(NOT OPT_COMMUNITY_BUILD)
                set(FORMATS "${FORMATS};ASTC")
            endif()
        else()
            set(FORMATS "${FORMATS};DXT5")
        endif()
    elseif(${BENCHMARK_VERSION_MAJOR_MINOR} STREQUAL "40" OR ${BENCHMARK_VERSION_MAJOR} STREQUAL "5" )
        if(ANDROID)
            if(NOT OPT_COMMUNITY_BUILD)
                set(FORMATS "${FORMATS};ASTC")
            endif()
        elseif(IOS)
        	if(NOT OPT_COMMUNITY_BUILD)
                set(FORMATS "${FORMATS};ASTC")
            endif()
        elseif(QNX)
            if(NOT OPT_COMMUNITY_BUILD)
                set(FORMATS "${FORMATS};ASTC")
            endif()
        else()
            set(FORMATS "${FORMATS};DXT5;ASTC")
        endif()
    endif()

    list(REMOVE_ITEM ALL_FORMATS ${FORMATS})
    foreach(format ${ALL_FORMATS})
        set(EXCLUDES ${EXCLUDES} PATTERN images_${format} EXCLUDE)
    endforeach()

    if (NOT TESTS)
        message(FATAL_ERROR "No tests selected for BENCHMARK_VERSION: ${BENCHMARK_VERSION_MAJOR_MINOR}")
    endif()

    foreach(test ${TESTS})
        file(COPY "${CMAKE_CURRENT_LIST_DIR}/data/gfx/${test}"
            NO_SOURCE_PERMISSIONS
            DESTINATION ${TFW_PACKAGE_DIR}/data/gfx
            ${EXCLUDES})
    endforeach()

    # ETC1 is always needed for qmatch
    list(FIND TESTS trex index)
    if (${index} GREATER -1)
        file(COPY "${CMAKE_CURRENT_LIST_DIR}/data/gfx/trex/images_ETC1"
            NO_SOURCE_PERMISSIONS
            DESTINATION ${TFW_PACKAGE_DIR}/data/gfx/trex)
    endif()
else()
	message(STATUS "message(\"ENV{BUNDLE_DATA} was not set, no need to install.\")")
endif()
