cmake_minimum_required(VERSION 3.5)

project(zlib)

set(VERSION "1.2.8")

option(ASM686 "Enable building i686 assembly implementation")
option(AMD64 "Enable building amd64 assembly implementation")
option(OPT_BUILD_APPS "Enable building example applications" OFF)

set(INSTALL_BIN_DIR "${CMAKE_INSTALL_PREFIX}/bin" CACHE PATH "Installation directory for executables")
set(INSTALL_LIB_DIR "${CMAKE_INSTALL_PREFIX}/lib" CACHE PATH "Installation directory for libraries")
set(INSTALL_INC_DIR "${CMAKE_INSTALL_PREFIX}/include" CACHE PATH "Installation directory for headers")
set(INSTALL_MAN_DIR "${CMAKE_INSTALL_PREFIX}/share/man" CACHE PATH "Installation directory for manual pages")
set(INSTALL_PKGCONFIG_DIR "${CMAKE_INSTALL_PREFIX}/share/pkgconfig" CACHE PATH "Installation directory for pkgconfig (.pc) files")

if (WINCE)
	find_package(wcecompat REQUIRED)
	include_directories(${WCECOMPAT_INCLUDE_DIRS})
endif()

include(CheckTypeSize)
include(CheckFunctionExists)
include(CheckIncludeFile)
include(CheckCSourceCompiles)

if(CMAKE_C_COMPILER_VERSION)
    if(NOT "5.1.0" VERSION_LESS ${CMAKE_C_COMPILER_VERSION} AND ${CMAKE_VERSION} VERSION_LESS "2.8.12.2" )
    	set(DO_CHECKS 1)
    endif()
endif()

if(DO_TEST)
enable_testing()
check_include_file(sys/types.h HAVE_SYS_TYPES_H)
check_include_file(stdint.h    HAVE_STDINT_H)
check_include_file(stddef.h    HAVE_STDDEF_H)
endif()
#
# Check to see if we have large file support
#
set(CMAKE_REQUIRED_DEFINITIONS -D_LARGEFILE64_SOURCE=1)
# We add these other definitions here because CheckTypeSize.cmake
# in CMake 2.4.x does not automatically do so and we want
# compatibility with CMake 2.4.x.
if(HAVE_SYS_TYPES_H)
    list(APPEND CMAKE_REQUIRED_DEFINITIONS -DHAVE_SYS_TYPES_H)
endif()
if(HAVE_STDINT_H)
    list(APPEND CMAKE_REQUIRED_DEFINITIONS -DHAVE_STDINT_H)
endif()
if(HAVE_STDDEF_H)
    list(APPEND CMAKE_REQUIRED_DEFINITIONS -DHAVE_STDDEF_H)
endif()
if(DO_TEST)
check_type_size(off64_t OFF64_T)
endif()
if(HAVE_OFF64_T)
   add_definitions(-D_LARGEFILE64_SOURCE=1)
endif()
set(CMAKE_REQUIRED_DEFINITIONS) # clear variable

#
# Check for fseeko
#
if(DO_TEST)
	check_function_exists(fseeko HAVE_FSEEKO)
	if(NOT HAVE_FSEEKO)
		add_definitions(-DNO_FSEEKO)
	endif()

	#
	# Check for unistd.h
	#
	check_include_file(unistd.h Z_HAVE_UNISTD_H)
endif()
if (IOS OR EMSCRIPTEN)
	set(Z_HAVE_UNISTD_H 1)
endif()

set(CMAKE_DEBUG_POSTFIX "_d")
if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
    add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR})
endif()

configure_file(	${CMAKE_CURRENT_SOURCE_DIR}/zconf.h.cmakein
		${CMAKE_CURRENT_BINARY_DIR}/zconf.h @ONLY)
include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_SOURCE_DIR})


#============================================================================
# zlib
#============================================================================

set(ZLIB_PUBLIC_HDRS
    ${CMAKE_CURRENT_BINARY_DIR}/zconf.h
    zlib.h
)
set(ZLIB_PRIVATE_HDRS
    crc32.h
    deflate.h
    gzguts.h
    inffast.h
    inffixed.h
    inflate.h
    inftrees.h
    trees.h
    zutil.h
)
set(ZLIB_SRCS
    adler32.c
    compress.c
    crc32.c
    deflate.c
    gzclose.c
    gzlib.c
    gzread.c
    gzwrite.c
    inflate.c
    infback.c
    inftrees.c
    inffast.c
    trees.c
    uncompr.c
    zutil.c
)

if(NOT MINGW)
    set(ZLIB_DLL_SRCS
        win32/zlib1.rc # If present will override custom build rule below.
    )
endif()

if(CMAKE_COMPILER_IS_GNUCC)
    if(ASM686)
        set(ZLIB_ASMS contrib/asm686/match.S)
    elseif (AMD64)
        set(ZLIB_ASMS contrib/amd64/amd64-match.S)
    endif ()

	if(ZLIB_ASMS)
		add_definitions(-DASMV)
		set_source_files_properties(${ZLIB_ASMS} PROPERTIES LANGUAGE C COMPILE_FLAGS -DNO_UNDERLINE)
	endif()
