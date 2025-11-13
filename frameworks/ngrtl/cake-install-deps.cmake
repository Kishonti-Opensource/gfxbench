cake_pkg(INSTALL cmake-utils)

include(${CMAKE_CURRENT_LIST_DIR}/decide_which_modules.cmake)

if(WINCE)
  cake_pkg(INSTALL NAME wcecompat)
endif()

if(NGRTL_ENABLE_FONT)
  cake_pkg(INSTALL NAME ZLIB)
  cake_pkg(INSTALL NAME Freetype)
endif()

if(NGRTL_ENABLE_GDAL)
  cake_pkg(INSTALL NAME GDAL)
endif()

if(NGRTL_ENABLE_LEGACY)
  cake_pkg(INSTALL NAME ZLIB)
endif()

if(NGRTL_ENABLE_OGLFONT)
  set(NGRTL_ENABLE_I18N ON)
  set(NGRTL_ENABLE_OGL ON)
  cake_pkg(INSTALL NAME Freetype)
endif()

if(NGRTL_ENABLE_OGL)
  cake_pkg(INSTALL NAME OGLX)
endif()

if(NGRTL_ENABLE_OCCI)
  cake_pkg(INSTALL NAME OCCI)
endif()

if(NGRTL_ENABLE_PDB)
  set(NGRTL_ENABLE_DBM ON)
  cake_pkg(INSTALL NAME Protobuf)
endif()

if(NGRTL_ENABLE_DBM)
  set(backends LMDB leveldb sqlite3)
  foreach(b ${backends})
    string(TOUPPER "${b}" B)
    if(NGRTL_WITH_${B})
      cake_pkg(INSTALL NAME ${b})
    endif()
  endforeach()
endif()

if(NGRTL_ENABLE_PNGIO)
  cake_pkg(INSTALL NAME PNG)
endif()

if(NGRTL_ENABLE_POSTGRES)
  cake_pkg(INSTALL NAME PostgreSQL)
  cake_pkg(INSTALL NAME pqtypes)
endif()

if(NGRTL_ENABLE_SQLITE)
  cake_pkg(INSTALL NAME sqlite3)
endif()
