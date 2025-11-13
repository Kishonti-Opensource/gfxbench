/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef GL_WRAPPER_PLATFORM_H
#define GL_WRAPPER_PLATFORM_H

#ifdef HAVE_EGL
#include <EGL/egl.h>
#endif

#include "private/gl_wrapper_gl31_tokens.h"
#include "private/gl_wrapper_gl31_functions.h"

extern GLboolean glewExperimental;
GLenum glewInit();
GLboolean glewIsSupported(const char *name);

void GL_APIENTRY glBlendFunci(GLuint buf, GLenum sfactor, GLenum dfactor);
void GL_APIENTRY glDrawBuffer(GLenum mode);
void GL_APIENTRY glDepthRange(float zNear, float zFar);
void GL_APIENTRY glGetTexImage(GLenum target, GLint level, GLenum format, GLenum type, GLvoid * pixels);
void GL_APIENTRY glPolygonMode(GLenum face, GLenum mode);
void GL_APIENTRY glPatchParameteri(GLenum pname, GLint value);
void GL_APIENTRY glGetQueryObjectui64v(GLuint id, GLenum pname, GLuint64* params);
#define __glew_h__

typedef void (GL_APIENTRY *GLDEBUGPROCARB)(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam);

void GL_APIENTRY glDebugMessageCallback(GLDEBUGPROCARB callback, const void *userParam);
GLuint GL_APIENTRY glGetDebugMessageLog(GLuint count, GLsizei bufsize, GLenum* sources, GLenum* types, GLuint* ids, GLenum* severities, GLsizei* lengths, GLchar* messageLog);
void GL_APIENTRY glDebugMessageControl(GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint* ids, GLboolean enabled);
void GL_APIENTRY glDebugMessageInsert(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* buf);
void GL_APIENTRY glGetObjectLabel(GLenum identifier, GLuint name, GLsizei bufSize, GLsizei* length, GLchar *label);
void GL_APIENTRY glGetObjectPtrLabel(const void *ptr, GLsizei bufSize, GLsizei* length, GLchar *label);
void GL_APIENTRY glObjectLabel(GLenum identifier, GLuint name, GLsizei length, const GLchar* label);
void GL_APIENTRY glObjectPtrLabel(const void *ptr, GLsizei length, const GLchar* label);
void GL_APIENTRY glPopDebugGroup(void);
void GL_APIENTRY glPushDebugGroup(GLenum source, GLuint id, GLsizei length, const GLchar *message);

void GL_APIENTRY glGetPointerv (GLenum pname, void* *params);

#ifndef GLEW_OK
#define GLEW_OK 0
#endif

#ifndef GL_DEBUG_SEVERITY_HIGH
#define GL_DEBUG_SEVERITY_HIGH 0x9146
#endif

#ifndef GL_DEBUG_SEVERITY_MEDIUM
#define GL_DEBUG_SEVERITY_MEDIUM 0x9147
#endif

#ifndef GL_DEBUG_SEVERITY_LOW
#define GL_DEBUG_SEVERITY_LOW 0x9148
#endif

#ifndef GL_TEXTURE_CUBE_MAP_SEAMLESS
#define GL_TEXTURE_CUBE_MAP_SEAMLESS 0x884F
#endif

#ifndef GL_TEXTURE_CUBE_MAP_ARRAY
#define GL_TEXTURE_CUBE_MAP_ARRAY 0x9009
#endif

#ifndef GL_CONTEXT_PROFILE_MASK
#define GL_CONTEXT_PROFILE_MASK 0x9126
#endif

#ifndef GL_CONTEXT_CORE_PROFILE_BIT
#define GL_CONTEXT_CORE_PROFILE_BIT 0x00000001
#endif

#ifndef GL_CONTEXT_COMPATIBILITY_PROFILE_BIT
#define GL_CONTEXT_COMPATIBILITY_PROFILE_BIT 0x00000002
#endif

#endif
