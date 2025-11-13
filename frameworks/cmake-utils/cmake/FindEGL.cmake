
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
 set_property(GLOBAL PROPERTY FIND_LIBRARY_USE_LIB64_PATHS TRUE)
endif()

find_path(EGL_INCLUDE_DIR EGL/egl.h)
find_library(EGL_LIBRARY NAMES EGL)

set(EGL_INCLUDE_DIRS ${EGL_INCLUDE_DIR})
set(EGL_LIBRARIES ${EGL_LIBRARY})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(EGL
                                  REQUIRED_VARS EGL_INCLUDE_DIR EGL_LIBRARY)
message("EGL: ${EGL_LIBRARIES}")
mark_as_advanced(EGL_INCLUDE_DIR EGL_LIBRARY)
