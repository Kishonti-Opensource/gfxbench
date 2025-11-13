message(STATUS "The path to emcc, em++, etc. is $ENV{EMSCRIPTEN}")

include ($ENV{EMSCRIPTEN}/cmake/Modules/Platform/Emscripten.cmake)

set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE BOTH)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY BOTH)

