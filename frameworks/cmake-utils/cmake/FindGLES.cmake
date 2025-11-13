if (IOS)
	find_path(GLES_INCLUDE_DIR OpenGLES/ES1/gl.h)
	set(GLES_LIBRARY "-framework OpenGLES")
else()
	find_path(GLES_INCLUDE_DIR GLES/gl.h)
	find_library(GLES_LIBRARY NAMES GLESv1_CM)

endif()
set(GLES_INCLUDE_DIRS ${GLES_INCLUDE_DIR})
set(GLES_LIBRARIES ${GLES_LIBRARY})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GLES
                                  REQUIRED_VARS GLES_INCLUDE_DIR GLES_LIBRARY)
mark_as_advanced(GLES_INCLUDE_DIR GLES_LIBRARY)
