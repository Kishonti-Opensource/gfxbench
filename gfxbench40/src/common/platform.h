/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
// NOTE:
//      Statistics mode: (slow and detailed)
//          ENABLE_COMPLETE_STATS -> 1
//          ENABLE_QUICK_STATS -> 0 (both here and in gl_wrapper_private.h)
//          force GL context to 4.5 in glfwgraphicswindow.cpp (needed by ARB_pipeline_statistics_query)
//          must have GL 4.5 capable gfx driver

//
//      Quick stats mode: (can be used runtime)
//          all the above, plus:
//          ENABLE_COMPLETE_STATS -> 0
//          ENABLE_QUICK_STATS -> 1 (both here and in gl_wrapper_private.h)


#define ENABLE_COMPLETE_STATS 0
#define ENABLE_QUICK_STATS 0

// Currently OpenGL wrapper is limited to GLEW
#define GL_WRAPPER_ENABLED ((ENABLE_COMPLETE_STATS || ENABLE_QUICK_STATS) && defined HAVE_GLEW)

// windows macosx linux ios emscripten android
void DiscardDepthAttachment();
void DiscardColorAttachment();

//////////////////
// header includes
#include <kcl_base.h>

#if defined WIN32 && !defined NOMINMAX
#define NOMINMAX
#endif

// Help out windows:
#if defined( _DEBUG ) && !defined( DEBUG )
#define DEBUG
#endif

#if defined HAVE_DX || defined USE_METAL
    // windows phone, windows metro, dx
#elif defined OPENGL_IMPLEMENTATION_NULL
#  include "nulldriver.h"
#elif defined ENABLE_FRAME_CAPTURE
#  include <..\framecapture\gl3_capture.h>

#define gl_wrapper
#define __glew_h__

void glewInit();
void glDrawBuffer(GLenum mode);
void glDepthRange(float zNear, float zFar);
void glPolygonMode(GLenum face, GLenum mode);
#else // android, ios
#  define GL_GLEXT_PROTOTYPES
#  if GL_WRAPPER_ENABLED
#    include "opengl/gl_wrapper/gl_wrapper_platform.h"
#  else
#    include "oglx/gl.h"
#  endif
#  if defined __APPLE__
#    include <TargetConditionals.h>
#    if TARGET_OS_IPHONE
#      define __gl3_h_
#      define HAVE_GLES3 1
#      include <OpenGLES/ES3/gl.h>
#      include <OpenGLES/ES3/glext.h>
#    endif
#  endif
#endif

#if !defined HAVE_DX
#  include "gl_defines.h"
#endif