endif()

if(MSVC)
    if(ASM686)
		ENABLE_LANGUAGE(ASM_MASM)
        set(ZLIB_ASMS
			contrib/masmx86/inffas32.asm
			contrib/masmx86/match686.asm
		)
    elseif (AMD64)
		ENABLE_LANGUAGE(ASM_MASM)
        set(ZLIB_ASMS
			contrib/masmx64/gvmat64.asm
			contrib/masmx64/inffasx64.asm
		)
    endif()

	if(ZLIB_ASMS)
		add_definitions(-DASMV -DASMINF)
	endif()
endif()

# parse the full version number from zlib.h and include in ZLIB_FULL_VERSION
file(READ ${CMAKE_CURRENT_SOURCE_DIR}/zlib.h _zlib_h_contents)
string(REGEX REPLACE ".*#define[ \t]+ZLIB_VERSION[ \t]+\"([-0-9A-Za-z.]+)\".*"
    "\\1" ZLIB_FULL_VERSION ${_zlib_h_contents})

if(BUILD_SHARED_LIBS)
    set(ZLIB_NAME zlib-${ZLIB_FULL_VERSION}-shared)
    set(ZLIB_DEFINITIONS "-DZLIB_DLL")
    set(ZLIB_COMPILE_DEFINITIONS "ZLIB_DLL")
else()
    set(ZLIB_NAME zlib-${ZLIB_FULL_VERSION}-static)
    set(ZLIB_DEFINITIONS "")
    set(ZLIB_COMPILE_DEFINITIONS "")
endif()

if(NOT BUILD_SHARED_LIBS)
    unset(ZLIB_DLL_SRCS)
endif()

add_library(${ZLIB_NAME} ${ZLIB_SRCS} ${ZLIB_ASMS} ${ZLIB_DLL_SRCS} ${ZLIB_PUBLIC_HDRS} ${ZLIB_PRIVATE_HDRS})
if (WINCE)
	target_link_libraries(${ZLIB_NAME} ${WCECOMPAT_LIBRARIES})
endif()

if(BUILD_SHARED_LIBS)
    set_target_properties(${ZLIB_NAME} PROPERTIES
        DEFINE_SYMBOL ZLIB_DLL
        SOVERSION 1)
endif()

set_target_properties(${ZLIB_NAME} PROPERTIES
    VERSION ${ZLIB_FULL_VERSION})

if(NOT SKIP_INSTALL_LIBRARIES AND NOT SKIP_INSTALL_ALL )
    install(TARGETS ${ZLIB_NAME}
        RUNTIME DESTINATION "${INSTALL_BIN_DIR}"
        ARCHIVE DESTINATION "${INSTALL_LIB_DIR}"
        LIBRARY DESTINATION "${INSTALL_LIB_DIR}" )
endif()
if(NOT SKIP_INSTALL_HEADERS AND NOT SKIP_INSTALL_ALL )
    install(FILES ${ZLIB_PUBLIC_HDRS} DESTINATION "${INSTALL_INC_DIR}")
endif()
if(NOT SKIP_INSTALL_FILES AND NOT SKIP_INSTALL_ALL )
    install(FILES zlib.3 DESTINATION "${INSTALL_MAN_DIR}/man3")
endif()
if(NOT SKIP_INSTALL_FILES AND NOT SKIP_INSTALL_ALL )
    configure_file(ZLIBConfig.cmake.in ZLIBConfig.cmake @ONLY)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/ZLIBConfig.cmake DESTINATION ${CMAKE_INSTALL_PREFIX})
endif()

#============================================================================
# Example binaries
#============================================================================

if(OPT_BUILD_APPS)
    add_executable(example test/example.c)
    target_link_libraries(example ${ZLIB_NAME})
    add_test(example example)

    add_executable(minigzip test/minigzip.c)
    target_link_libraries(minigzip ${ZLIB_NAME})

    set_property(TARGET example minigzip APPEND PROPERTY
        COMPILE_DEFINITIONS ${ZLIB_COMPILE_DEFINITIONS})

    if(HAVE_OFF64_T)
        add_executable(example64 test/example.c)
        target_link_libraries(example64 ${ZLIB_NAME})
        set_target_properties(example64 PROPERTIES COMPILE_FLAGS "-D_FILE_OFFSET_BITS=64")
        add_test(example64 example64)

        add_executable(minigzip64 test/minigzip.c)
        target_link_libraries(minigzip64 ${ZLIB_NAME})
        set_target_properties(minigzip64 PROPERTIES COMPILE_FLAGS "-D_FILE_OFFSET_BITS=64")

        set_property(TARGET example64 minigzip64 APPEND PROPERTY
            COMPILE_DEFINITIONS ${ZLIB_COMPILE_DEFINITIONS})
    endif()
endif(OPT_BUILD_APPS)
