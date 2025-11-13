# decide_which_modules.cmake
# - reads NGRTL_MODULES, translates it to NGRTL_ENABLE_<MODULE> variables
# - listed modules will set NGRTL_ENABLE_<MODULE> to ON, missing ones to FALSE

# So you can either list lower-case module names in NGRTL_MODULES variable
# or you can define the following variables:

# - if NGRTL_ENABLE_<MODULE> is ON (or any true value) forces building that module
# - if NGRTL_ENABLE_<MODULE> is OFF (or any false value including unset) skips building that module
# - if you don't change an NGRTL_ENABLE_<MODULE> variable that previous value from the cache will be effective
# 
# additional options:
# NGRTL_WITH_LMDB, NGRTL_WITH_LEVELDB, NGRTL_WITH_SQLITE3 (default is OFF)

set(default_modules core binfile pngio jpegxr i18n font gdal pdb legacy ogl oglfont)
set(default_win32_macosx_modules oracle sqlite navteq postgres dbm)
set(all_modules ${default_modules} ${default_win32_macosx_modules})

if(NOT "$ENV{NGRTL_MODULES}" STREQUAL "")
  set(NGRTL_MODULES $ENV{NGRTL_MODULES})
endif()

if(NGRTL_MODULES)
  # validate NGRTL_MODULES
  foreach(m ${NGRTL_MODULES})
    list(FIND all_modules "${m}" midx)
    if(midx EQUAL -1)
      message(FATAL_ERROR "NGRTL_MODULES contains module ${m} which is not a valid ngrtl module.")
    endif()
  endforeach()

  foreach(m ${all_modules})
    string(TOUPPER "${m}" M)
    if(DEFINED NGRTL_ENABLE_${M})
      message(FATAL_ERROR "Both NGRTL_MODULES and NGRTL_ENABLE_${M} are defined, define only one of them.")
    endif()
    list(FIND NGRTL_MODULES "${m}" midx)
    if(midx EQUAL -1)
      set(NGRTL_ENABLE_${M} OFF)
    else()
      set(NGRTL_ENABLE_${M} ON)
    endif()
  endforeach()
else() # no NGRTL_MODULES
  set(NGRTL_ENABLE_CORE "YES") # always needed
endif()

