function(gp_resolved_file_type_override resolved_file type)
    string(TOLOWER "${resolved_file}" lower)
    if(lower MATCHES ".*(api|ext)-ms.*\\.dll")
        set("${type}" "system" PARENT_SCOPE)
    endif()
    if (lower MATCHES "^c:/windows/.*\\.dll")
        set("${type}" "system" PARENT_SCOPE)
    endif()
    if (lower MATCHES "d3d12.dll")
        set("${type}" "system" PARENT_SCOPE)
    endif()    
    if (lower MATCHES "vulkan-1.dll")
        set("${type}" "system" PARENT_SCOPE)
    endif()
    if (lower MATCHES "libvulkan.*.so")
        set("${type}" "system" PARENT_SCOPE)
    endif()
endfunction()

function(gp_resolve_item_override context item exepath dirs resolved_item resolved)
    string(TOLOWER "${item}" lower)
    if(lower MATCHES ".*(api|ext)-ms.*\\.dll")
        set("${resolved_item}" "C:/please_dont_resolve_this/${item}" PARENT_SCOPE)
        set("${resolved}" 1 PARENT_SCOPE)
    endif()
endfunction()



include(BundleUtilities)



if(APPLE)
    get_dotapp_dir(${SRC} APP)
    set(SRC ${APP})
endif()
if("${OPT_CONFIG}" STREQUAL "Debug")
   set(CMAKE_INSTALL_DEBUG_LIBRARIES true)
endif()
if("${DST}" STREQUAL "")
    fixup_bundle("${SRC}" "${LIBS}" "${DIRS}")
else()
    file(MAKE_DIRECTORY "${DST}")
    file(COPY "${SRC}" DESTINATION "${DST}/")
    get_filename_component(FILE_NAME "${SRC}" NAME)

    if(NOT "$ENV{PLATFORM}" MATCHES "qnx-x86_64" AND
       NOT "$ENV{PLATFORM}" MATCHES "qnx-aarch64" AND
       NOT "$ENV{PLATFORM}" MATCHES "qnx-armv7")
    fixup_bundle("${DST}/${FILE_NAME}" "${LIBS}" "${DIRS}")
    endif()
endif()
