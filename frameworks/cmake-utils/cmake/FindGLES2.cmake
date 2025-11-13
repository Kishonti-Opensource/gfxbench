if (IOS)
	find_path(GLES2_INCLUDE_DIR OpenGLES/ES2/gl.h)
	set(GLES2_LIBRARY "-framework OpenGLES")
else()
	find_path(GLES2_INCLUDE_DIR GLES2/gl2.h)
	find_library(GLES2_LIBRARY NAMES GLESv2)

endif()
set(GLES2_INCLUDE_DIRS ${GLES2_INCLUDE_DIR})
set(GLES2_LIBRARIES ${GLES2_LIBRARY})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GLES2
                                  REQUIRED_VARS GLES2_INCLUDE_DIR GLES2_LIBRARY)
mark_as_advanced(GLES2_INCLUDE_DIR GLES2_LIBRARY)
