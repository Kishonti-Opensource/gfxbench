# CMakeLists.txt

# Copyright (C) 2007-2013 Glenn Randers-Pehrson

# This code is released under the libpng license.
# For conditions of distribution and use, see the disclaimer
# and license in png.h

cmake_minimum_required(VERSION 3.5)

project(libpng C)
enable_testing()

set(PNGLIB_MAJOR 1)
set(PNGLIB_MINOR 6)
set(PNGLIB_RELEASE 7)
set(PNGLIB_NAME libpng${PNGLIB_MAJOR}${PNGLIB_MINOR})
set(PNGLIB_VERSION ${PNGLIB_MAJOR}.${PNGLIB_MINOR}.${PNGLIB_RELEASE})

if(APPLE OR ANDROID)
  add_definitions(-DPNG_ARM_NEON_OPT=0)
endif()

# needed packages
find_package(ZLIB REQUIRED)
if(WINCE)
	find_package(wcecompat REQUIRED)
endif()
include_directories(${ZLIB_INCLUDE_DIRS} ${WCECOMPAT_INCLUDE_DIRS})

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
 set_property(GLOBAL PROPERTY FIND_LIBRARY_USE_LIB64_PATHS TRUE)
endif()
if(NOT WIN32)
  find_library(M_LIBRARY
    NAMES m
    PATHS /usr/lib /usr/local/lib
  )
  if(NOT M_LIBRARY)
    message(STATUS "math library 'libm' not found - using standard option: -lm")
    set(M_LIBRARY "-lm")
  else()
    set(PNG_M_LIBRARY_NEEDED 1)
  endif()
else()
  # not needed on windows
  set(M_LIBRARY "")
endif()

option(PNG_TESTS  "Build libpng tests" YES)

# Many more configuration options could be added here
option(PNG_DEBUG         "Build with debug output" NO)
option(PNGARG            "Disable ANSI-C prototypes" NO)

# SET LIBNAME
set(PNG_LIB_NAME png-${PNGLIB_MAJOR}.${PNGLIB_MINOR}.${PNGLIB_RELEASE})

if(BUILD_SHARED_LIBS)
  set(PNG_LIB_NAME ${PNG_LIB_NAME}-shared)
  set(PNG_DEFINITIONS -DPNG_USE_DLL)
  set(PNG_COMPILE_DEFINITIONS PNG_USE_DLL)
else()
  set(PNG_LIB_NAME ${PNG_LIB_NAME}-static)
  set(PNG_DEFINITIONS)
  set(PNG_COMPILE_DEFINITIONS)
endif()

# to distinguish between debug and release lib
set(CMAKE_DEBUG_POSTFIX "_d")

# Use the prebuilt pnglibconf.h file from the scripts folder
# TODO: fix this by building with awk; without this no cmake build can be
# configured directly (to do so indirectly use your local awk to build a
# pnglibconf.h in the build directory.)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/scripts/pnglibconf.h.prebuilt
               ${CMAKE_CURRENT_BINARY_DIR}/pnglibconf.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# OUR SOURCES
set(libpng_public_hdrs
  png.h
  pngconf.h
  ${CMAKE_CURRENT_BINARY_DIR}/pnglibconf.h
)
set(libpng_sources
  ${libpng_public_hdrs}
  pngdebug.h
  pnginfo.h
  pngpriv.h
  pngstruct.h
  png.c
  pngerror.c
  pngget.c
  pngmem.c
  pngpread.c
  pngread.c
  pngrio.c
  pngrtran.c
  pngrutil.c
  pngset.c
  pngtrans.c
  pngwio.c
  pngwrite.c
  pngwtran.c
  pngwutil.c
)
set(pngtest_sources
  pngtest.c
)
set(pngvalid_sources
  contrib/libtests/pngvalid.c
)
set(pngstest_sources
  contrib/libtests/pngstest.c
)
# SOME NEEDED DEFINITIONS

if(MSVC)
  add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
endif(MSVC)

if(PNG_DEBUG)
  add_definitions(-DPNG_DEBUG)
endif()

# NOW BUILD OUR TARGET
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${ZLIB_INCLUDE_DIRS})

add_library(${PNG_LIB_NAME} ${libpng_sources})
set(PNG_LINK_LIBRARIES ${ZLIB_LIBRARIES} ${M_LIBRARY})
target_link_libraries(${PNG_LIB_NAME} ${PNG_LINK_LIBRARIES})

if(PNG_SHARED AND WIN32)
  set_target_properties(${PNG_LIB_NAME} PROPERTIES DEFINE_SYMBOL PNG_BUILD_DLL)
endif()

