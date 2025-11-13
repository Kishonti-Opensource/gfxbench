/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef INCLUDE_GUARD_gl_1399294876
#define INCLUDE_GUARD_gl_1399294876

#if defined HAVE_EGL
#  include <EGL/egl.h>
#endif
#if defined __APPLE__
#  include <TargetConditionals.h>
#  if TARGET_OS_IPHONE
#    include <OpenGLES/ES1/gl.h>
#    include <OpenGLES/ES2/gl.h>     // TODO: if IOS7 include ES3
#    include <OpenGLES/ES2/glext.h>
#  else
#    if defined HAVE_GLEW
#      include <GL/glew.h>
#    elif defined HAVE_GL
#      include <OpenGL/gl.h>
#    else
#      error "Cannot find OpenGL headers on MACOSX"
#    endif
#  endif
#else
#  if defined HAVE_GLEW
#    include <GL/glew.h>
#  elif defined HAVE_GL
#    ifdef _WIN32
#      include <windows.h>
#    endif
#    include <GL/gl.h>
#  elif defined HAVE_GLES31 || HAVE_GLES3 || defined HAVE_GLES2 || defined HAVE_GLES
#    if defined HAVE_GLES31
#		define GL_GLEXT_PROTOTYPES
#		include <GLES3/gl31.h>
#       include <GLES2/gl2ext.h>
#     elif defined HAVE_GLES3
#       include <GLES3/gl3.h>

#    elif defined HAVE_GLES2
#       include <GLES2/gl2.h>
#       include <GLES2/gl2ext.h>
#    endif
#    if defined HAVE_GLES
#      include <GLES/gl.h>
#      include <GLES/glext.h>
#    endif
#  else
#    error "Cannot find OpenGL headers"
#  endif
#endif

#endif
