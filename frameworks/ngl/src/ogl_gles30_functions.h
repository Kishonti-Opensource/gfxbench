/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef __OGL_GLES30_FUNCIONS_H__
#define __OGL_GLES30_FUNCIONS_H__

#include "ogl_gles31_functions.h"

void GFXB_APIENTRY glGetProgramResourceivLegacy(GLuint program, GLenum programInterface, GLuint index, GLsizei propCount, const GLenum* props, GLsizei bufSize, GLsizei *length, GLint *params);
void GFXB_APIENTRY glGetProgramInterfaceivLegacy(GLuint program, GLenum programInterface, GLenum pname, GLint* params);
void GFXB_APIENTRY glGetProgramResourceNameLegacy(GLuint program, GLenum programInterface, GLuint index, GLsizei bufSize, GLsizei* length, GLchar *name);

void getES30ProcAddresses();

#if defined HAVE_GLES3 // || defined ENABLE_FRAME_CAPTURE || GL_WRAPPER_ENABLED || defined EMSCRIPTEN || defined NACL

#define GL_OFFSET 0x92FC
#define GL_BLOCK_INDEX 0x92FD
#define GL_ARRAY_STRIDE 0x92FE
#define GL_MATRIX_STRIDE 0x92FF
#define GL_IS_ROW_MAJOR 0x9300
#define GL_NUM_ACTIVE_VARIABLES 0x9304
#define GL_ACTIVE_VARIABLES 0x9305

#define GL_BUFFER_BINDING 0x9302
#define GL_BUFFER_DATA_SIZE 0x9303
#define GL_REFERENCED_BY_VERTEX_SHADER 0x9306
#define GL_REFERENCED_BY_TESS_CONTROL_SHADER 0x9307

#endif

#endif  // __OGL_GLES30_FUNCIONS_H__