if(PNG_TESTS AND NOT IOS AND NOT WINCE)
  if(NOT BUILD_SHARED_LIBS)
    set(EXE_LINK_LIBRARIES ${PNG_LINK_LIBRARIES})
  endif()

  add_executable(pngtest ${pngtest_sources})
  target_link_libraries(pngtest ${PNG_LIB_NAME} ${EXE_LINK_LIBRARIES})
  add_test(pngtest ./pngtest ${CMAKE_CURRENT_SOURCE_DIR}/pngtest.png)
  #
  add_executable(pngvalid ${pngvalid_sources})
  target_link_libraries(pngvalid ${PNG_LIB_NAME} ${EXE_LINK_LIBRARIES})
  add_test(pngvalid ./pngvalid)
  add_executable(pngstest ${pngstest_sources})
  target_link_libraries(pngstest ${PNG_LIB_NAME} ${EXE_LINK_LIBRARIES})
  add_test(pngstest ./pngstest
    ${CMAKE_CURRENT_SOURCE_DIR}/contrib/pngsuite/basn0g01.png
    ${CMAKE_CURRENT_SOURCE_DIR}/contrib/pngsuite/basn0g02.png
    ${CMAKE_CURRENT_SOURCE_DIR}/contrib/pngsuite/basn0g04.png
    ${CMAKE_CURRENT_SOURCE_DIR}/contrib/pngsuite/basn0g08.png
    ${CMAKE_CURRENT_SOURCE_DIR}/contrib/pngsuite/basn0g16.png
    ${CMAKE_CURRENT_SOURCE_DIR}/contrib/pngsuite/basn2c08.png
    ${CMAKE_CURRENT_SOURCE_DIR}/contrib/pngsuite/basn2c16.png
    ${CMAKE_CURRENT_SOURCE_DIR}/contrib/pngsuite/basn3p01.png
    ${CMAKE_CURRENT_SOURCE_DIR}/contrib/pngsuite/basn3p02.png
    ${CMAKE_CURRENT_SOURCE_DIR}/contrib/pngsuite/basn3p04.png
    ${CMAKE_CURRENT_SOURCE_DIR}/contrib/pngsuite/basn3p08.png
    ${CMAKE_CURRENT_SOURCE_DIR}/contrib/pngsuite/basn4a08.png
    ${CMAKE_CURRENT_SOURCE_DIR}/contrib/pngsuite/basn4a16.png
    ${CMAKE_CURRENT_SOURCE_DIR}/contrib/pngsuite/basn6a08.png
    ${CMAKE_CURRENT_SOURCE_DIR}/contrib/pngsuite/basn6a16.png
    ${CMAKE_CURRENT_SOURCE_DIR}/contrib/pngsuite/ftbbn0g01.png
    ${CMAKE_CURRENT_SOURCE_DIR}/contrib/pngsuite/ftbbn0g02.png
    ${CMAKE_CURRENT_SOURCE_DIR}/contrib/pngsuite/ftbbn0g04.png
    ${CMAKE_CURRENT_SOURCE_DIR}/contrib/pngsuite/ftbbn2c16.png
    ${CMAKE_CURRENT_SOURCE_DIR}/contrib/pngsuite/ftbbn3p08.png
    ${CMAKE_CURRENT_SOURCE_DIR}/contrib/pngsuite/ftbgn2c16.png
    ${CMAKE_CURRENT_SOURCE_DIR}/contrib/pngsuite/ftbgn3p08.png
    ${CMAKE_CURRENT_SOURCE_DIR}/contrib/pngsuite/ftbrn2c08.png
    ${CMAKE_CURRENT_SOURCE_DIR}/contrib/pngsuite/ftbwn0g16.png
    ${CMAKE_CURRENT_SOURCE_DIR}/contrib/pngsuite/ftbwn3p08.png
    ${CMAKE_CURRENT_SOURCE_DIR}/contrib/pngsuite/ftbyn3p08.png
    ${CMAKE_CURRENT_SOURCE_DIR}/contrib/pngsuite/ftp0n0g08.png
    ${CMAKE_CURRENT_SOURCE_DIR}/contrib/pngsuite/ftp0n2c08.png
    ${CMAKE_CURRENT_SOURCE_DIR}/contrib/pngsuite/ftp0n3p08.png
    ${CMAKE_CURRENT_SOURCE_DIR}/contrib/pngsuite/ftp1n3p08.png
  )
  set_property(
    TARGET pngtest pngvalid pngstest APPEND
    PROPERTY COMPILE_DEFINITIONS ${PNG_COMPILE_DEFINITIONS})
endif()

set_target_properties(${PNG_LIB_NAME} PROPERTIES
#   VERSION 16.${PNGLIB_RELEASE}.1.6.7
    VERSION 16.${PNGLIB_RELEASE}.0
    SOVERSION 16
  )

if(NOT SKIP_INSTALL_LIBRARIES AND NOT SKIP_INSTALL_ALL )
  install(TARGETS ${PNG_LIB_NAME}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib)
endif()

if(NOT SKIP_INSTALL_HEADERS AND NOT SKIP_INSTALL_ALL )
  install(FILES ${libpng_public_hdrs}   DESTINATION include)
endif()

if(NOT SKIP_INSTALL_FILES AND NOT SKIP_INSTALL_ALL )
  # Install man pages
  if(NOT PNG_MAN_DIR)
    set(PNG_MAN_DIR "share/man")
  endif()
  install(FILES libpng.3 libpngpf.3      DESTINATION ${PNG_MAN_DIR}/man3)
  install(FILES png.5                    DESTINATION ${PNG_MAN_DIR}/man5)
endif()

configure_file(PNGConfig.cmake.in PNGConfig.cmake @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/PNGConfig.cmake DESTINATION .)

# what's with libpng-$VER%.txt and all the extra files?

# UNINSTALL
# do we need this?

# DIST
# do we need this?

# to create msvc import lib for mingw compiled shared lib
# pexports libpng.dll > libpng.def
# lib /def:libpng.def /machine:x86

