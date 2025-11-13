/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "gl_wrapper_private.h"
#ifdef HAVE_GLEW
GLWrapperImpl * wrapper = GLWrapperImpl::Get();

#pragma push_macro("glActiveTexture")
#undef glActiveTexture
void GL_APIENTRY glActiveTexture (GLenum texture) {
#pragma pop_macro("glActiveTexture")
	wrapper->OnFunctionCall("glActiveTexture");
	wrapper->OnActiveTexture(texture);
	GL_Impl::glActiveTexture(texture);
	wrapper->CheckGLError("glActiveTexture");
}

#pragma push_macro("glAttachShader")
#undef glAttachShader
void GL_APIENTRY glAttachShader (GLuint program, GLuint shader) {
#pragma pop_macro("glAttachShader")
	wrapper->OnFunctionCall("glAttachShader");
	GL_Impl::glAttachShader(program, shader);
	wrapper->CheckGLError("glAttachShader");
}

#pragma push_macro("glBindAttribLocation")
#undef glBindAttribLocation
void GL_APIENTRY glBindAttribLocation (GLuint program, GLuint index, const GLchar *name) {
#pragma pop_macro("glBindAttribLocation")
	wrapper->OnFunctionCall("glBindAttribLocation");
	GL_Impl::glBindAttribLocation(program, index, name);
	wrapper->CheckGLError("glBindAttribLocation");
}

#pragma push_macro("glBindBuffer")
#undef glBindBuffer
void GL_APIENTRY glBindBuffer (GLenum target, GLuint buffer) {
#pragma pop_macro("glBindBuffer")
	wrapper->OnFunctionCall("glBindBuffer");
	GL_Impl::glBindBuffer(target, buffer);
	wrapper->CheckGLError("glBindBuffer");
}

#pragma push_macro("glBindFramebuffer")
#undef glBindFramebuffer
void GL_APIENTRY glBindFramebuffer (GLenum target, GLuint framebuffer) {
#pragma pop_macro("glBindFramebuffer")
	wrapper->OnFunctionCall("glBindFramebuffer");
	GL_Impl::glBindFramebuffer(target, framebuffer);
	wrapper->CheckGLError("glBindFramebuffer");
}

#pragma push_macro("glBindRenderbuffer")
#undef glBindRenderbuffer
void GL_APIENTRY glBindRenderbuffer (GLenum target, GLuint renderbuffer) {
#pragma pop_macro("glBindRenderbuffer")
	wrapper->OnFunctionCall("glBindRenderbuffer");
	GL_Impl::glBindRenderbuffer(target, renderbuffer);
	wrapper->CheckGLError("glBindRenderbuffer");
}

#pragma push_macro("glBindTexture")
#undef glBindTexture
void GL_APIENTRY glBindTexture (GLenum target, GLuint texture) {
#pragma pop_macro("glBindTexture")
	wrapper->OnFunctionCall("glBindTexture");
	GL_Impl::glBindTexture(target, texture);
	wrapper->CheckGLError("glBindTexture");
}

#pragma push_macro("glBlendColor")
#undef glBlendColor
void GL_APIENTRY glBlendColor (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) {
#pragma pop_macro("glBlendColor")
	wrapper->OnFunctionCall("glBlendColor");
	GL_Impl::glBlendColor(red, green, blue, alpha);
	wrapper->CheckGLError("glBlendColor");
}

#pragma push_macro("glBlendEquation")
#undef glBlendEquation
void GL_APIENTRY glBlendEquation (GLenum mode) {
#pragma pop_macro("glBlendEquation")
	wrapper->OnFunctionCall("glBlendEquation");
	GL_Impl::glBlendEquation(mode);
	wrapper->CheckGLError("glBlendEquation");
}

#pragma push_macro("glBlendEquationSeparate")
#undef glBlendEquationSeparate
void GL_APIENTRY glBlendEquationSeparate (GLenum modeRGB, GLenum modeAlpha) {
#pragma pop_macro("glBlendEquationSeparate")
	wrapper->OnFunctionCall("glBlendEquationSeparate");
	GL_Impl::glBlendEquationSeparate(modeRGB, modeAlpha);
	wrapper->CheckGLError("glBlendEquationSeparate");
}

#pragma push_macro("glBlendFunc")
#undef glBlendFunc
void GL_APIENTRY glBlendFunc (GLenum sfactor, GLenum dfactor) {
#pragma pop_macro("glBlendFunc")
	wrapper->OnFunctionCall("glBlendFunc");
	GL_Impl::glBlendFunc(sfactor, dfactor);
	wrapper->CheckGLError("glBlendFunc");
}

#pragma push_macro("glBlendFuncSeparate")
#undef glBlendFuncSeparate
void GL_APIENTRY glBlendFuncSeparate (GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha) {
#pragma pop_macro("glBlendFuncSeparate")
	wrapper->OnFunctionCall("glBlendFuncSeparate");
	GL_Impl::glBlendFuncSeparate(sfactorRGB, dfactorRGB, sfactorAlpha, dfactorAlpha);
	wrapper->CheckGLError("glBlendFuncSeparate");
}

#pragma push_macro("glBufferData")
#undef glBufferData
void GL_APIENTRY glBufferData (GLenum target, GLsizeiptr size, const void *data, GLenum usage) {
#pragma pop_macro("glBufferData")
	wrapper->OnFunctionCall("glBufferData");
	wrapper->OnBufferData(target, size, data, usage);
	GL_Impl::glBufferData(target, size, data, usage);
	wrapper->CheckGLError("glBufferData");
}

#pragma push_macro("glBufferSubData")
#undef glBufferSubData
void GL_APIENTRY glBufferSubData (GLenum target, GLintptr offset, GLsizeiptr size, const void *data) {
#pragma pop_macro("glBufferSubData")
	wrapper->OnFunctionCall("glBufferSubData");
	GL_Impl::glBufferSubData(target, offset, size, data);
	wrapper->CheckGLError("glBufferSubData");
}

#pragma push_macro("glCheckFramebufferStatus")
#undef glCheckFramebufferStatus
GLenum GL_APIENTRY glCheckFramebufferStatus (GLenum target) {
#pragma pop_macro("glCheckFramebufferStatus")
	wrapper->OnFunctionCall("glCheckFramebufferStatus");
	GLenum retval = (GLenum)GL_Impl::glCheckFramebufferStatus(target);
	wrapper->CheckGLError("glCheckFramebufferStatus");
	return retval;
}

#pragma push_macro("glClear")
#undef glClear
void GL_APIENTRY glClear (GLbitfield mask) {
#pragma pop_macro("glClear")
	wrapper->OnFunctionCall("glClear");
	GL_Impl::glClear(mask);
	wrapper->CheckGLError("glClear");
}

#pragma push_macro("glClearColor")
#undef glClearColor
void GL_APIENTRY glClearColor (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) {
#pragma pop_macro("glClearColor")
	wrapper->OnFunctionCall("glClearColor");
	GL_Impl::glClearColor(red, green, blue, alpha);
	wrapper->CheckGLError("glClearColor");
}

#pragma push_macro("glClearDepthf")
#undef glClearDepthf
void GL_APIENTRY glClearDepthf (GLfloat d) {
#pragma pop_macro("glClearDepthf")
	wrapper->OnFunctionCall("glClearDepthf");
	GL_Impl::glClearDepthf(d);
	wrapper->CheckGLError("glClearDepthf");
}

#pragma push_macro("glClearStencil")
#undef glClearStencil
void GL_APIENTRY glClearStencil (GLint s) {
#pragma pop_macro("glClearStencil")
	wrapper->OnFunctionCall("glClearStencil");
	GL_Impl::glClearStencil(s);
	wrapper->CheckGLError("glClearStencil");
}

#pragma push_macro("glColorMask")
#undef glColorMask
void GL_APIENTRY glColorMask (GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha) {
#pragma pop_macro("glColorMask")
	wrapper->OnFunctionCall("glColorMask");
	GL_Impl::glColorMask(red, green, blue, alpha);
	wrapper->CheckGLError("glColorMask");
}

#pragma push_macro("glCompileShader")
#undef glCompileShader
void GL_APIENTRY glCompileShader (GLuint shader) {
#pragma pop_macro("glCompileShader")
	wrapper->OnFunctionCall("glCompileShader");
	GL_Impl::glCompileShader(shader);
	wrapper->CheckGLError("glCompileShader");
}

#pragma push_macro("glCompressedTexImage2D")
#undef glCompressedTexImage2D
void GL_APIENTRY glCompressedTexImage2D (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void *data) {
#pragma pop_macro("glCompressedTexImage2D")
	wrapper->OnFunctionCall("glCompressedTexImage2D");
	GL_Impl::glCompressedTexImage2D(target, level, internalformat, width, height, border, imageSize, data);
	wrapper->CheckGLError("glCompressedTexImage2D");
}

#pragma push_macro("glCompressedTexSubImage2D")
#undef glCompressedTexSubImage2D
void GL_APIENTRY glCompressedTexSubImage2D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void *data) {
#pragma pop_macro("glCompressedTexSubImage2D")
	wrapper->OnFunctionCall("glCompressedTexSubImage2D");
	GL_Impl::glCompressedTexSubImage2D(target, level, xoffset, yoffset, width, height, format, imageSize, data);
	wrapper->CheckGLError("glCompressedTexSubImage2D");
}

#pragma push_macro("glCopyTexImage2D")
#undef glCopyTexImage2D
void GL_APIENTRY glCopyTexImage2D (GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border) {
#pragma pop_macro("glCopyTexImage2D")
	wrapper->OnFunctionCall("glCopyTexImage2D");
	GL_Impl::glCopyTexImage2D(target, level, internalformat, x, y, width, height, border);
	wrapper->CheckGLError("glCopyTexImage2D");
}

#pragma push_macro("glCopyTexSubImage2D")
#undef glCopyTexSubImage2D
void GL_APIENTRY glCopyTexSubImage2D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height) {
#pragma pop_macro("glCopyTexSubImage2D")
	wrapper->OnFunctionCall("glCopyTexSubImage2D");
	GL_Impl::glCopyTexSubImage2D(target, level, xoffset, yoffset, x, y, width, height);
	wrapper->CheckGLError("glCopyTexSubImage2D");
}

#pragma push_macro("glCreateProgram")
#undef glCreateProgram
GLuint GL_APIENTRY glCreateProgram (void) {
#pragma pop_macro("glCreateProgram")
	wrapper->OnFunctionCall("glCreateProgram");
	GLuint retval = (GLuint)GL_Impl::glCreateProgram();
	wrapper->CheckGLError("glCreateProgram");
	wrapper->OnGenObject(GLWrapperImpl::TYPE_PROGRAM, 1, &retval);
	return retval;
}

#pragma push_macro("glCreateShader")
#undef glCreateShader
GLuint GL_APIENTRY glCreateShader (GLenum type) {
#pragma pop_macro("glCreateShader")
	wrapper->OnFunctionCall("glCreateShader");
	GLuint retval = (GLuint)GL_Impl::glCreateShader(type);
	wrapper->CheckGLError("glCreateShader");
	wrapper->OnGenObject(GLWrapperImpl::TYPE_SHADER, 1, &retval);
	return retval;
}

#pragma push_macro("glCullFace")
#undef glCullFace
void GL_APIENTRY glCullFace (GLenum mode) {
#pragma pop_macro("glCullFace")
	wrapper->OnFunctionCall("glCullFace");
	GL_Impl::glCullFace(mode);
	wrapper->CheckGLError("glCullFace");
}

#pragma push_macro("glDeleteBuffers")
#undef glDeleteBuffers
void GL_APIENTRY glDeleteBuffers (GLsizei n, const GLuint *buffers) {
#pragma pop_macro("glDeleteBuffers")
	wrapper->OnFunctionCall("glDeleteBuffers");
	wrapper->OnDeleteBuffers(n, buffers);
	GL_Impl::glDeleteBuffers(n, buffers);
	wrapper->CheckGLError("glDeleteBuffers");
	wrapper->OnDeleteObject(GLWrapperImpl::TYPE_BUFFER, n, buffers);
}

#pragma push_macro("glDeleteFramebuffers")
#undef glDeleteFramebuffers
void GL_APIENTRY glDeleteFramebuffers (GLsizei n, const GLuint *framebuffers) {
#pragma pop_macro("glDeleteFramebuffers")
	wrapper->OnFunctionCall("glDeleteFramebuffers");
	GL_Impl::glDeleteFramebuffers(n, framebuffers);
	wrapper->CheckGLError("glDeleteFramebuffers");
	wrapper->OnDeleteObject(GLWrapperImpl::TYPE_FRAMEBUFFER, n, framebuffers);
}

#pragma push_macro("glDeleteProgram")
#undef glDeleteProgram
void GL_APIENTRY glDeleteProgram (GLuint program) {
#pragma pop_macro("glDeleteProgram")
	wrapper->OnFunctionCall("glDeleteProgram");
	GL_Impl::glDeleteProgram(program);
	wrapper->CheckGLError("glDeleteProgram");
	wrapper->OnDeleteObject(GLWrapperImpl::TYPE_PROGRAM, 1, &program);
}

#pragma push_macro("glDeleteRenderbuffers")
#undef glDeleteRenderbuffers
void GL_APIENTRY glDeleteRenderbuffers (GLsizei n, const GLuint *renderbuffers) {
#pragma pop_macro("glDeleteRenderbuffers")
	wrapper->OnFunctionCall("glDeleteRenderbuffers");
	GL_Impl::glDeleteRenderbuffers(n, renderbuffers);
	wrapper->CheckGLError("glDeleteRenderbuffers");
	wrapper->OnDeleteObject(GLWrapperImpl::TYPE_RENDERBUFFER, n, renderbuffers);
}

#pragma push_macro("glDeleteShader")
#undef glDeleteShader
void GL_APIENTRY glDeleteShader (GLuint shader) {
#pragma pop_macro("glDeleteShader")
	wrapper->OnFunctionCall("glDeleteShader");
	GL_Impl::glDeleteShader(shader);
	wrapper->CheckGLError("glDeleteShader");
	wrapper->OnDeleteObject(GLWrapperImpl::TYPE_SHADER, 1, &shader);
}

#pragma push_macro("glDeleteTextures")
#undef glDeleteTextures
void GL_APIENTRY glDeleteTextures (GLsizei n, const GLuint *textures) {
#pragma pop_macro("glDeleteTextures")
	wrapper->OnFunctionCall("glDeleteTextures");
	GL_Impl::glDeleteTextures(n, textures);
	wrapper->CheckGLError("glDeleteTextures");
	wrapper->OnDeleteObject(GLWrapperImpl::TYPE_TEXTURE, n, textures);
}

#pragma push_macro("glDepthFunc")
#undef glDepthFunc
void GL_APIENTRY glDepthFunc (GLenum func) {
#pragma pop_macro("glDepthFunc")
	wrapper->OnFunctionCall("glDepthFunc");
	GL_Impl::glDepthFunc(func);
	wrapper->CheckGLError("glDepthFunc");
}

#pragma push_macro("glDepthMask")
#undef glDepthMask
void GL_APIENTRY glDepthMask (GLboolean flag) {
#pragma pop_macro("glDepthMask")
	wrapper->OnFunctionCall("glDepthMask");
	GL_Impl::glDepthMask(flag);
	wrapper->CheckGLError("glDepthMask");
}

#pragma push_macro("glDepthRangef")
#undef glDepthRangef
void GL_APIENTRY glDepthRangef (GLfloat n, GLfloat f) {
#pragma pop_macro("glDepthRangef")
	wrapper->OnFunctionCall("glDepthRangef");
	GL_Impl::glDepthRangef(n, f);
	wrapper->CheckGLError("glDepthRangef");
}

#pragma push_macro("glDetachShader")
#undef glDetachShader
void GL_APIENTRY glDetachShader (GLuint program, GLuint shader) {
#pragma pop_macro("glDetachShader")
	wrapper->OnFunctionCall("glDetachShader");
	GL_Impl::glDetachShader(program, shader);
	wrapper->CheckGLError("glDetachShader");
}

#pragma push_macro("glDisable")
#undef glDisable
void GL_APIENTRY glDisable (GLenum cap) {
#pragma pop_macro("glDisable")
	wrapper->OnFunctionCall("glDisable");
	GL_Impl::glDisable(cap);
	wrapper->CheckGLError("glDisable");
}

#pragma push_macro("glDisableVertexAttribArray")
#undef glDisableVertexAttribArray
void GL_APIENTRY glDisableVertexAttribArray (GLuint index) {
#pragma pop_macro("glDisableVertexAttribArray")
	wrapper->OnFunctionCall("glDisableVertexAttribArray");
	GL_Impl::glDisableVertexAttribArray(index);
	wrapper->CheckGLError("glDisableVertexAttribArray");
}

#pragma push_macro("glDrawArrays")
#undef glDrawArrays
void GL_APIENTRY glDrawArrays (GLenum mode, GLint first, GLsizei count) {
#pragma pop_macro("glDrawArrays")
	wrapper->OnFunctionCall("glDrawArrays");
	wrapper->OnPreDrawCall(mode, count, 1);
	GL_Impl::glDrawArrays(mode, first, count);
	wrapper->CheckGLError("glDrawArrays");
	wrapper->OnPostDrawCall();
}

#pragma push_macro("glDrawElements")
#undef glDrawElements
void GL_APIENTRY glDrawElements (GLenum mode, GLsizei count, GLenum type, const void *indices) {
#pragma pop_macro("glDrawElements")
	wrapper->OnFunctionCall("glDrawElements");
	wrapper->OnPreDrawCall(mode, count, 1);
	GL_Impl::glDrawElements(mode, count, type, indices);
	wrapper->CheckGLError("glDrawElements");
	wrapper->OnPostDrawCall();
}

#pragma push_macro("glEnable")
#undef glEnable
void GL_APIENTRY glEnable (GLenum cap) {
#pragma pop_macro("glEnable")
	wrapper->OnFunctionCall("glEnable");
	GL_Impl::glEnable(cap);
	wrapper->CheckGLError("glEnable");
}

#pragma push_macro("glEnableVertexAttribArray")
#undef glEnableVertexAttribArray
void GL_APIENTRY glEnableVertexAttribArray (GLuint index) {
#pragma pop_macro("glEnableVertexAttribArray")
	wrapper->OnFunctionCall("glEnableVertexAttribArray");
	GL_Impl::glEnableVertexAttribArray(index);
	wrapper->CheckGLError("glEnableVertexAttribArray");
}

#pragma push_macro("glFinish")
#undef glFinish
void GL_APIENTRY glFinish (void) {
#pragma pop_macro("glFinish")
	wrapper->OnFunctionCall("glFinish");
	GL_Impl::glFinish();
	wrapper->CheckGLError("glFinish");
}

#pragma push_macro("glFlush")
#undef glFlush
void GL_APIENTRY glFlush (void) {
#pragma pop_macro("glFlush")
	wrapper->OnFunctionCall("glFlush");
	GL_Impl::glFlush();
	wrapper->CheckGLError("glFlush");
}

#pragma push_macro("glFramebufferRenderbuffer")
#undef glFramebufferRenderbuffer
void GL_APIENTRY glFramebufferRenderbuffer (GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer) {
#pragma pop_macro("glFramebufferRenderbuffer")
	wrapper->OnFunctionCall("glFramebufferRenderbuffer");
	GL_Impl::glFramebufferRenderbuffer(target, attachment, renderbuffertarget, renderbuffer);
	wrapper->CheckGLError("glFramebufferRenderbuffer");
}

#pragma push_macro("glFramebufferTexture2D")
#undef glFramebufferTexture2D
void GL_APIENTRY glFramebufferTexture2D (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level) {
#pragma pop_macro("glFramebufferTexture2D")
	wrapper->OnFunctionCall("glFramebufferTexture2D");
	GL_Impl::glFramebufferTexture2D(target, attachment, textarget, texture, level);
	wrapper->CheckGLError("glFramebufferTexture2D");
}

#pragma push_macro("glFrontFace")
#undef glFrontFace
void GL_APIENTRY glFrontFace (GLenum mode) {
#pragma pop_macro("glFrontFace")
	wrapper->OnFunctionCall("glFrontFace");
	GL_Impl::glFrontFace(mode);
	wrapper->CheckGLError("glFrontFace");
}

#pragma push_macro("glGenBuffers")
#undef glGenBuffers
void GL_APIENTRY glGenBuffers (GLsizei n, GLuint *buffers) {
#pragma pop_macro("glGenBuffers")
	wrapper->OnFunctionCall("glGenBuffers");
	GL_Impl::glGenBuffers(n, buffers);
	wrapper->CheckGLError("glGenBuffers");
	wrapper->OnGenObject(GLWrapperImpl::TYPE_BUFFER, n, buffers);
}

#pragma push_macro("glGenerateMipmap")
#undef glGenerateMipmap
void GL_APIENTRY glGenerateMipmap (GLenum target) {
#pragma pop_macro("glGenerateMipmap")
	wrapper->OnFunctionCall("glGenerateMipmap");
	GL_Impl::glGenerateMipmap(target);
	wrapper->CheckGLError("glGenerateMipmap");
}

#pragma push_macro("glGenFramebuffers")
#undef glGenFramebuffers
void GL_APIENTRY glGenFramebuffers (GLsizei n, GLuint *framebuffers) {
#pragma pop_macro("glGenFramebuffers")
	wrapper->OnFunctionCall("glGenFramebuffers");
	GL_Impl::glGenFramebuffers(n, framebuffers);
	wrapper->CheckGLError("glGenFramebuffers");
	wrapper->OnGenObject(GLWrapperImpl::TYPE_FRAMEBUFFER, n, framebuffers);
}

#pragma push_macro("glGenRenderbuffers")
#undef glGenRenderbuffers
void GL_APIENTRY glGenRenderbuffers (GLsizei n, GLuint *renderbuffers) {
#pragma pop_macro("glGenRenderbuffers")
	wrapper->OnFunctionCall("glGenRenderbuffers");
	GL_Impl::glGenRenderbuffers(n, renderbuffers);
	wrapper->CheckGLError("glGenRenderbuffers");
	wrapper->OnGenObject(GLWrapperImpl::TYPE_RENDERBUFFER, n, renderbuffers);
}

#pragma push_macro("glGenTextures")
#undef glGenTextures
void GL_APIENTRY glGenTextures (GLsizei n, GLuint *textures) {
#pragma pop_macro("glGenTextures")
	wrapper->OnFunctionCall("glGenTextures");
	GL_Impl::glGenTextures(n, textures);
	wrapper->CheckGLError("glGenTextures");
	wrapper->OnGenObject(GLWrapperImpl::TYPE_TEXTURE, n, textures);
}

#pragma push_macro("glGetActiveAttrib")
#undef glGetActiveAttrib
void GL_APIENTRY glGetActiveAttrib (GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name) {
#pragma pop_macro("glGetActiveAttrib")
	wrapper->OnFunctionCall("glGetActiveAttrib");
	GL_Impl::glGetActiveAttrib(program, index, bufSize, length, size, type, name);
	wrapper->CheckGLError("glGetActiveAttrib");
}

#pragma push_macro("glGetActiveUniform")
#undef glGetActiveUniform
void GL_APIENTRY glGetActiveUniform (GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name) {
#pragma pop_macro("glGetActiveUniform")
	wrapper->OnFunctionCall("glGetActiveUniform");
	GL_Impl::glGetActiveUniform(program, index, bufSize, length, size, type, name);
	wrapper->CheckGLError("glGetActiveUniform");
}

#pragma push_macro("glGetAttachedShaders")
#undef glGetAttachedShaders
void GL_APIENTRY glGetAttachedShaders (GLuint program, GLsizei maxCount, GLsizei *count, GLuint *shaders) {
#pragma pop_macro("glGetAttachedShaders")
	wrapper->OnFunctionCall("glGetAttachedShaders");
	GL_Impl::glGetAttachedShaders(program, maxCount, count, shaders);
	wrapper->CheckGLError("glGetAttachedShaders");
}

#pragma push_macro("glGetAttribLocation")
#undef glGetAttribLocation
GLint GL_APIENTRY glGetAttribLocation (GLuint program, const GLchar *name) {
#pragma pop_macro("glGetAttribLocation")
	wrapper->OnFunctionCall("glGetAttribLocation");
	GLint retval = (GLint)GL_Impl::glGetAttribLocation(program, name);
	wrapper->CheckGLError("glGetAttribLocation");
	return retval;
}

#pragma push_macro("glGetBooleanv")
#undef glGetBooleanv
void GL_APIENTRY glGetBooleanv (GLenum pname, GLboolean *data) {
#pragma pop_macro("glGetBooleanv")
	wrapper->OnFunctionCall("glGetBooleanv");
	GL_Impl::glGetBooleanv(pname, data);
	wrapper->CheckGLError("glGetBooleanv");
}

#pragma push_macro("glGetBufferParameteriv")
#undef glGetBufferParameteriv
void GL_APIENTRY glGetBufferParameteriv (GLenum target, GLenum pname, GLint *params) {
#pragma pop_macro("glGetBufferParameteriv")
	wrapper->OnFunctionCall("glGetBufferParameteriv");
	GL_Impl::glGetBufferParameteriv(target, pname, params);
	wrapper->CheckGLError("glGetBufferParameteriv");
}

#pragma push_macro("glGetError")
#undef glGetError
GLenum GL_APIENTRY glGetError (void) {
#pragma pop_macro("glGetError")
	wrapper->OnFunctionCall("glGetError");
	GLenum retval = (GLenum)GL_Impl::glGetError();
	wrapper->CheckGLError("glGetError");
	return retval;
}

#pragma push_macro("glGetFloatv")
#undef glGetFloatv
void GL_APIENTRY glGetFloatv (GLenum pname, GLfloat *data) {
#pragma pop_macro("glGetFloatv")
	wrapper->OnFunctionCall("glGetFloatv");
	GL_Impl::glGetFloatv(pname, data);
	wrapper->CheckGLError("glGetFloatv");
}

#pragma push_macro("glGetFramebufferAttachmentParameteriv")
#undef glGetFramebufferAttachmentParameteriv
void GL_APIENTRY glGetFramebufferAttachmentParameteriv (GLenum target, GLenum attachment, GLenum pname, GLint *params) {
#pragma pop_macro("glGetFramebufferAttachmentParameteriv")
	wrapper->OnFunctionCall("glGetFramebufferAttachmentParameteriv");
	GL_Impl::glGetFramebufferAttachmentParameteriv(target, attachment, pname, params);
	wrapper->CheckGLError("glGetFramebufferAttachmentParameteriv");
}

#pragma push_macro("glGetIntegerv")
#undef glGetIntegerv
void GL_APIENTRY glGetIntegerv (GLenum pname, GLint *data) {
#pragma pop_macro("glGetIntegerv")
	wrapper->OnFunctionCall("glGetIntegerv");
	GL_Impl::glGetIntegerv(pname, data);
	wrapper->CheckGLError("glGetIntegerv");
}

#pragma push_macro("glGetProgramiv")
#undef glGetProgramiv
void GL_APIENTRY glGetProgramiv (GLuint program, GLenum pname, GLint *params) {
#pragma pop_macro("glGetProgramiv")
	wrapper->OnFunctionCall("glGetProgramiv");
	GL_Impl::glGetProgramiv(program, pname, params);
	wrapper->CheckGLError("glGetProgramiv");
}

#pragma push_macro("glGetProgramInfoLog")
#undef glGetProgramInfoLog
void GL_APIENTRY glGetProgramInfoLog (GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog) {
#pragma pop_macro("glGetProgramInfoLog")
	wrapper->OnFunctionCall("glGetProgramInfoLog");
	GL_Impl::glGetProgramInfoLog(program, bufSize, length, infoLog);
	wrapper->CheckGLError("glGetProgramInfoLog");
}

#pragma push_macro("glGetRenderbufferParameteriv")
#undef glGetRenderbufferParameteriv
void GL_APIENTRY glGetRenderbufferParameteriv (GLenum target, GLenum pname, GLint *params) {
#pragma pop_macro("glGetRenderbufferParameteriv")
	wrapper->OnFunctionCall("glGetRenderbufferParameteriv");
	GL_Impl::glGetRenderbufferParameteriv(target, pname, params);
	wrapper->CheckGLError("glGetRenderbufferParameteriv");
}

#pragma push_macro("glGetShaderiv")
#undef glGetShaderiv
void GL_APIENTRY glGetShaderiv (GLuint shader, GLenum pname, GLint *params) {
#pragma pop_macro("glGetShaderiv")
	wrapper->OnFunctionCall("glGetShaderiv");
	GL_Impl::glGetShaderiv(shader, pname, params);
	wrapper->CheckGLError("glGetShaderiv");
}

#pragma push_macro("glGetShaderInfoLog")
#undef glGetShaderInfoLog
void GL_APIENTRY glGetShaderInfoLog (GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog) {
#pragma pop_macro("glGetShaderInfoLog")
	wrapper->OnFunctionCall("glGetShaderInfoLog");
	GL_Impl::glGetShaderInfoLog(shader, bufSize, length, infoLog);
	wrapper->CheckGLError("glGetShaderInfoLog");
}

#pragma push_macro("glGetShaderPrecisionFormat")
#undef glGetShaderPrecisionFormat
void GL_APIENTRY glGetShaderPrecisionFormat (GLenum shadertype, GLenum precisiontype, GLint *range, GLint *precision) {
#pragma pop_macro("glGetShaderPrecisionFormat")
	wrapper->OnFunctionCall("glGetShaderPrecisionFormat");
	GL_Impl::glGetShaderPrecisionFormat(shadertype, precisiontype, range, precision);
	wrapper->CheckGLError("glGetShaderPrecisionFormat");
}

#pragma push_macro("glGetShaderSource")
#undef glGetShaderSource
void GL_APIENTRY glGetShaderSource (GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *source) {
#pragma pop_macro("glGetShaderSource")
	wrapper->OnFunctionCall("glGetShaderSource");
	GL_Impl::glGetShaderSource(shader, bufSize, length, source);
	wrapper->CheckGLError("glGetShaderSource");
}

#pragma push_macro("glGetString")
#undef glGetString
const GLubyte *GL_APIENTRY glGetString (GLenum name) {
#pragma pop_macro("glGetString")
	wrapper->OnFunctionCall("glGetString");
	GLubyte* retval = (GLubyte*)GL_Impl::glGetString(name);
	wrapper->CheckGLError("glGetString");
	return retval;
}

#pragma push_macro("glGetTexParameterfv")
#undef glGetTexParameterfv
void GL_APIENTRY glGetTexParameterfv (GLenum target, GLenum pname, GLfloat *params) {
#pragma pop_macro("glGetTexParameterfv")
	wrapper->OnFunctionCall("glGetTexParameterfv");
	GL_Impl::glGetTexParameterfv(target, pname, params);
	wrapper->CheckGLError("glGetTexParameterfv");
}

#pragma push_macro("glGetTexParameteriv")
#undef glGetTexParameteriv
void GL_APIENTRY glGetTexParameteriv (GLenum target, GLenum pname, GLint *params) {
#pragma pop_macro("glGetTexParameteriv")
	wrapper->OnFunctionCall("glGetTexParameteriv");
	GL_Impl::glGetTexParameteriv(target, pname, params);
	wrapper->CheckGLError("glGetTexParameteriv");
}

#pragma push_macro("glGetUniformfv")
#undef glGetUniformfv
void GL_APIENTRY glGetUniformfv (GLuint program, GLint location, GLfloat *params) {
#pragma pop_macro("glGetUniformfv")
	wrapper->OnFunctionCall("glGetUniformfv");
	GL_Impl::glGetUniformfv(program, location, params);
	wrapper->CheckGLError("glGetUniformfv");
}

#pragma push_macro("glGetUniformiv")
#undef glGetUniformiv
void GL_APIENTRY glGetUniformiv (GLuint program, GLint location, GLint *params) {
#pragma pop_macro("glGetUniformiv")
	wrapper->OnFunctionCall("glGetUniformiv");
	GL_Impl::glGetUniformiv(program, location, params);
	wrapper->CheckGLError("glGetUniformiv");
}

#pragma push_macro("glGetUniformLocation")
#undef glGetUniformLocation
GLint GL_APIENTRY glGetUniformLocation (GLuint program, const GLchar *name) {
#pragma pop_macro("glGetUniformLocation")
	wrapper->OnFunctionCall("glGetUniformLocation");
	GLint retval = (GLint)GL_Impl::glGetUniformLocation(program, name);
	wrapper->CheckGLError("glGetUniformLocation");
	return retval;
}

#pragma push_macro("glGetVertexAttribfv")
#undef glGetVertexAttribfv
void GL_APIENTRY glGetVertexAttribfv (GLuint index, GLenum pname, GLfloat *params) {
#pragma pop_macro("glGetVertexAttribfv")
	wrapper->OnFunctionCall("glGetVertexAttribfv");
	GL_Impl::glGetVertexAttribfv(index, pname, params);
	wrapper->CheckGLError("glGetVertexAttribfv");
}

#pragma push_macro("glGetVertexAttribiv")
#undef glGetVertexAttribiv
void GL_APIENTRY glGetVertexAttribiv (GLuint index, GLenum pname, GLint *params) {
#pragma pop_macro("glGetVertexAttribiv")
	wrapper->OnFunctionCall("glGetVertexAttribiv");
	GL_Impl::glGetVertexAttribiv(index, pname, params);
	wrapper->CheckGLError("glGetVertexAttribiv");
}

#pragma push_macro("glGetVertexAttribPointerv")
#undef glGetVertexAttribPointerv
void GL_APIENTRY glGetVertexAttribPointerv (GLuint index, GLenum pname, void **pointer) {
#pragma pop_macro("glGetVertexAttribPointerv")
	wrapper->OnFunctionCall("glGetVertexAttribPointerv");
	GL_Impl::glGetVertexAttribPointerv(index, pname, pointer);
	wrapper->CheckGLError("glGetVertexAttribPointerv");
}

#pragma push_macro("glHint")
#undef glHint
void GL_APIENTRY glHint (GLenum target, GLenum mode) {
#pragma pop_macro("glHint")
	wrapper->OnFunctionCall("glHint");
	GL_Impl::glHint(target, mode);
	wrapper->CheckGLError("glHint");
}

#pragma push_macro("glIsBuffer")
#undef glIsBuffer
GLboolean GL_APIENTRY glIsBuffer (GLuint buffer) {
#pragma pop_macro("glIsBuffer")
	wrapper->OnFunctionCall("glIsBuffer");
	GLboolean retval = (GLboolean)GL_Impl::glIsBuffer(buffer);
	wrapper->CheckGLError("glIsBuffer");
	return retval;
}

#pragma push_macro("glIsEnabled")
#undef glIsEnabled
GLboolean GL_APIENTRY glIsEnabled (GLenum cap) {
#pragma pop_macro("glIsEnabled")
	wrapper->OnFunctionCall("glIsEnabled");
	GLboolean retval = (GLboolean)GL_Impl::glIsEnabled(cap);
	wrapper->CheckGLError("glIsEnabled");
	return retval;
}

#pragma push_macro("glIsFramebuffer")
#undef glIsFramebuffer
GLboolean GL_APIENTRY glIsFramebuffer (GLuint framebuffer) {
#pragma pop_macro("glIsFramebuffer")
	wrapper->OnFunctionCall("glIsFramebuffer");
	GLboolean retval = (GLboolean)GL_Impl::glIsFramebuffer(framebuffer);
	wrapper->CheckGLError("glIsFramebuffer");
	return retval;
}

#pragma push_macro("glIsProgram")
#undef glIsProgram
GLboolean GL_APIENTRY glIsProgram (GLuint program) {
#pragma pop_macro("glIsProgram")
	wrapper->OnFunctionCall("glIsProgram");
	GLboolean retval = (GLboolean)GL_Impl::glIsProgram(program);
	wrapper->CheckGLError("glIsProgram");
	return retval;
}

#pragma push_macro("glIsRenderbuffer")
#undef glIsRenderbuffer
GLboolean GL_APIENTRY glIsRenderbuffer (GLuint renderbuffer) {
#pragma pop_macro("glIsRenderbuffer")
	wrapper->OnFunctionCall("glIsRenderbuffer");
	GLboolean retval = (GLboolean)GL_Impl::glIsRenderbuffer(renderbuffer);
	wrapper->CheckGLError("glIsRenderbuffer");
	return retval;
}

#pragma push_macro("glIsShader")
#undef glIsShader
GLboolean GL_APIENTRY glIsShader (GLuint shader) {
#pragma pop_macro("glIsShader")
	wrapper->OnFunctionCall("glIsShader");
	GLboolean retval = (GLboolean)GL_Impl::glIsShader(shader);
	wrapper->CheckGLError("glIsShader");
	return retval;
}

#pragma push_macro("glIsTexture")
#undef glIsTexture
GLboolean GL_APIENTRY glIsTexture (GLuint texture) {
#pragma pop_macro("glIsTexture")
	wrapper->OnFunctionCall("glIsTexture");
	GLboolean retval = (GLboolean)GL_Impl::glIsTexture(texture);
	wrapper->CheckGLError("glIsTexture");
	return retval;
}

#pragma push_macro("glLineWidth")
#undef glLineWidth
void GL_APIENTRY glLineWidth (GLfloat width) {
#pragma pop_macro("glLineWidth")
	wrapper->OnFunctionCall("glLineWidth");
	GL_Impl::glLineWidth(width);
	wrapper->CheckGLError("glLineWidth");
}

#pragma push_macro("glLinkProgram")
#undef glLinkProgram
void GL_APIENTRY glLinkProgram (GLuint program) {
#pragma pop_macro("glLinkProgram")
	wrapper->OnFunctionCall("glLinkProgram");
	GL_Impl::glLinkProgram(program);
	wrapper->CheckGLError("glLinkProgram");
}

#pragma push_macro("glPixelStorei")
#undef glPixelStorei
void GL_APIENTRY glPixelStorei (GLenum pname, GLint param) {
#pragma pop_macro("glPixelStorei")
	wrapper->OnFunctionCall("glPixelStorei");
	GL_Impl::glPixelStorei(pname, param);
	wrapper->CheckGLError("glPixelStorei");
}

#pragma push_macro("glPolygonOffset")
#undef glPolygonOffset
void GL_APIENTRY glPolygonOffset (GLfloat factor, GLfloat units) {
#pragma pop_macro("glPolygonOffset")
	wrapper->OnFunctionCall("glPolygonOffset");
	GL_Impl::glPolygonOffset(factor, units);
	wrapper->CheckGLError("glPolygonOffset");
}

#pragma push_macro("glReadPixels")
#undef glReadPixels
void GL_APIENTRY glReadPixels (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void *pixels) {
#pragma pop_macro("glReadPixels")
	wrapper->OnFunctionCall("glReadPixels");
	GL_Impl::glReadPixels(x, y, width, height, format, type, pixels);
	wrapper->CheckGLError("glReadPixels");
}

#pragma push_macro("glReleaseShaderCompiler")
#undef glReleaseShaderCompiler
void GL_APIENTRY glReleaseShaderCompiler (void) {
#pragma pop_macro("glReleaseShaderCompiler")
	wrapper->OnFunctionCall("glReleaseShaderCompiler");
	GL_Impl::glReleaseShaderCompiler();
	wrapper->CheckGLError("glReleaseShaderCompiler");
}

#pragma push_macro("glRenderbufferStorage")
#undef glRenderbufferStorage
void GL_APIENTRY glRenderbufferStorage (GLenum target, GLenum internalformat, GLsizei width, GLsizei height) {
#pragma pop_macro("glRenderbufferStorage")
	wrapper->OnFunctionCall("glRenderbufferStorage");
	GL_Impl::glRenderbufferStorage(target, internalformat, width, height);
	wrapper->CheckGLError("glRenderbufferStorage");
}

#pragma push_macro("glSampleCoverage")
#undef glSampleCoverage
void GL_APIENTRY glSampleCoverage (GLfloat value, GLboolean invert) {
#pragma pop_macro("glSampleCoverage")
	wrapper->OnFunctionCall("glSampleCoverage");
	GL_Impl::glSampleCoverage(value, invert);
	wrapper->CheckGLError("glSampleCoverage");
}

#pragma push_macro("glScissor")
#undef glScissor
void GL_APIENTRY glScissor (GLint x, GLint y, GLsizei width, GLsizei height) {
#pragma pop_macro("glScissor")
	wrapper->OnFunctionCall("glScissor");
	GL_Impl::glScissor(x, y, width, height);
	wrapper->CheckGLError("glScissor");
}

#pragma push_macro("glShaderBinary")
#undef glShaderBinary
void GL_APIENTRY glShaderBinary (GLsizei count, const GLuint *shaders, GLenum binaryformat, const void *binary, GLsizei length) {
#pragma pop_macro("glShaderBinary")
	wrapper->OnFunctionCall("glShaderBinary");
	GL_Impl::glShaderBinary(count, shaders, binaryformat, binary, length);
	wrapper->CheckGLError("glShaderBinary");
}

#pragma push_macro("glShaderSource")
#undef glShaderSource
void GL_APIENTRY glShaderSource (GLuint shader, GLsizei count, const GLchar *const*string, const GLint *length) {
#pragma pop_macro("glShaderSource")
	wrapper->OnFunctionCall("glShaderSource");
	GL_Impl::glShaderSource(shader, count, (const GL_Impl::GLchar **)string, length);
	wrapper->CheckGLError("glShaderSource");
}

#pragma push_macro("glStencilFunc")
#undef glStencilFunc
void GL_APIENTRY glStencilFunc (GLenum func, GLint ref, GLuint mask) {
#pragma pop_macro("glStencilFunc")
	wrapper->OnFunctionCall("glStencilFunc");
	GL_Impl::glStencilFunc(func, ref, mask);
	wrapper->CheckGLError("glStencilFunc");
}

#pragma push_macro("glStencilFuncSeparate")
#undef glStencilFuncSeparate
void GL_APIENTRY glStencilFuncSeparate (GLenum face, GLenum func, GLint ref, GLuint mask) {
#pragma pop_macro("glStencilFuncSeparate")
	wrapper->OnFunctionCall("glStencilFuncSeparate");
	GL_Impl::glStencilFuncSeparate(face, func, ref, mask);
	wrapper->CheckGLError("glStencilFuncSeparate");
}

#pragma push_macro("glStencilMask")
#undef glStencilMask
void GL_APIENTRY glStencilMask (GLuint mask) {
#pragma pop_macro("glStencilMask")
	wrapper->OnFunctionCall("glStencilMask");
	GL_Impl::glStencilMask(mask);
	wrapper->CheckGLError("glStencilMask");
}

#pragma push_macro("glStencilMaskSeparate")
#undef glStencilMaskSeparate
void GL_APIENTRY glStencilMaskSeparate (GLenum face, GLuint mask) {
#pragma pop_macro("glStencilMaskSeparate")
	wrapper->OnFunctionCall("glStencilMaskSeparate");
	GL_Impl::glStencilMaskSeparate(face, mask);
	wrapper->CheckGLError("glStencilMaskSeparate");
}

#pragma push_macro("glStencilOp")
#undef glStencilOp
void GL_APIENTRY glStencilOp (GLenum fail, GLenum zfail, GLenum zpass) {
#pragma pop_macro("glStencilOp")
	wrapper->OnFunctionCall("glStencilOp");
	GL_Impl::glStencilOp(fail, zfail, zpass);
	wrapper->CheckGLError("glStencilOp");
}

#pragma push_macro("glStencilOpSeparate")
#undef glStencilOpSeparate
void GL_APIENTRY glStencilOpSeparate (GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass) {
#pragma pop_macro("glStencilOpSeparate")
	wrapper->OnFunctionCall("glStencilOpSeparate");
	GL_Impl::glStencilOpSeparate(face, sfail, dpfail, dppass);
	wrapper->CheckGLError("glStencilOpSeparate");
}

#pragma push_macro("glTexImage2D")
#undef glTexImage2D
void GL_APIENTRY glTexImage2D (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels) {
#pragma pop_macro("glTexImage2D")
	wrapper->OnFunctionCall("glTexImage2D");
	GL_Impl::glTexImage2D(target, level, internalformat, width, height, border, format, type, pixels);
	wrapper->CheckGLError("glTexImage2D");
}

#pragma push_macro("glTexParameterf")
#undef glTexParameterf
void GL_APIENTRY glTexParameterf (GLenum target, GLenum pname, GLfloat param) {
#pragma pop_macro("glTexParameterf")
	wrapper->OnFunctionCall("glTexParameterf");
	GL_Impl::glTexParameterf(target, pname, param);
	wrapper->CheckGLError("glTexParameterf");
}

#pragma push_macro("glTexParameterfv")
#undef glTexParameterfv
void GL_APIENTRY glTexParameterfv (GLenum target, GLenum pname, const GLfloat *params) {
#pragma pop_macro("glTexParameterfv")
	wrapper->OnFunctionCall("glTexParameterfv");
	GL_Impl::glTexParameterfv(target, pname, params);
	wrapper->CheckGLError("glTexParameterfv");
}

#pragma push_macro("glTexParameteri")
#undef glTexParameteri
void GL_APIENTRY glTexParameteri (GLenum target, GLenum pname, GLint param) {
#pragma pop_macro("glTexParameteri")
	wrapper->OnFunctionCall("glTexParameteri");
	GL_Impl::glTexParameteri(target, pname, param);
	wrapper->CheckGLError("glTexParameteri");
}

#pragma push_macro("glTexParameteriv")
#undef glTexParameteriv
void GL_APIENTRY glTexParameteriv (GLenum target, GLenum pname, const GLint *params) {
#pragma pop_macro("glTexParameteriv")
	wrapper->OnFunctionCall("glTexParameteriv");
	GL_Impl::glTexParameteriv(target, pname, params);
	wrapper->CheckGLError("glTexParameteriv");
}

#pragma push_macro("glTexSubImage2D")
#undef glTexSubImage2D
void GL_APIENTRY glTexSubImage2D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels) {
#pragma pop_macro("glTexSubImage2D")
	wrapper->OnFunctionCall("glTexSubImage2D");
	GL_Impl::glTexSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels);
	wrapper->CheckGLError("glTexSubImage2D");
}

#pragma push_macro("glUniform1f")
#undef glUniform1f
void GL_APIENTRY glUniform1f (GLint location, GLfloat v0) {
#pragma pop_macro("glUniform1f")
	wrapper->OnFunctionCall("glUniform1f");
	GL_Impl::glUniform1f(location, v0);
	wrapper->CheckGLError("glUniform1f");
}

#pragma push_macro("glUniform1fv")
#undef glUniform1fv
void GL_APIENTRY glUniform1fv (GLint location, GLsizei count, const GLfloat *value) {
#pragma pop_macro("glUniform1fv")
	wrapper->OnFunctionCall("glUniform1fv");
	GL_Impl::glUniform1fv(location, count, value);
	wrapper->CheckGLError("glUniform1fv");
}

#pragma push_macro("glUniform1i")
#undef glUniform1i
void GL_APIENTRY glUniform1i (GLint location, GLint v0) {
#pragma pop_macro("glUniform1i")
	wrapper->OnFunctionCall("glUniform1i");
	GL_Impl::glUniform1i(location, v0);
	wrapper->CheckGLError("glUniform1i");
}

#pragma push_macro("glUniform1iv")
#undef glUniform1iv
void GL_APIENTRY glUniform1iv (GLint location, GLsizei count, const GLint *value) {
#pragma pop_macro("glUniform1iv")
	wrapper->OnFunctionCall("glUniform1iv");
	GL_Impl::glUniform1iv(location, count, value);
	wrapper->CheckGLError("glUniform1iv");
}

#pragma push_macro("glUniform2f")
#undef glUniform2f
void GL_APIENTRY glUniform2f (GLint location, GLfloat v0, GLfloat v1) {
#pragma pop_macro("glUniform2f")
	wrapper->OnFunctionCall("glUniform2f");
	GL_Impl::glUniform2f(location, v0, v1);
	wrapper->CheckGLError("glUniform2f");
}

#pragma push_macro("glUniform2fv")
#undef glUniform2fv
void GL_APIENTRY glUniform2fv (GLint location, GLsizei count, const GLfloat *value) {
#pragma pop_macro("glUniform2fv")
	wrapper->OnFunctionCall("glUniform2fv");
	GL_Impl::glUniform2fv(location, count, value);
	wrapper->CheckGLError("glUniform2fv");
}

#pragma push_macro("glUniform2i")
#undef glUniform2i
void GL_APIENTRY glUniform2i (GLint location, GLint v0, GLint v1) {
#pragma pop_macro("glUniform2i")
	wrapper->OnFunctionCall("glUniform2i");
	GL_Impl::glUniform2i(location, v0, v1);
	wrapper->CheckGLError("glUniform2i");
}

#pragma push_macro("glUniform2iv")
#undef glUniform2iv
void GL_APIENTRY glUniform2iv (GLint location, GLsizei count, const GLint *value) {
#pragma pop_macro("glUniform2iv")
	wrapper->OnFunctionCall("glUniform2iv");
	GL_Impl::glUniform2iv(location, count, value);
	wrapper->CheckGLError("glUniform2iv");
}

#pragma push_macro("glUniform3f")
#undef glUniform3f
void GL_APIENTRY glUniform3f (GLint location, GLfloat v0, GLfloat v1, GLfloat v2) {
#pragma pop_macro("glUniform3f")
	wrapper->OnFunctionCall("glUniform3f");
	GL_Impl::glUniform3f(location, v0, v1, v2);
	wrapper->CheckGLError("glUniform3f");
}

#pragma push_macro("glUniform3fv")
#undef glUniform3fv
void GL_APIENTRY glUniform3fv (GLint location, GLsizei count, const GLfloat *value) {
#pragma pop_macro("glUniform3fv")
	wrapper->OnFunctionCall("glUniform3fv");
	GL_Impl::glUniform3fv(location, count, value);
	wrapper->CheckGLError("glUniform3fv");
}

#pragma push_macro("glUniform3i")
#undef glUniform3i
void GL_APIENTRY glUniform3i (GLint location, GLint v0, GLint v1, GLint v2) {
#pragma pop_macro("glUniform3i")
	wrapper->OnFunctionCall("glUniform3i");
	GL_Impl::glUniform3i(location, v0, v1, v2);
	wrapper->CheckGLError("glUniform3i");
}

#pragma push_macro("glUniform3iv")
#undef glUniform3iv
void GL_APIENTRY glUniform3iv (GLint location, GLsizei count, const GLint *value) {
#pragma pop_macro("glUniform3iv")
	wrapper->OnFunctionCall("glUniform3iv");
	GL_Impl::glUniform3iv(location, count, value);
	wrapper->CheckGLError("glUniform3iv");
}

#pragma push_macro("glUniform4f")
#undef glUniform4f
void GL_APIENTRY glUniform4f (GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3) {
#pragma pop_macro("glUniform4f")
	wrapper->OnFunctionCall("glUniform4f");
	GL_Impl::glUniform4f(location, v0, v1, v2, v3);
	wrapper->CheckGLError("glUniform4f");
}

#pragma push_macro("glUniform4fv")
#undef glUniform4fv
void GL_APIENTRY glUniform4fv (GLint location, GLsizei count, const GLfloat *value) {
#pragma pop_macro("glUniform4fv")
	wrapper->OnFunctionCall("glUniform4fv");
	GL_Impl::glUniform4fv(location, count, value);
	wrapper->CheckGLError("glUniform4fv");
}

#pragma push_macro("glUniform4i")
#undef glUniform4i
void GL_APIENTRY glUniform4i (GLint location, GLint v0, GLint v1, GLint v2, GLint v3) {
#pragma pop_macro("glUniform4i")
	wrapper->OnFunctionCall("glUniform4i");
	GL_Impl::glUniform4i(location, v0, v1, v2, v3);
	wrapper->CheckGLError("glUniform4i");
}

#pragma push_macro("glUniform4iv")
#undef glUniform4iv
void GL_APIENTRY glUniform4iv (GLint location, GLsizei count, const GLint *value) {
#pragma pop_macro("glUniform4iv")
	wrapper->OnFunctionCall("glUniform4iv");
	GL_Impl::glUniform4iv(location, count, value);
	wrapper->CheckGLError("glUniform4iv");
}

#pragma push_macro("glUniformMatrix2fv")
#undef glUniformMatrix2fv
void GL_APIENTRY glUniformMatrix2fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) {
#pragma pop_macro("glUniformMatrix2fv")
	wrapper->OnFunctionCall("glUniformMatrix2fv");
	GL_Impl::glUniformMatrix2fv(location, count, transpose, value);
	wrapper->CheckGLError("glUniformMatrix2fv");
}

#pragma push_macro("glUniformMatrix3fv")
#undef glUniformMatrix3fv
void GL_APIENTRY glUniformMatrix3fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) {
#pragma pop_macro("glUniformMatrix3fv")
	wrapper->OnFunctionCall("glUniformMatrix3fv");
	GL_Impl::glUniformMatrix3fv(location, count, transpose, value);
	wrapper->CheckGLError("glUniformMatrix3fv");
}

#pragma push_macro("glUniformMatrix4fv")
#undef glUniformMatrix4fv
void GL_APIENTRY glUniformMatrix4fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) {
#pragma pop_macro("glUniformMatrix4fv")
	wrapper->OnFunctionCall("glUniformMatrix4fv");
	GL_Impl::glUniformMatrix4fv(location, count, transpose, value);
	wrapper->CheckGLError("glUniformMatrix4fv");
}

#pragma push_macro("glUseProgram")
#undef glUseProgram
void GL_APIENTRY glUseProgram (GLuint program) {
#pragma pop_macro("glUseProgram")
	wrapper->OnFunctionCall("glUseProgram");
	GL_Impl::glUseProgram(program);
	wrapper->CheckGLError("glUseProgram");
}

#pragma push_macro("glValidateProgram")
#undef glValidateProgram
void GL_APIENTRY glValidateProgram (GLuint program) {
#pragma pop_macro("glValidateProgram")
	wrapper->OnFunctionCall("glValidateProgram");
	GL_Impl::glValidateProgram(program);
	wrapper->CheckGLError("glValidateProgram");
}

#pragma push_macro("glVertexAttrib1f")
#undef glVertexAttrib1f
void GL_APIENTRY glVertexAttrib1f (GLuint index, GLfloat x) {
#pragma pop_macro("glVertexAttrib1f")
	wrapper->OnFunctionCall("glVertexAttrib1f");
	GL_Impl::glVertexAttrib1f(index, x);
	wrapper->CheckGLError("glVertexAttrib1f");
}

#pragma push_macro("glVertexAttrib1fv")
#undef glVertexAttrib1fv
void GL_APIENTRY glVertexAttrib1fv (GLuint index, const GLfloat *v) {
#pragma pop_macro("glVertexAttrib1fv")
	wrapper->OnFunctionCall("glVertexAttrib1fv");
	GL_Impl::glVertexAttrib1fv(index, v);
	wrapper->CheckGLError("glVertexAttrib1fv");
}

#pragma push_macro("glVertexAttrib2f")
#undef glVertexAttrib2f
void GL_APIENTRY glVertexAttrib2f (GLuint index, GLfloat x, GLfloat y) {
#pragma pop_macro("glVertexAttrib2f")
	wrapper->OnFunctionCall("glVertexAttrib2f");
	GL_Impl::glVertexAttrib2f(index, x, y);
	wrapper->CheckGLError("glVertexAttrib2f");
}

#pragma push_macro("glVertexAttrib2fv")
#undef glVertexAttrib2fv
void GL_APIENTRY glVertexAttrib2fv (GLuint index, const GLfloat *v) {
#pragma pop_macro("glVertexAttrib2fv")
	wrapper->OnFunctionCall("glVertexAttrib2fv");
	GL_Impl::glVertexAttrib2fv(index, v);
	wrapper->CheckGLError("glVertexAttrib2fv");
}

#pragma push_macro("glVertexAttrib3f")
#undef glVertexAttrib3f
void GL_APIENTRY glVertexAttrib3f (GLuint index, GLfloat x, GLfloat y, GLfloat z) {
#pragma pop_macro("glVertexAttrib3f")
	wrapper->OnFunctionCall("glVertexAttrib3f");
	GL_Impl::glVertexAttrib3f(index, x, y, z);
	wrapper->CheckGLError("glVertexAttrib3f");
}

#pragma push_macro("glVertexAttrib3fv")
#undef glVertexAttrib3fv
void GL_APIENTRY glVertexAttrib3fv (GLuint index, const GLfloat *v) {
#pragma pop_macro("glVertexAttrib3fv")
	wrapper->OnFunctionCall("glVertexAttrib3fv");
	GL_Impl::glVertexAttrib3fv(index, v);
	wrapper->CheckGLError("glVertexAttrib3fv");
}

#pragma push_macro("glVertexAttrib4f")
#undef glVertexAttrib4f
void GL_APIENTRY glVertexAttrib4f (GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w) {
#pragma pop_macro("glVertexAttrib4f")
	wrapper->OnFunctionCall("glVertexAttrib4f");
	GL_Impl::glVertexAttrib4f(index, x, y, z, w);
	wrapper->CheckGLError("glVertexAttrib4f");
}

#pragma push_macro("glVertexAttrib4fv")
#undef glVertexAttrib4fv
void GL_APIENTRY glVertexAttrib4fv (GLuint index, const GLfloat *v) {
#pragma pop_macro("glVertexAttrib4fv")
	wrapper->OnFunctionCall("glVertexAttrib4fv");
	GL_Impl::glVertexAttrib4fv(index, v);
	wrapper->CheckGLError("glVertexAttrib4fv");
}

#pragma push_macro("glVertexAttribPointer")
#undef glVertexAttribPointer
void GL_APIENTRY glVertexAttribPointer (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer) {
#pragma pop_macro("glVertexAttribPointer")
	wrapper->OnFunctionCall("glVertexAttribPointer");
	GL_Impl::glVertexAttribPointer(index, size, type, normalized, stride, pointer);
	wrapper->CheckGLError("glVertexAttribPointer");
}

#pragma push_macro("glViewport")
#undef glViewport
void GL_APIENTRY glViewport (GLint x, GLint y, GLsizei width, GLsizei height) {
#pragma pop_macro("glViewport")
	wrapper->OnFunctionCall("glViewport");
	GL_Impl::glViewport(x, y, width, height);
	wrapper->CheckGLError("glViewport");
}

#pragma push_macro("glReadBuffer")
#undef glReadBuffer
void GL_APIENTRY glReadBuffer (GLenum mode) {
#pragma pop_macro("glReadBuffer")
	wrapper->OnFunctionCall("glReadBuffer");
	GL_Impl::glReadBuffer(mode);
	wrapper->CheckGLError("glReadBuffer");
}

#pragma push_macro("glDrawRangeElements")
#undef glDrawRangeElements
void GL_APIENTRY glDrawRangeElements (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void *indices) {
#pragma pop_macro("glDrawRangeElements")
	wrapper->OnFunctionCall("glDrawRangeElements");
	wrapper->OnPreDrawCall(mode, count, 1);
	GL_Impl::glDrawRangeElements(mode, start, end, count, type, indices);
	wrapper->CheckGLError("glDrawRangeElements");
	wrapper->OnPostDrawCall();
}

#pragma push_macro("glTexImage3D")
#undef glTexImage3D
void GL_APIENTRY glTexImage3D (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void *pixels) {
#pragma pop_macro("glTexImage3D")
	wrapper->OnFunctionCall("glTexImage3D");
	GL_Impl::glTexImage3D(target, level, internalformat, width, height, depth, border, format, type, pixels);
	wrapper->CheckGLError("glTexImage3D");
}

#pragma push_macro("glTexSubImage3D")
#undef glTexSubImage3D
void GL_APIENTRY glTexSubImage3D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void *pixels) {
#pragma pop_macro("glTexSubImage3D")
	wrapper->OnFunctionCall("glTexSubImage3D");
	GL_Impl::glTexSubImage3D(target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels);
	wrapper->CheckGLError("glTexSubImage3D");
}

#pragma push_macro("glCopyTexSubImage3D")
#undef glCopyTexSubImage3D
void GL_APIENTRY glCopyTexSubImage3D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height) {
#pragma pop_macro("glCopyTexSubImage3D")
	wrapper->OnFunctionCall("glCopyTexSubImage3D");
	GL_Impl::glCopyTexSubImage3D(target, level, xoffset, yoffset, zoffset, x, y, width, height);
	wrapper->CheckGLError("glCopyTexSubImage3D");
}

#pragma push_macro("glCompressedTexImage3D")
#undef glCompressedTexImage3D
void GL_APIENTRY glCompressedTexImage3D (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const void *data) {
#pragma pop_macro("glCompressedTexImage3D")
	wrapper->OnFunctionCall("glCompressedTexImage3D");
	GL_Impl::glCompressedTexImage3D(target, level, internalformat, width, height, depth, border, imageSize, data);
	wrapper->CheckGLError("glCompressedTexImage3D");
}

#pragma push_macro("glCompressedTexSubImage3D")
#undef glCompressedTexSubImage3D
void GL_APIENTRY glCompressedTexSubImage3D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void *data) {
#pragma pop_macro("glCompressedTexSubImage3D")
	wrapper->OnFunctionCall("glCompressedTexSubImage3D");
	GL_Impl::glCompressedTexSubImage3D(target, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, data);
	wrapper->CheckGLError("glCompressedTexSubImage3D");
}

#pragma push_macro("glGenQueries")
#undef glGenQueries
void GL_APIENTRY glGenQueries (GLsizei n, GLuint *ids) {
#pragma pop_macro("glGenQueries")
	wrapper->OnFunctionCall("glGenQueries");
	GL_Impl::glGenQueries(n, ids);
	wrapper->CheckGLError("glGenQueries");
	wrapper->OnGenObject(GLWrapperImpl::TYPE_QUERY, n, ids);
}

#pragma push_macro("glDeleteQueries")
#undef glDeleteQueries
void GL_APIENTRY glDeleteQueries (GLsizei n, const GLuint *ids) {
#pragma pop_macro("glDeleteQueries")
	wrapper->OnFunctionCall("glDeleteQueries");
	GL_Impl::glDeleteQueries(n, ids);
	wrapper->CheckGLError("glDeleteQueries");
	wrapper->OnDeleteObject(GLWrapperImpl::TYPE_QUERY, n, ids);
}

#pragma push_macro("glIsQuery")
#undef glIsQuery
GLboolean GL_APIENTRY glIsQuery (GLuint id) {
#pragma pop_macro("glIsQuery")
	wrapper->OnFunctionCall("glIsQuery");
	GLboolean retval = (GLboolean)GL_Impl::glIsQuery(id);
	wrapper->CheckGLError("glIsQuery");
	return retval;
}

#pragma push_macro("glBeginQuery")
#undef glBeginQuery
void GL_APIENTRY glBeginQuery (GLenum target, GLuint id) {
#pragma pop_macro("glBeginQuery")
	wrapper->OnFunctionCall("glBeginQuery");
	GL_Impl::glBeginQuery(target, id);
	wrapper->CheckGLError("glBeginQuery");
}

#pragma push_macro("glEndQuery")
#undef glEndQuery
void GL_APIENTRY glEndQuery (GLenum target) {
#pragma pop_macro("glEndQuery")
	wrapper->OnFunctionCall("glEndQuery");
	GL_Impl::glEndQuery(target);
	wrapper->CheckGLError("glEndQuery");
}

#pragma push_macro("glGetQueryiv")
#undef glGetQueryiv
void GL_APIENTRY glGetQueryiv (GLenum target, GLenum pname, GLint *params) {
#pragma pop_macro("glGetQueryiv")
	wrapper->OnFunctionCall("glGetQueryiv");
	GL_Impl::glGetQueryiv(target, pname, params);
	wrapper->CheckGLError("glGetQueryiv");
}

#pragma push_macro("glGetQueryObjectuiv")
#undef glGetQueryObjectuiv
void GL_APIENTRY glGetQueryObjectuiv (GLuint id, GLenum pname, GLuint *params) {
#pragma pop_macro("glGetQueryObjectuiv")
	wrapper->OnFunctionCall("glGetQueryObjectuiv");
	GL_Impl::glGetQueryObjectuiv(id, pname, params);
	wrapper->CheckGLError("glGetQueryObjectuiv");
}

#pragma push_macro("glUnmapBuffer")
#undef glUnmapBuffer
GLboolean GL_APIENTRY glUnmapBuffer (GLenum target) {
#pragma pop_macro("glUnmapBuffer")
	wrapper->OnFunctionCall("glUnmapBuffer");
	GLboolean retval = (GLboolean)GL_Impl::glUnmapBuffer(target);
	wrapper->CheckGLError("glUnmapBuffer");
	return retval;
}

#pragma push_macro("glGetBufferPointerv")
#undef glGetBufferPointerv
void GL_APIENTRY glGetBufferPointerv (GLenum target, GLenum pname, void **params) {
#pragma pop_macro("glGetBufferPointerv")
	wrapper->OnFunctionCall("glGetBufferPointerv");
	GL_Impl::glGetBufferPointerv(target, pname, params);
	wrapper->CheckGLError("glGetBufferPointerv");
}

#pragma push_macro("glDrawBuffers")
#undef glDrawBuffers
void GL_APIENTRY glDrawBuffers (GLsizei n, const GLenum *bufs) {
#pragma pop_macro("glDrawBuffers")
	wrapper->OnFunctionCall("glDrawBuffers");
	GL_Impl::glDrawBuffers(n, bufs);
	wrapper->CheckGLError("glDrawBuffers");
}

#pragma push_macro("glUniformMatrix2x3fv")
#undef glUniformMatrix2x3fv
void GL_APIENTRY glUniformMatrix2x3fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) {
#pragma pop_macro("glUniformMatrix2x3fv")
	wrapper->OnFunctionCall("glUniformMatrix2x3fv");
	GL_Impl::glUniformMatrix2x3fv(location, count, transpose, value);
	wrapper->CheckGLError("glUniformMatrix2x3fv");
}

#pragma push_macro("glUniformMatrix3x2fv")
#undef glUniformMatrix3x2fv
void GL_APIENTRY glUniformMatrix3x2fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) {
#pragma pop_macro("glUniformMatrix3x2fv")
	wrapper->OnFunctionCall("glUniformMatrix3x2fv");
	GL_Impl::glUniformMatrix3x2fv(location, count, transpose, value);
	wrapper->CheckGLError("glUniformMatrix3x2fv");
}

#pragma push_macro("glUniformMatrix2x4fv")
#undef glUniformMatrix2x4fv
void GL_APIENTRY glUniformMatrix2x4fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) {
#pragma pop_macro("glUniformMatrix2x4fv")
	wrapper->OnFunctionCall("glUniformMatrix2x4fv");
	GL_Impl::glUniformMatrix2x4fv(location, count, transpose, value);
	wrapper->CheckGLError("glUniformMatrix2x4fv");
}

#pragma push_macro("glUniformMatrix4x2fv")
#undef glUniformMatrix4x2fv
void GL_APIENTRY glUniformMatrix4x2fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) {
#pragma pop_macro("glUniformMatrix4x2fv")
	wrapper->OnFunctionCall("glUniformMatrix4x2fv");
	GL_Impl::glUniformMatrix4x2fv(location, count, transpose, value);
	wrapper->CheckGLError("glUniformMatrix4x2fv");
}

#pragma push_macro("glUniformMatrix3x4fv")
#undef glUniformMatrix3x4fv
void GL_APIENTRY glUniformMatrix3x4fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) {
#pragma pop_macro("glUniformMatrix3x4fv")
	wrapper->OnFunctionCall("glUniformMatrix3x4fv");
	GL_Impl::glUniformMatrix3x4fv(location, count, transpose, value);
	wrapper->CheckGLError("glUniformMatrix3x4fv");
}

#pragma push_macro("glUniformMatrix4x3fv")
#undef glUniformMatrix4x3fv
void GL_APIENTRY glUniformMatrix4x3fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) {
#pragma pop_macro("glUniformMatrix4x3fv")
	wrapper->OnFunctionCall("glUniformMatrix4x3fv");
	GL_Impl::glUniformMatrix4x3fv(location, count, transpose, value);
	wrapper->CheckGLError("glUniformMatrix4x3fv");
}

#pragma push_macro("glBlitFramebuffer")
#undef glBlitFramebuffer
void GL_APIENTRY glBlitFramebuffer (GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter) {
#pragma pop_macro("glBlitFramebuffer")
	wrapper->OnFunctionCall("glBlitFramebuffer");
	GL_Impl::glBlitFramebuffer(srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter);
	wrapper->CheckGLError("glBlitFramebuffer");
}

#pragma push_macro("glRenderbufferStorageMultisample")
#undef glRenderbufferStorageMultisample
void GL_APIENTRY glRenderbufferStorageMultisample (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height) {
#pragma pop_macro("glRenderbufferStorageMultisample")
	wrapper->OnFunctionCall("glRenderbufferStorageMultisample");
	GL_Impl::glRenderbufferStorageMultisample(target, samples, internalformat, width, height);
	wrapper->CheckGLError("glRenderbufferStorageMultisample");
}

#pragma push_macro("glFramebufferTextureLayer")
#undef glFramebufferTextureLayer
void GL_APIENTRY glFramebufferTextureLayer (GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer) {
#pragma pop_macro("glFramebufferTextureLayer")
	wrapper->OnFunctionCall("glFramebufferTextureLayer");
	GL_Impl::glFramebufferTextureLayer(target, attachment, texture, level, layer);
	wrapper->CheckGLError("glFramebufferTextureLayer");
}

#pragma push_macro("glMapBufferRange")
#undef glMapBufferRange
void *GL_APIENTRY glMapBufferRange (GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access) {
#pragma pop_macro("glMapBufferRange")
	wrapper->OnFunctionCall("glMapBufferRange");
	void* retval = (void*)GL_Impl::glMapBufferRange(target, offset, length, access);
	wrapper->CheckGLError("glMapBufferRange");
	return retval;
}

#pragma push_macro("glFlushMappedBufferRange")
#undef glFlushMappedBufferRange
void GL_APIENTRY glFlushMappedBufferRange (GLenum target, GLintptr offset, GLsizeiptr length) {
#pragma pop_macro("glFlushMappedBufferRange")
	wrapper->OnFunctionCall("glFlushMappedBufferRange");
	GL_Impl::glFlushMappedBufferRange(target, offset, length);
	wrapper->CheckGLError("glFlushMappedBufferRange");
}

#pragma push_macro("glBindVertexArray")
#undef glBindVertexArray
void GL_APIENTRY glBindVertexArray (GLuint array) {
#pragma pop_macro("glBindVertexArray")
	wrapper->OnFunctionCall("glBindVertexArray");
	GL_Impl::glBindVertexArray(array);
	wrapper->CheckGLError("glBindVertexArray");
}

#pragma push_macro("glDeleteVertexArrays")
#undef glDeleteVertexArrays
void GL_APIENTRY glDeleteVertexArrays (GLsizei n, const GLuint *arrays) {
#pragma pop_macro("glDeleteVertexArrays")
	wrapper->OnFunctionCall("glDeleteVertexArrays");
	GL_Impl::glDeleteVertexArrays(n, arrays);
	wrapper->CheckGLError("glDeleteVertexArrays");
	wrapper->OnDeleteObject(GLWrapperImpl::TYPE_VERTEX_ARRAY, n, arrays);
}

#pragma push_macro("glGenVertexArrays")
#undef glGenVertexArrays
void GL_APIENTRY glGenVertexArrays (GLsizei n, GLuint *arrays) {
#pragma pop_macro("glGenVertexArrays")
	wrapper->OnFunctionCall("glGenVertexArrays");
	GL_Impl::glGenVertexArrays(n, arrays);
	wrapper->CheckGLError("glGenVertexArrays");
	wrapper->OnGenObject(GLWrapperImpl::TYPE_VERTEX_ARRAY, n, arrays);
}

#pragma push_macro("glIsVertexArray")
#undef glIsVertexArray
GLboolean GL_APIENTRY glIsVertexArray (GLuint array) {
#pragma pop_macro("glIsVertexArray")
	wrapper->OnFunctionCall("glIsVertexArray");
	GLboolean retval = (GLboolean)GL_Impl::glIsVertexArray(array);
	wrapper->CheckGLError("glIsVertexArray");
	return retval;
}

#pragma push_macro("glGetIntegeri_v")
#undef glGetIntegeri_v
void GL_APIENTRY glGetIntegeri_v (GLenum target, GLuint index, GLint *data) {
#pragma pop_macro("glGetIntegeri_v")
	wrapper->OnFunctionCall("glGetIntegeri_v");
	GL_Impl::glGetIntegeri_v(target, index, data);
	wrapper->CheckGLError("glGetIntegeri_v");
}

#pragma push_macro("glBeginTransformFeedback")
#undef glBeginTransformFeedback
void GL_APIENTRY glBeginTransformFeedback (GLenum primitiveMode) {
#pragma pop_macro("glBeginTransformFeedback")
	wrapper->OnFunctionCall("glBeginTransformFeedback");
	GL_Impl::glBeginTransformFeedback(primitiveMode);
	wrapper->CheckGLError("glBeginTransformFeedback");
}

#pragma push_macro("glEndTransformFeedback")
#undef glEndTransformFeedback
void GL_APIENTRY glEndTransformFeedback (void) {
#pragma pop_macro("glEndTransformFeedback")
	wrapper->OnFunctionCall("glEndTransformFeedback");
	GL_Impl::glEndTransformFeedback();
	wrapper->CheckGLError("glEndTransformFeedback");
}

#pragma push_macro("glBindBufferRange")
#undef glBindBufferRange
void GL_APIENTRY glBindBufferRange (GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size) {
#pragma pop_macro("glBindBufferRange")
	wrapper->OnFunctionCall("glBindBufferRange");
	GL_Impl::glBindBufferRange(target, index, buffer, offset, size);
	wrapper->CheckGLError("glBindBufferRange");
}

#pragma push_macro("glBindBufferBase")
#undef glBindBufferBase
void GL_APIENTRY glBindBufferBase (GLenum target, GLuint index, GLuint buffer) {
#pragma pop_macro("glBindBufferBase")
	wrapper->OnFunctionCall("glBindBufferBase");
	GL_Impl::glBindBufferBase(target, index, buffer);
	wrapper->CheckGLError("glBindBufferBase");
}

#pragma push_macro("glTransformFeedbackVaryings")
#undef glTransformFeedbackVaryings
void GL_APIENTRY glTransformFeedbackVaryings (GLuint program, GLsizei count, const GLchar *const*varyings, GLenum bufferMode) {
#pragma pop_macro("glTransformFeedbackVaryings")
	wrapper->OnFunctionCall("glTransformFeedbackVaryings");
	GL_Impl::glTransformFeedbackVaryings(program, count, (const GL_Impl::GLchar **)varyings, bufferMode);
	wrapper->CheckGLError("glTransformFeedbackVaryings");
}

#pragma push_macro("glGetTransformFeedbackVarying")
#undef glGetTransformFeedbackVarying
void GL_APIENTRY glGetTransformFeedbackVarying (GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLsizei *size, GLenum *type, GLchar *name) {
#pragma pop_macro("glGetTransformFeedbackVarying")
	wrapper->OnFunctionCall("glGetTransformFeedbackVarying");
	GL_Impl::glGetTransformFeedbackVarying(program, index, bufSize, length, size, type, name);
	wrapper->CheckGLError("glGetTransformFeedbackVarying");
}

#pragma push_macro("glVertexAttribIPointer")
#undef glVertexAttribIPointer
void GL_APIENTRY glVertexAttribIPointer (GLuint index, GLint size, GLenum type, GLsizei stride, const void *pointer) {
#pragma pop_macro("glVertexAttribIPointer")
	wrapper->OnFunctionCall("glVertexAttribIPointer");
	GL_Impl::glVertexAttribIPointer(index, size, type, stride, pointer);
	wrapper->CheckGLError("glVertexAttribIPointer");
}

#pragma push_macro("glGetVertexAttribIiv")
#undef glGetVertexAttribIiv
void GL_APIENTRY glGetVertexAttribIiv (GLuint index, GLenum pname, GLint *params) {
#pragma pop_macro("glGetVertexAttribIiv")
	wrapper->OnFunctionCall("glGetVertexAttribIiv");
	GL_Impl::glGetVertexAttribIiv(index, pname, params);
	wrapper->CheckGLError("glGetVertexAttribIiv");
}

#pragma push_macro("glGetVertexAttribIuiv")
#undef glGetVertexAttribIuiv
void GL_APIENTRY glGetVertexAttribIuiv (GLuint index, GLenum pname, GLuint *params) {
#pragma pop_macro("glGetVertexAttribIuiv")
	wrapper->OnFunctionCall("glGetVertexAttribIuiv");
	GL_Impl::glGetVertexAttribIuiv(index, pname, params);
	wrapper->CheckGLError("glGetVertexAttribIuiv");
}

#pragma push_macro("glVertexAttribI4i")
#undef glVertexAttribI4i
void GL_APIENTRY glVertexAttribI4i (GLuint index, GLint x, GLint y, GLint z, GLint w) {
#pragma pop_macro("glVertexAttribI4i")
	wrapper->OnFunctionCall("glVertexAttribI4i");
	GL_Impl::glVertexAttribI4i(index, x, y, z, w);
	wrapper->CheckGLError("glVertexAttribI4i");
}

#pragma push_macro("glVertexAttribI4ui")
#undef glVertexAttribI4ui
void GL_APIENTRY glVertexAttribI4ui (GLuint index, GLuint x, GLuint y, GLuint z, GLuint w) {
#pragma pop_macro("glVertexAttribI4ui")
	wrapper->OnFunctionCall("glVertexAttribI4ui");
	GL_Impl::glVertexAttribI4ui(index, x, y, z, w);
	wrapper->CheckGLError("glVertexAttribI4ui");
}

#pragma push_macro("glVertexAttribI4iv")
#undef glVertexAttribI4iv
void GL_APIENTRY glVertexAttribI4iv (GLuint index, const GLint *v) {
#pragma pop_macro("glVertexAttribI4iv")
	wrapper->OnFunctionCall("glVertexAttribI4iv");
	GL_Impl::glVertexAttribI4iv(index, v);
	wrapper->CheckGLError("glVertexAttribI4iv");
}

#pragma push_macro("glVertexAttribI4uiv")
#undef glVertexAttribI4uiv
void GL_APIENTRY glVertexAttribI4uiv (GLuint index, const GLuint *v) {
#pragma pop_macro("glVertexAttribI4uiv")
	wrapper->OnFunctionCall("glVertexAttribI4uiv");
	GL_Impl::glVertexAttribI4uiv(index, v);
	wrapper->CheckGLError("glVertexAttribI4uiv");
}

#pragma push_macro("glGetUniformuiv")
#undef glGetUniformuiv
void GL_APIENTRY glGetUniformuiv (GLuint program, GLint location, GLuint *params) {
#pragma pop_macro("glGetUniformuiv")
	wrapper->OnFunctionCall("glGetUniformuiv");
	GL_Impl::glGetUniformuiv(program, location, params);
	wrapper->CheckGLError("glGetUniformuiv");
}

#pragma push_macro("glGetFragDataLocation")
#undef glGetFragDataLocation
GLint GL_APIENTRY glGetFragDataLocation (GLuint program, const GLchar *name) {
#pragma pop_macro("glGetFragDataLocation")
	wrapper->OnFunctionCall("glGetFragDataLocation");
	GLint retval = (GLint)GL_Impl::glGetFragDataLocation(program, name);
	wrapper->CheckGLError("glGetFragDataLocation");
	return retval;
}

#pragma push_macro("glUniform1ui")
#undef glUniform1ui
void GL_APIENTRY glUniform1ui (GLint location, GLuint v0) {
#pragma pop_macro("glUniform1ui")
	wrapper->OnFunctionCall("glUniform1ui");
	GL_Impl::glUniform1ui(location, v0);
	wrapper->CheckGLError("glUniform1ui");
}

#pragma push_macro("glUniform2ui")
#undef glUniform2ui
void GL_APIENTRY glUniform2ui (GLint location, GLuint v0, GLuint v1) {
#pragma pop_macro("glUniform2ui")
	wrapper->OnFunctionCall("glUniform2ui");
	GL_Impl::glUniform2ui(location, v0, v1);
	wrapper->CheckGLError("glUniform2ui");
}

#pragma push_macro("glUniform3ui")
#undef glUniform3ui
void GL_APIENTRY glUniform3ui (GLint location, GLuint v0, GLuint v1, GLuint v2) {
#pragma pop_macro("glUniform3ui")
	wrapper->OnFunctionCall("glUniform3ui");
	GL_Impl::glUniform3ui(location, v0, v1, v2);
	wrapper->CheckGLError("glUniform3ui");
}

#pragma push_macro("glUniform4ui")
#undef glUniform4ui
void GL_APIENTRY glUniform4ui (GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3) {
#pragma pop_macro("glUniform4ui")
	wrapper->OnFunctionCall("glUniform4ui");
	GL_Impl::glUniform4ui(location, v0, v1, v2, v3);
	wrapper->CheckGLError("glUniform4ui");
}

#pragma push_macro("glUniform1uiv")
#undef glUniform1uiv
void GL_APIENTRY glUniform1uiv (GLint location, GLsizei count, const GLuint *value) {
#pragma pop_macro("glUniform1uiv")
	wrapper->OnFunctionCall("glUniform1uiv");
	GL_Impl::glUniform1uiv(location, count, value);
	wrapper->CheckGLError("glUniform1uiv");
}

#pragma push_macro("glUniform2uiv")
#undef glUniform2uiv
void GL_APIENTRY glUniform2uiv (GLint location, GLsizei count, const GLuint *value) {
#pragma pop_macro("glUniform2uiv")
	wrapper->OnFunctionCall("glUniform2uiv");
	GL_Impl::glUniform2uiv(location, count, value);
	wrapper->CheckGLError("glUniform2uiv");
}

#pragma push_macro("glUniform3uiv")
#undef glUniform3uiv
void GL_APIENTRY glUniform3uiv (GLint location, GLsizei count, const GLuint *value) {
#pragma pop_macro("glUniform3uiv")
	wrapper->OnFunctionCall("glUniform3uiv");
	GL_Impl::glUniform3uiv(location, count, value);
	wrapper->CheckGLError("glUniform3uiv");
}

#pragma push_macro("glUniform4uiv")
#undef glUniform4uiv
void GL_APIENTRY glUniform4uiv (GLint location, GLsizei count, const GLuint *value) {
#pragma pop_macro("glUniform4uiv")
	wrapper->OnFunctionCall("glUniform4uiv");
	GL_Impl::glUniform4uiv(location, count, value);
	wrapper->CheckGLError("glUniform4uiv");
}

#pragma push_macro("glClearBufferiv")
#undef glClearBufferiv
void GL_APIENTRY glClearBufferiv (GLenum buffer, GLint drawbuffer, const GLint *value) {
#pragma pop_macro("glClearBufferiv")
	wrapper->OnFunctionCall("glClearBufferiv");
	GL_Impl::glClearBufferiv(buffer, drawbuffer, value);
	wrapper->CheckGLError("glClearBufferiv");
}

#pragma push_macro("glClearBufferuiv")
#undef glClearBufferuiv
void GL_APIENTRY glClearBufferuiv (GLenum buffer, GLint drawbuffer, const GLuint *value) {
#pragma pop_macro("glClearBufferuiv")
	wrapper->OnFunctionCall("glClearBufferuiv");
	GL_Impl::glClearBufferuiv(buffer, drawbuffer, value);
	wrapper->CheckGLError("glClearBufferuiv");
}

#pragma push_macro("glClearBufferfv")
#undef glClearBufferfv
void GL_APIENTRY glClearBufferfv (GLenum buffer, GLint drawbuffer, const GLfloat *value) {
#pragma pop_macro("glClearBufferfv")
	wrapper->OnFunctionCall("glClearBufferfv");
	GL_Impl::glClearBufferfv(buffer, drawbuffer, value);
	wrapper->CheckGLError("glClearBufferfv");
}

#pragma push_macro("glClearBufferfi")
#undef glClearBufferfi
void GL_APIENTRY glClearBufferfi (GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil) {
#pragma pop_macro("glClearBufferfi")
	wrapper->OnFunctionCall("glClearBufferfi");
	GL_Impl::glClearBufferfi(buffer, drawbuffer, depth, stencil);
	wrapper->CheckGLError("glClearBufferfi");
}

#pragma push_macro("glGetStringi")
#undef glGetStringi
const GLubyte *GL_APIENTRY glGetStringi (GLenum name, GLuint index) {
#pragma pop_macro("glGetStringi")
	wrapper->OnFunctionCall("glGetStringi");
	GLubyte* retval = (GLubyte*)GL_Impl::glGetStringi(name, index);
	wrapper->CheckGLError("glGetStringi");
	return retval;
}

#pragma push_macro("glCopyBufferSubData")
#undef glCopyBufferSubData
void GL_APIENTRY glCopyBufferSubData (GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size) {
#pragma pop_macro("glCopyBufferSubData")
	wrapper->OnFunctionCall("glCopyBufferSubData");
	GL_Impl::glCopyBufferSubData(readTarget, writeTarget, readOffset, writeOffset, size);
	wrapper->CheckGLError("glCopyBufferSubData");
}

#pragma push_macro("glGetUniformIndices")
#undef glGetUniformIndices
void GL_APIENTRY glGetUniformIndices (GLuint program, GLsizei uniformCount, const GLchar *const*uniformNames, GLuint *uniformIndices) {
#pragma pop_macro("glGetUniformIndices")
	wrapper->OnFunctionCall("glGetUniformIndices");
	GL_Impl::glGetUniformIndices(program, uniformCount, (const GL_Impl::GLchar **)uniformNames, uniformIndices);
	wrapper->CheckGLError("glGetUniformIndices");
}

#pragma push_macro("glGetActiveUniformsiv")
#undef glGetActiveUniformsiv
void GL_APIENTRY glGetActiveUniformsiv (GLuint program, GLsizei uniformCount, const GLuint *uniformIndices, GLenum pname, GLint *params) {
#pragma pop_macro("glGetActiveUniformsiv")
	wrapper->OnFunctionCall("glGetActiveUniformsiv");
	GL_Impl::glGetActiveUniformsiv(program, uniformCount, uniformIndices, pname, params);
	wrapper->CheckGLError("glGetActiveUniformsiv");
}

#pragma push_macro("glGetUniformBlockIndex")
#undef glGetUniformBlockIndex
GLuint GL_APIENTRY glGetUniformBlockIndex (GLuint program, const GLchar *uniformBlockName) {
#pragma pop_macro("glGetUniformBlockIndex")
	wrapper->OnFunctionCall("glGetUniformBlockIndex");
	GLuint retval = (GLuint)GL_Impl::glGetUniformBlockIndex(program, uniformBlockName);
	wrapper->CheckGLError("glGetUniformBlockIndex");
	return retval;
}

#pragma push_macro("glGetActiveUniformBlockiv")
#undef glGetActiveUniformBlockiv
void GL_APIENTRY glGetActiveUniformBlockiv (GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint *params) {
#pragma pop_macro("glGetActiveUniformBlockiv")
	wrapper->OnFunctionCall("glGetActiveUniformBlockiv");
	GL_Impl::glGetActiveUniformBlockiv(program, uniformBlockIndex, pname, params);
	wrapper->CheckGLError("glGetActiveUniformBlockiv");
}

#pragma push_macro("glGetActiveUniformBlockName")
#undef glGetActiveUniformBlockName
void GL_APIENTRY glGetActiveUniformBlockName (GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei *length, GLchar *uniformBlockName) {
#pragma pop_macro("glGetActiveUniformBlockName")
	wrapper->OnFunctionCall("glGetActiveUniformBlockName");
	GL_Impl::glGetActiveUniformBlockName(program, uniformBlockIndex, bufSize, length, uniformBlockName);
	wrapper->CheckGLError("glGetActiveUniformBlockName");
}

#pragma push_macro("glUniformBlockBinding")
#undef glUniformBlockBinding
void GL_APIENTRY glUniformBlockBinding (GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding) {
#pragma pop_macro("glUniformBlockBinding")
	wrapper->OnFunctionCall("glUniformBlockBinding");
	GL_Impl::glUniformBlockBinding(program, uniformBlockIndex, uniformBlockBinding);
	wrapper->CheckGLError("glUniformBlockBinding");
}

#pragma push_macro("glDrawArraysInstanced")
#undef glDrawArraysInstanced
void GL_APIENTRY glDrawArraysInstanced (GLenum mode, GLint first, GLsizei count, GLsizei instancecount) {
#pragma pop_macro("glDrawArraysInstanced")
	wrapper->OnFunctionCall("glDrawArraysInstanced");
	wrapper->OnPreDrawCall(mode, count, instancecount);
	GL_Impl::glDrawArraysInstanced(mode, first, count, instancecount);
	wrapper->CheckGLError("glDrawArraysInstanced");
	wrapper->OnPostDrawCall();
}

#pragma push_macro("glDrawElementsInstanced")
#undef glDrawElementsInstanced
void GL_APIENTRY glDrawElementsInstanced (GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount) {
#pragma pop_macro("glDrawElementsInstanced")
	wrapper->OnFunctionCall("glDrawElementsInstanced");
	wrapper->OnPreDrawCall(mode, count, instancecount);
	GL_Impl::glDrawElementsInstanced(mode, count, type, indices, instancecount);
	wrapper->CheckGLError("glDrawElementsInstanced");
	wrapper->OnPostDrawCall();
}

#pragma push_macro("glGetInteger64v")
#undef glGetInteger64v
void GL_APIENTRY glGetInteger64v (GLenum pname, GLint64 *data) {
#pragma pop_macro("glGetInteger64v")
	wrapper->OnFunctionCall("glGetInteger64v");
	GL_Impl::glGetInteger64v(pname, data);
	wrapper->CheckGLError("glGetInteger64v");
}

#pragma push_macro("glGetInteger64i_v")
#undef glGetInteger64i_v
void GL_APIENTRY glGetInteger64i_v (GLenum target, GLuint index, GLint64 *data) {
#pragma pop_macro("glGetInteger64i_v")
	wrapper->OnFunctionCall("glGetInteger64i_v");
	GL_Impl::glGetInteger64i_v(target, index, data);
	wrapper->CheckGLError("glGetInteger64i_v");
}

#pragma push_macro("glGetBufferParameteri64v")
#undef glGetBufferParameteri64v
void GL_APIENTRY glGetBufferParameteri64v (GLenum target, GLenum pname, GLint64 *params) {
#pragma pop_macro("glGetBufferParameteri64v")
	wrapper->OnFunctionCall("glGetBufferParameteri64v");
	GL_Impl::glGetBufferParameteri64v(target, pname, params);
	wrapper->CheckGLError("glGetBufferParameteri64v");
}

#pragma push_macro("glGenSamplers")
#undef glGenSamplers
void GL_APIENTRY glGenSamplers (GLsizei count, GLuint *samplers) {
#pragma pop_macro("glGenSamplers")
	wrapper->OnFunctionCall("glGenSamplers");
	GL_Impl::glGenSamplers(count, samplers);
	wrapper->CheckGLError("glGenSamplers");
	wrapper->OnGenObject(GLWrapperImpl::TYPE_SAMPLER, count, samplers);
}

#pragma push_macro("glDeleteSamplers")
#undef glDeleteSamplers
void GL_APIENTRY glDeleteSamplers (GLsizei count, const GLuint *samplers) {
#pragma pop_macro("glDeleteSamplers")
	wrapper->OnFunctionCall("glDeleteSamplers");
	GL_Impl::glDeleteSamplers(count, samplers);
	wrapper->CheckGLError("glDeleteSamplers");
	wrapper->OnDeleteObject(GLWrapperImpl::TYPE_SAMPLER, count, samplers);
}

#pragma push_macro("glIsSampler")
#undef glIsSampler
GLboolean GL_APIENTRY glIsSampler (GLuint sampler) {
#pragma pop_macro("glIsSampler")
	wrapper->OnFunctionCall("glIsSampler");
	GLboolean retval = (GLboolean)GL_Impl::glIsSampler(sampler);
	wrapper->CheckGLError("glIsSampler");
	return retval;
}

#pragma push_macro("glBindSampler")
#undef glBindSampler
void GL_APIENTRY glBindSampler (GLuint unit, GLuint sampler) {
#pragma pop_macro("glBindSampler")
	wrapper->OnFunctionCall("glBindSampler");
	wrapper->OnBindSampler(unit, sampler);
	GL_Impl::glBindSampler(unit, sampler);
	wrapper->CheckGLError("glBindSampler");
}

#pragma push_macro("glSamplerParameteri")
#undef glSamplerParameteri
void GL_APIENTRY glSamplerParameteri (GLuint sampler, GLenum pname, GLint param) {
#pragma pop_macro("glSamplerParameteri")
	wrapper->OnFunctionCall("glSamplerParameteri");
	GL_Impl::glSamplerParameteri(sampler, pname, param);
	wrapper->CheckGLError("glSamplerParameteri");
}

#pragma push_macro("glSamplerParameteriv")
#undef glSamplerParameteriv
void GL_APIENTRY glSamplerParameteriv (GLuint sampler, GLenum pname, const GLint *param) {
#pragma pop_macro("glSamplerParameteriv")
	wrapper->OnFunctionCall("glSamplerParameteriv");
	GL_Impl::glSamplerParameteriv(sampler, pname, param);
	wrapper->CheckGLError("glSamplerParameteriv");
}

#pragma push_macro("glSamplerParameterf")
#undef glSamplerParameterf
void GL_APIENTRY glSamplerParameterf (GLuint sampler, GLenum pname, GLfloat param) {
#pragma pop_macro("glSamplerParameterf")
	wrapper->OnFunctionCall("glSamplerParameterf");
	GL_Impl::glSamplerParameterf(sampler, pname, param);
	wrapper->CheckGLError("glSamplerParameterf");
}

#pragma push_macro("glSamplerParameterfv")
#undef glSamplerParameterfv
void GL_APIENTRY glSamplerParameterfv (GLuint sampler, GLenum pname, const GLfloat *param) {
#pragma pop_macro("glSamplerParameterfv")
	wrapper->OnFunctionCall("glSamplerParameterfv");
	GL_Impl::glSamplerParameterfv(sampler, pname, param);
	wrapper->CheckGLError("glSamplerParameterfv");
}

#pragma push_macro("glGetSamplerParameteriv")
#undef glGetSamplerParameteriv
void GL_APIENTRY glGetSamplerParameteriv (GLuint sampler, GLenum pname, GLint *params) {
#pragma pop_macro("glGetSamplerParameteriv")
	wrapper->OnFunctionCall("glGetSamplerParameteriv");
	GL_Impl::glGetSamplerParameteriv(sampler, pname, params);
	wrapper->CheckGLError("glGetSamplerParameteriv");
}

#pragma push_macro("glGetSamplerParameterfv")
#undef glGetSamplerParameterfv
void GL_APIENTRY glGetSamplerParameterfv (GLuint sampler, GLenum pname, GLfloat *params) {
#pragma pop_macro("glGetSamplerParameterfv")
	wrapper->OnFunctionCall("glGetSamplerParameterfv");
	GL_Impl::glGetSamplerParameterfv(sampler, pname, params);
	wrapper->CheckGLError("glGetSamplerParameterfv");
}

#pragma push_macro("glVertexAttribDivisor")
#undef glVertexAttribDivisor
void GL_APIENTRY glVertexAttribDivisor (GLuint index, GLuint divisor) {
#pragma pop_macro("glVertexAttribDivisor")
	wrapper->OnFunctionCall("glVertexAttribDivisor");
	GL_Impl::glVertexAttribDivisor(index, divisor);
	wrapper->CheckGLError("glVertexAttribDivisor");
}

#pragma push_macro("glBindTransformFeedback")
#undef glBindTransformFeedback
void GL_APIENTRY glBindTransformFeedback (GLenum target, GLuint id) {
#pragma pop_macro("glBindTransformFeedback")
	wrapper->OnFunctionCall("glBindTransformFeedback");
	GL_Impl::glBindTransformFeedback(target, id);
	wrapper->CheckGLError("glBindTransformFeedback");
}

#pragma push_macro("glDeleteTransformFeedbacks")
#undef glDeleteTransformFeedbacks
void GL_APIENTRY glDeleteTransformFeedbacks (GLsizei n, const GLuint *ids) {
#pragma pop_macro("glDeleteTransformFeedbacks")
	wrapper->OnFunctionCall("glDeleteTransformFeedbacks");
	GL_Impl::glDeleteTransformFeedbacks(n, ids);
	wrapper->CheckGLError("glDeleteTransformFeedbacks");
	wrapper->OnDeleteObject(GLWrapperImpl::TYPE_TRANSFORM_FEEDBACK, n, ids);
}

#pragma push_macro("glGenTransformFeedbacks")
#undef glGenTransformFeedbacks
void GL_APIENTRY glGenTransformFeedbacks (GLsizei n, GLuint *ids) {
#pragma pop_macro("glGenTransformFeedbacks")
	wrapper->OnFunctionCall("glGenTransformFeedbacks");
	GL_Impl::glGenTransformFeedbacks(n, ids);
	wrapper->CheckGLError("glGenTransformFeedbacks");
	wrapper->OnGenObject(GLWrapperImpl::TYPE_TRANSFORM_FEEDBACK, n, ids);
}

#pragma push_macro("glIsTransformFeedback")
#undef glIsTransformFeedback
GLboolean GL_APIENTRY glIsTransformFeedback (GLuint id) {
#pragma pop_macro("glIsTransformFeedback")
	wrapper->OnFunctionCall("glIsTransformFeedback");
	GLboolean retval = (GLboolean)GL_Impl::glIsTransformFeedback(id);
	wrapper->CheckGLError("glIsTransformFeedback");
	return retval;
}

#pragma push_macro("glPauseTransformFeedback")
#undef glPauseTransformFeedback
void GL_APIENTRY glPauseTransformFeedback (void) {
#pragma pop_macro("glPauseTransformFeedback")
	wrapper->OnFunctionCall("glPauseTransformFeedback");
	GL_Impl::glPauseTransformFeedback();
	wrapper->CheckGLError("glPauseTransformFeedback");
}

#pragma push_macro("glResumeTransformFeedback")
#undef glResumeTransformFeedback
void GL_APIENTRY glResumeTransformFeedback (void) {
#pragma pop_macro("glResumeTransformFeedback")
	wrapper->OnFunctionCall("glResumeTransformFeedback");
	GL_Impl::glResumeTransformFeedback();
	wrapper->CheckGLError("glResumeTransformFeedback");
}

#pragma push_macro("glGetProgramBinary")
#undef glGetProgramBinary
void GL_APIENTRY glGetProgramBinary (GLuint program, GLsizei bufSize, GLsizei *length, GLenum *binaryFormat, void *binary) {
#pragma pop_macro("glGetProgramBinary")
	wrapper->OnFunctionCall("glGetProgramBinary");
	GL_Impl::glGetProgramBinary(program, bufSize, length, binaryFormat, binary);
	wrapper->CheckGLError("glGetProgramBinary");
}

#pragma push_macro("glProgramBinary")
#undef glProgramBinary
void GL_APIENTRY glProgramBinary (GLuint program, GLenum binaryFormat, const void *binary, GLsizei length) {
#pragma pop_macro("glProgramBinary")
	wrapper->OnFunctionCall("glProgramBinary");
	GL_Impl::glProgramBinary(program, binaryFormat, binary, length);
	wrapper->CheckGLError("glProgramBinary");
}

#pragma push_macro("glProgramParameteri")
#undef glProgramParameteri
void GL_APIENTRY glProgramParameteri (GLuint program, GLenum pname, GLint value) {
#pragma pop_macro("glProgramParameteri")
	wrapper->OnFunctionCall("glProgramParameteri");
	GL_Impl::glProgramParameteri(program, pname, value);
	wrapper->CheckGLError("glProgramParameteri");
}

#pragma push_macro("glInvalidateFramebuffer")
#undef glInvalidateFramebuffer
void GL_APIENTRY glInvalidateFramebuffer (GLenum target, GLsizei numAttachments, const GLenum *attachments) {
#pragma pop_macro("glInvalidateFramebuffer")
	wrapper->OnFunctionCall("glInvalidateFramebuffer");
	GL_Impl::glInvalidateFramebuffer(target, numAttachments, attachments);
	wrapper->CheckGLError("glInvalidateFramebuffer");
}

#pragma push_macro("glInvalidateSubFramebuffer")
#undef glInvalidateSubFramebuffer
void GL_APIENTRY glInvalidateSubFramebuffer (GLenum target, GLsizei numAttachments, const GLenum *attachments, GLint x, GLint y, GLsizei width, GLsizei height) {
#pragma pop_macro("glInvalidateSubFramebuffer")
	wrapper->OnFunctionCall("glInvalidateSubFramebuffer");
	GL_Impl::glInvalidateSubFramebuffer(target, numAttachments, attachments, x, y, width, height);
	wrapper->CheckGLError("glInvalidateSubFramebuffer");
}

#pragma push_macro("glTexStorage2D")
#undef glTexStorage2D
void GL_APIENTRY glTexStorage2D (GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height) {
#pragma pop_macro("glTexStorage2D")
	wrapper->OnFunctionCall("glTexStorage2D");
	GL_Impl::glTexStorage2D(target, levels, internalformat, width, height);
	wrapper->CheckGLError("glTexStorage2D");
}

#pragma push_macro("glTexStorage3D")
#undef glTexStorage3D
void GL_APIENTRY glTexStorage3D (GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth) {
#pragma pop_macro("glTexStorage3D")
	wrapper->OnFunctionCall("glTexStorage3D");
	GL_Impl::glTexStorage3D(target, levels, internalformat, width, height, depth);
	wrapper->CheckGLError("glTexStorage3D");
}

#pragma push_macro("glGetInternalformativ")
#undef glGetInternalformativ
void GL_APIENTRY glGetInternalformativ (GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLint *params) {
#pragma pop_macro("glGetInternalformativ")
	wrapper->OnFunctionCall("glGetInternalformativ");
	GL_Impl::glGetInternalformativ(target, internalformat, pname, bufSize, params);
	wrapper->CheckGLError("glGetInternalformativ");
}

#pragma push_macro("glDispatchCompute")
#undef glDispatchCompute
void GL_APIENTRY glDispatchCompute (GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z) {
#pragma pop_macro("glDispatchCompute")
	wrapper->OnFunctionCall("glDispatchCompute");
	wrapper->OnDispatchCompute(num_groups_x, num_groups_y, num_groups_z);
	GL_Impl::glDispatchCompute(num_groups_x, num_groups_y, num_groups_z);
	wrapper->CheckGLError("glDispatchCompute");
}

#pragma push_macro("glDispatchComputeIndirect")
#undef glDispatchComputeIndirect
void GL_APIENTRY glDispatchComputeIndirect (GLintptr indirect) {
#pragma pop_macro("glDispatchComputeIndirect")
	wrapper->OnFunctionCall("glDispatchComputeIndirect");
	wrapper->OnDispatchCompute(0, 0, 0);
	GL_Impl::glDispatchComputeIndirect(indirect);
	wrapper->CheckGLError("glDispatchComputeIndirect");
}

#pragma push_macro("glDrawArraysIndirect")
#undef glDrawArraysIndirect
void GL_APIENTRY glDrawArraysIndirect (GLenum mode, const void *indirect) {
#pragma pop_macro("glDrawArraysIndirect")
	wrapper->OnFunctionCall("glDrawArraysIndirect");
	wrapper->OnPreDrawCall(mode, 0, 0);
	GL_Impl::glDrawArraysIndirect(mode, indirect);
	wrapper->CheckGLError("glDrawArraysIndirect");
	wrapper->OnPostDrawCall();
}

#pragma push_macro("glDrawElementsIndirect")
#undef glDrawElementsIndirect
void GL_APIENTRY glDrawElementsIndirect (GLenum mode, GLenum type, const void *indirect) {
#pragma pop_macro("glDrawElementsIndirect")
	wrapper->OnFunctionCall("glDrawElementsIndirect");
	wrapper->OnPreDrawCall(mode, 0, 0);
	GL_Impl::glDrawElementsIndirect(mode, type, indirect);
	wrapper->CheckGLError("glDrawElementsIndirect");
	wrapper->OnPostDrawCall();
}

#pragma push_macro("glFramebufferParameteri")
#undef glFramebufferParameteri
void GL_APIENTRY glFramebufferParameteri (GLenum target, GLenum pname, GLint param) {
#pragma pop_macro("glFramebufferParameteri")
	wrapper->OnFunctionCall("glFramebufferParameteri");
	GL_Impl::glFramebufferParameteri(target, pname, param);
	wrapper->CheckGLError("glFramebufferParameteri");
}

#pragma push_macro("glGetFramebufferParameteriv")
#undef glGetFramebufferParameteriv
void GL_APIENTRY glGetFramebufferParameteriv (GLenum target, GLenum pname, GLint *params) {
#pragma pop_macro("glGetFramebufferParameteriv")
	wrapper->OnFunctionCall("glGetFramebufferParameteriv");
	GL_Impl::glGetFramebufferParameteriv(target, pname, params);
	wrapper->CheckGLError("glGetFramebufferParameteriv");
}

#pragma push_macro("glGetProgramInterfaceiv")
#undef glGetProgramInterfaceiv
void GL_APIENTRY glGetProgramInterfaceiv (GLuint program, GLenum programInterface, GLenum pname, GLint *params) {
#pragma pop_macro("glGetProgramInterfaceiv")
	wrapper->OnFunctionCall("glGetProgramInterfaceiv");
	GL_Impl::glGetProgramInterfaceiv(program, programInterface, pname, params);
	wrapper->CheckGLError("glGetProgramInterfaceiv");
}

#pragma push_macro("glGetProgramResourceIndex")
#undef glGetProgramResourceIndex
GLuint GL_APIENTRY glGetProgramResourceIndex (GLuint program, GLenum programInterface, const GLchar *name) {
#pragma pop_macro("glGetProgramResourceIndex")
	wrapper->OnFunctionCall("glGetProgramResourceIndex");
	GLuint retval = (GLuint)GL_Impl::glGetProgramResourceIndex(program, programInterface, name);
	wrapper->CheckGLError("glGetProgramResourceIndex");
	return retval;
}

#pragma push_macro("glGetProgramResourceName")
#undef glGetProgramResourceName
void GL_APIENTRY glGetProgramResourceName (GLuint program, GLenum programInterface, GLuint index, GLsizei bufSize, GLsizei *length, GLchar *name) {
#pragma pop_macro("glGetProgramResourceName")
	wrapper->OnFunctionCall("glGetProgramResourceName");
	GL_Impl::glGetProgramResourceName(program, programInterface, index, bufSize, length, name);
	wrapper->CheckGLError("glGetProgramResourceName");
}

#pragma push_macro("glGetProgramResourceiv")
#undef glGetProgramResourceiv
void GL_APIENTRY glGetProgramResourceiv (GLuint program, GLenum programInterface, GLuint index, GLsizei propCount, const GLenum *props, GLsizei bufSize, GLsizei *length, GLint *params) {
#pragma pop_macro("glGetProgramResourceiv")
	wrapper->OnFunctionCall("glGetProgramResourceiv");
	GL_Impl::glGetProgramResourceiv(program, programInterface, index, propCount, props, bufSize, length, params);
	wrapper->CheckGLError("glGetProgramResourceiv");
}

#pragma push_macro("glGetProgramResourceLocation")
#undef glGetProgramResourceLocation
GLint GL_APIENTRY glGetProgramResourceLocation (GLuint program, GLenum programInterface, const GLchar *name) {
#pragma pop_macro("glGetProgramResourceLocation")
	wrapper->OnFunctionCall("glGetProgramResourceLocation");
	GLint retval = (GLint)GL_Impl::glGetProgramResourceLocation(program, programInterface, name);
	wrapper->CheckGLError("glGetProgramResourceLocation");
	return retval;
}

#pragma push_macro("glUseProgramStages")
#undef glUseProgramStages
void GL_APIENTRY glUseProgramStages (GLuint pipeline, GLbitfield stages, GLuint program) {
#pragma pop_macro("glUseProgramStages")
	wrapper->OnFunctionCall("glUseProgramStages");
	GL_Impl::glUseProgramStages(pipeline, stages, program);
	wrapper->CheckGLError("glUseProgramStages");
}

#pragma push_macro("glActiveShaderProgram")
#undef glActiveShaderProgram
void GL_APIENTRY glActiveShaderProgram (GLuint pipeline, GLuint program) {
#pragma pop_macro("glActiveShaderProgram")
	wrapper->OnFunctionCall("glActiveShaderProgram");
	GL_Impl::glActiveShaderProgram(pipeline, program);
	wrapper->CheckGLError("glActiveShaderProgram");
}

#pragma push_macro("glCreateShaderProgramv")
#undef glCreateShaderProgramv
GLuint GL_APIENTRY glCreateShaderProgramv (GLenum type, GLsizei count, const GLchar *const*strings) {
#pragma pop_macro("glCreateShaderProgramv")
	wrapper->OnFunctionCall("glCreateShaderProgramv");
	GLuint retval = (GLuint)GL_Impl::glCreateShaderProgramv(type, count, (const GL_Impl::GLchar **)strings);
	wrapper->CheckGLError("glCreateShaderProgramv");
	return retval;
}

#pragma push_macro("glBindProgramPipeline")
#undef glBindProgramPipeline
void GL_APIENTRY glBindProgramPipeline (GLuint pipeline) {
#pragma pop_macro("glBindProgramPipeline")
	wrapper->OnFunctionCall("glBindProgramPipeline");
	GL_Impl::glBindProgramPipeline(pipeline);
	wrapper->CheckGLError("glBindProgramPipeline");
}

#pragma push_macro("glDeleteProgramPipelines")
#undef glDeleteProgramPipelines
void GL_APIENTRY glDeleteProgramPipelines (GLsizei n, const GLuint *pipelines) {
#pragma pop_macro("glDeleteProgramPipelines")
	wrapper->OnFunctionCall("glDeleteProgramPipelines");
	GL_Impl::glDeleteProgramPipelines(n, pipelines);
	wrapper->CheckGLError("glDeleteProgramPipelines");
	wrapper->OnDeleteObject(GLWrapperImpl::TYPE_PROGRAM_PIPELINE, n, pipelines);
}

#pragma push_macro("glGenProgramPipelines")
#undef glGenProgramPipelines
void GL_APIENTRY glGenProgramPipelines (GLsizei n, GLuint *pipelines) {
#pragma pop_macro("glGenProgramPipelines")
	wrapper->OnFunctionCall("glGenProgramPipelines");
	GL_Impl::glGenProgramPipelines(n, pipelines);
	wrapper->CheckGLError("glGenProgramPipelines");
	wrapper->OnGenObject(GLWrapperImpl::TYPE_PROGRAM_PIPELINE, n, pipelines);
}

#pragma push_macro("glIsProgramPipeline")
#undef glIsProgramPipeline
GLboolean GL_APIENTRY glIsProgramPipeline (GLuint pipeline) {
#pragma pop_macro("glIsProgramPipeline")
	wrapper->OnFunctionCall("glIsProgramPipeline");
	GLboolean retval = (GLboolean)GL_Impl::glIsProgramPipeline(pipeline);
	wrapper->CheckGLError("glIsProgramPipeline");
	return retval;
}

#pragma push_macro("glGetProgramPipelineiv")
#undef glGetProgramPipelineiv
void GL_APIENTRY glGetProgramPipelineiv (GLuint pipeline, GLenum pname, GLint *params) {
#pragma pop_macro("glGetProgramPipelineiv")
	wrapper->OnFunctionCall("glGetProgramPipelineiv");
	GL_Impl::glGetProgramPipelineiv(pipeline, pname, params);
	wrapper->CheckGLError("glGetProgramPipelineiv");
}

#pragma push_macro("glProgramUniform1i")
#undef glProgramUniform1i
void GL_APIENTRY glProgramUniform1i (GLuint program, GLint location, GLint v0) {
#pragma pop_macro("glProgramUniform1i")
	wrapper->OnFunctionCall("glProgramUniform1i");
	GL_Impl::glProgramUniform1i(program, location, v0);
	wrapper->CheckGLError("glProgramUniform1i");
}

#pragma push_macro("glProgramUniform2i")
#undef glProgramUniform2i
void GL_APIENTRY glProgramUniform2i (GLuint program, GLint location, GLint v0, GLint v1) {
#pragma pop_macro("glProgramUniform2i")
	wrapper->OnFunctionCall("glProgramUniform2i");
	GL_Impl::glProgramUniform2i(program, location, v0, v1);
	wrapper->CheckGLError("glProgramUniform2i");
}

#pragma push_macro("glProgramUniform3i")
#undef glProgramUniform3i
void GL_APIENTRY glProgramUniform3i (GLuint program, GLint location, GLint v0, GLint v1, GLint v2) {
#pragma pop_macro("glProgramUniform3i")
	wrapper->OnFunctionCall("glProgramUniform3i");
	GL_Impl::glProgramUniform3i(program, location, v0, v1, v2);
	wrapper->CheckGLError("glProgramUniform3i");
}

#pragma push_macro("glProgramUniform4i")
#undef glProgramUniform4i
void GL_APIENTRY glProgramUniform4i (GLuint program, GLint location, GLint v0, GLint v1, GLint v2, GLint v3) {
#pragma pop_macro("glProgramUniform4i")
	wrapper->OnFunctionCall("glProgramUniform4i");
	GL_Impl::glProgramUniform4i(program, location, v0, v1, v2, v3);
	wrapper->CheckGLError("glProgramUniform4i");
}

#pragma push_macro("glProgramUniform1ui")
#undef glProgramUniform1ui
void GL_APIENTRY glProgramUniform1ui (GLuint program, GLint location, GLuint v0) {
#pragma pop_macro("glProgramUniform1ui")
	wrapper->OnFunctionCall("glProgramUniform1ui");
	GL_Impl::glProgramUniform1ui(program, location, v0);
	wrapper->CheckGLError("glProgramUniform1ui");
}

#pragma push_macro("glProgramUniform2ui")
#undef glProgramUniform2ui
void GL_APIENTRY glProgramUniform2ui (GLuint program, GLint location, GLuint v0, GLuint v1) {
#pragma pop_macro("glProgramUniform2ui")
	wrapper->OnFunctionCall("glProgramUniform2ui");
	GL_Impl::glProgramUniform2ui(program, location, v0, v1);
	wrapper->CheckGLError("glProgramUniform2ui");
}

#pragma push_macro("glProgramUniform3ui")
#undef glProgramUniform3ui
void GL_APIENTRY glProgramUniform3ui (GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2) {
#pragma pop_macro("glProgramUniform3ui")
	wrapper->OnFunctionCall("glProgramUniform3ui");
	GL_Impl::glProgramUniform3ui(program, location, v0, v1, v2);
	wrapper->CheckGLError("glProgramUniform3ui");
}

#pragma push_macro("glProgramUniform4ui")
#undef glProgramUniform4ui
void GL_APIENTRY glProgramUniform4ui (GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3) {
#pragma pop_macro("glProgramUniform4ui")
	wrapper->OnFunctionCall("glProgramUniform4ui");
	GL_Impl::glProgramUniform4ui(program, location, v0, v1, v2, v3);
	wrapper->CheckGLError("glProgramUniform4ui");
}

#pragma push_macro("glProgramUniform1f")
#undef glProgramUniform1f
void GL_APIENTRY glProgramUniform1f (GLuint program, GLint location, GLfloat v0) {
#pragma pop_macro("glProgramUniform1f")
	wrapper->OnFunctionCall("glProgramUniform1f");
	GL_Impl::glProgramUniform1f(program, location, v0);
	wrapper->CheckGLError("glProgramUniform1f");
}

#pragma push_macro("glProgramUniform2f")
#undef glProgramUniform2f
void GL_APIENTRY glProgramUniform2f (GLuint program, GLint location, GLfloat v0, GLfloat v1) {
#pragma pop_macro("glProgramUniform2f")
	wrapper->OnFunctionCall("glProgramUniform2f");
	GL_Impl::glProgramUniform2f(program, location, v0, v1);
	wrapper->CheckGLError("glProgramUniform2f");
}

#pragma push_macro("glProgramUniform3f")
#undef glProgramUniform3f
void GL_APIENTRY glProgramUniform3f (GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2) {
#pragma pop_macro("glProgramUniform3f")
	wrapper->OnFunctionCall("glProgramUniform3f");
	GL_Impl::glProgramUniform3f(program, location, v0, v1, v2);
	wrapper->CheckGLError("glProgramUniform3f");
}

#pragma push_macro("glProgramUniform4f")
#undef glProgramUniform4f
void GL_APIENTRY glProgramUniform4f (GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3) {
#pragma pop_macro("glProgramUniform4f")
	wrapper->OnFunctionCall("glProgramUniform4f");
	GL_Impl::glProgramUniform4f(program, location, v0, v1, v2, v3);
	wrapper->CheckGLError("glProgramUniform4f");
}

#pragma push_macro("glProgramUniform1iv")
#undef glProgramUniform1iv
void GL_APIENTRY glProgramUniform1iv (GLuint program, GLint location, GLsizei count, const GLint *value) {
#pragma pop_macro("glProgramUniform1iv")
	wrapper->OnFunctionCall("glProgramUniform1iv");
	GL_Impl::glProgramUniform1iv(program, location, count, value);
	wrapper->CheckGLError("glProgramUniform1iv");
}

#pragma push_macro("glProgramUniform2iv")
#undef glProgramUniform2iv
void GL_APIENTRY glProgramUniform2iv (GLuint program, GLint location, GLsizei count, const GLint *value) {
#pragma pop_macro("glProgramUniform2iv")
	wrapper->OnFunctionCall("glProgramUniform2iv");
	GL_Impl::glProgramUniform2iv(program, location, count, value);
	wrapper->CheckGLError("glProgramUniform2iv");
}

#pragma push_macro("glProgramUniform3iv")
#undef glProgramUniform3iv
void GL_APIENTRY glProgramUniform3iv (GLuint program, GLint location, GLsizei count, const GLint *value) {
#pragma pop_macro("glProgramUniform3iv")
	wrapper->OnFunctionCall("glProgramUniform3iv");
	GL_Impl::glProgramUniform3iv(program, location, count, value);
	wrapper->CheckGLError("glProgramUniform3iv");
}

#pragma push_macro("glProgramUniform4iv")
#undef glProgramUniform4iv
void GL_APIENTRY glProgramUniform4iv (GLuint program, GLint location, GLsizei count, const GLint *value) {
#pragma pop_macro("glProgramUniform4iv")
	wrapper->OnFunctionCall("glProgramUniform4iv");
	GL_Impl::glProgramUniform4iv(program, location, count, value);
	wrapper->CheckGLError("glProgramUniform4iv");
}

#pragma push_macro("glProgramUniform1uiv")
#undef glProgramUniform1uiv
void GL_APIENTRY glProgramUniform1uiv (GLuint program, GLint location, GLsizei count, const GLuint *value) {
#pragma pop_macro("glProgramUniform1uiv")
	wrapper->OnFunctionCall("glProgramUniform1uiv");
	GL_Impl::glProgramUniform1uiv(program, location, count, value);
	wrapper->CheckGLError("glProgramUniform1uiv");
}

#pragma push_macro("glProgramUniform2uiv")
#undef glProgramUniform2uiv
void GL_APIENTRY glProgramUniform2uiv (GLuint program, GLint location, GLsizei count, const GLuint *value) {
#pragma pop_macro("glProgramUniform2uiv")
	wrapper->OnFunctionCall("glProgramUniform2uiv");
	GL_Impl::glProgramUniform2uiv(program, location, count, value);
	wrapper->CheckGLError("glProgramUniform2uiv");
}

#pragma push_macro("glProgramUniform3uiv")
#undef glProgramUniform3uiv
void GL_APIENTRY glProgramUniform3uiv (GLuint program, GLint location, GLsizei count, const GLuint *value) {
#pragma pop_macro("glProgramUniform3uiv")
	wrapper->OnFunctionCall("glProgramUniform3uiv");
	GL_Impl::glProgramUniform3uiv(program, location, count, value);
	wrapper->CheckGLError("glProgramUniform3uiv");
}

#pragma push_macro("glProgramUniform4uiv")
#undef glProgramUniform4uiv
void GL_APIENTRY glProgramUniform4uiv (GLuint program, GLint location, GLsizei count, const GLuint *value) {
#pragma pop_macro("glProgramUniform4uiv")
	wrapper->OnFunctionCall("glProgramUniform4uiv");
	GL_Impl::glProgramUniform4uiv(program, location, count, value);
	wrapper->CheckGLError("glProgramUniform4uiv");
}

#pragma push_macro("glProgramUniform1fv")
#undef glProgramUniform1fv
void GL_APIENTRY glProgramUniform1fv (GLuint program, GLint location, GLsizei count, const GLfloat *value) {
#pragma pop_macro("glProgramUniform1fv")
	wrapper->OnFunctionCall("glProgramUniform1fv");
	GL_Impl::glProgramUniform1fv(program, location, count, value);
	wrapper->CheckGLError("glProgramUniform1fv");
}

#pragma push_macro("glProgramUniform2fv")
#undef glProgramUniform2fv
void GL_APIENTRY glProgramUniform2fv (GLuint program, GLint location, GLsizei count, const GLfloat *value) {
#pragma pop_macro("glProgramUniform2fv")
	wrapper->OnFunctionCall("glProgramUniform2fv");
	GL_Impl::glProgramUniform2fv(program, location, count, value);
	wrapper->CheckGLError("glProgramUniform2fv");
}

#pragma push_macro("glProgramUniform3fv")
#undef glProgramUniform3fv
void GL_APIENTRY glProgramUniform3fv (GLuint program, GLint location, GLsizei count, const GLfloat *value) {
#pragma pop_macro("glProgramUniform3fv")
	wrapper->OnFunctionCall("glProgramUniform3fv");
	GL_Impl::glProgramUniform3fv(program, location, count, value);
	wrapper->CheckGLError("glProgramUniform3fv");
}

#pragma push_macro("glProgramUniform4fv")
#undef glProgramUniform4fv
void GL_APIENTRY glProgramUniform4fv (GLuint program, GLint location, GLsizei count, const GLfloat *value) {
#pragma pop_macro("glProgramUniform4fv")
	wrapper->OnFunctionCall("glProgramUniform4fv");
	GL_Impl::glProgramUniform4fv(program, location, count, value);
	wrapper->CheckGLError("glProgramUniform4fv");
}

#pragma push_macro("glProgramUniformMatrix2fv")
#undef glProgramUniformMatrix2fv
void GL_APIENTRY glProgramUniformMatrix2fv (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) {
#pragma pop_macro("glProgramUniformMatrix2fv")
	wrapper->OnFunctionCall("glProgramUniformMatrix2fv");
	GL_Impl::glProgramUniformMatrix2fv(program, location, count, transpose, value);
	wrapper->CheckGLError("glProgramUniformMatrix2fv");
}

#pragma push_macro("glProgramUniformMatrix3fv")
#undef glProgramUniformMatrix3fv
void GL_APIENTRY glProgramUniformMatrix3fv (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) {
#pragma pop_macro("glProgramUniformMatrix3fv")
	wrapper->OnFunctionCall("glProgramUniformMatrix3fv");
	GL_Impl::glProgramUniformMatrix3fv(program, location, count, transpose, value);
	wrapper->CheckGLError("glProgramUniformMatrix3fv");
}

#pragma push_macro("glProgramUniformMatrix4fv")
#undef glProgramUniformMatrix4fv
void GL_APIENTRY glProgramUniformMatrix4fv (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) {
#pragma pop_macro("glProgramUniformMatrix4fv")
	wrapper->OnFunctionCall("glProgramUniformMatrix4fv");
	GL_Impl::glProgramUniformMatrix4fv(program, location, count, transpose, value);
	wrapper->CheckGLError("glProgramUniformMatrix4fv");
}

#pragma push_macro("glProgramUniformMatrix2x3fv")
#undef glProgramUniformMatrix2x3fv
void GL_APIENTRY glProgramUniformMatrix2x3fv (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) {
#pragma pop_macro("glProgramUniformMatrix2x3fv")
	wrapper->OnFunctionCall("glProgramUniformMatrix2x3fv");
	GL_Impl::glProgramUniformMatrix2x3fv(program, location, count, transpose, value);
	wrapper->CheckGLError("glProgramUniformMatrix2x3fv");
}

#pragma push_macro("glProgramUniformMatrix3x2fv")
#undef glProgramUniformMatrix3x2fv
void GL_APIENTRY glProgramUniformMatrix3x2fv (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) {
#pragma pop_macro("glProgramUniformMatrix3x2fv")
	wrapper->OnFunctionCall("glProgramUniformMatrix3x2fv");
	GL_Impl::glProgramUniformMatrix3x2fv(program, location, count, transpose, value);
	wrapper->CheckGLError("glProgramUniformMatrix3x2fv");
}

#pragma push_macro("glProgramUniformMatrix2x4fv")
#undef glProgramUniformMatrix2x4fv
void GL_APIENTRY glProgramUniformMatrix2x4fv (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) {
#pragma pop_macro("glProgramUniformMatrix2x4fv")
	wrapper->OnFunctionCall("glProgramUniformMatrix2x4fv");
	GL_Impl::glProgramUniformMatrix2x4fv(program, location, count, transpose, value);
	wrapper->CheckGLError("glProgramUniformMatrix2x4fv");
}

#pragma push_macro("glProgramUniformMatrix4x2fv")
#undef glProgramUniformMatrix4x2fv
void GL_APIENTRY glProgramUniformMatrix4x2fv (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) {
#pragma pop_macro("glProgramUniformMatrix4x2fv")
	wrapper->OnFunctionCall("glProgramUniformMatrix4x2fv");
	GL_Impl::glProgramUniformMatrix4x2fv(program, location, count, transpose, value);
	wrapper->CheckGLError("glProgramUniformMatrix4x2fv");
}

#pragma push_macro("glProgramUniformMatrix3x4fv")
#undef glProgramUniformMatrix3x4fv
void GL_APIENTRY glProgramUniformMatrix3x4fv (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) {
#pragma pop_macro("glProgramUniformMatrix3x4fv")
	wrapper->OnFunctionCall("glProgramUniformMatrix3x4fv");
	GL_Impl::glProgramUniformMatrix3x4fv(program, location, count, transpose, value);
	wrapper->CheckGLError("glProgramUniformMatrix3x4fv");
}

#pragma push_macro("glProgramUniformMatrix4x3fv")
#undef glProgramUniformMatrix4x3fv
void GL_APIENTRY glProgramUniformMatrix4x3fv (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) {
#pragma pop_macro("glProgramUniformMatrix4x3fv")
	wrapper->OnFunctionCall("glProgramUniformMatrix4x3fv");
	GL_Impl::glProgramUniformMatrix4x3fv(program, location, count, transpose, value);
	wrapper->CheckGLError("glProgramUniformMatrix4x3fv");
}

#pragma push_macro("glValidateProgramPipeline")
#undef glValidateProgramPipeline
void GL_APIENTRY glValidateProgramPipeline (GLuint pipeline) {
#pragma pop_macro("glValidateProgramPipeline")
	wrapper->OnFunctionCall("glValidateProgramPipeline");
	GL_Impl::glValidateProgramPipeline(pipeline);
	wrapper->CheckGLError("glValidateProgramPipeline");
}

#pragma push_macro("glGetProgramPipelineInfoLog")
#undef glGetProgramPipelineInfoLog
void GL_APIENTRY glGetProgramPipelineInfoLog (GLuint pipeline, GLsizei bufSize, GLsizei *length, GLchar *infoLog) {
#pragma pop_macro("glGetProgramPipelineInfoLog")
	wrapper->OnFunctionCall("glGetProgramPipelineInfoLog");
	GL_Impl::glGetProgramPipelineInfoLog(pipeline, bufSize, length, infoLog);
	wrapper->CheckGLError("glGetProgramPipelineInfoLog");
}

#pragma push_macro("glBindImageTexture")
#undef glBindImageTexture
void GL_APIENTRY glBindImageTexture (GLuint unit, GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum access, GLenum format) {
#pragma pop_macro("glBindImageTexture")
	wrapper->OnFunctionCall("glBindImageTexture");
	wrapper->OnBindImageTexture(unit, texture, level, layered, layer, access, format);
	GL_Impl::glBindImageTexture(unit, texture, level, layered, layer, access, format);
	wrapper->CheckGLError("glBindImageTexture");
}

#pragma push_macro("glGetBooleani_v")
#undef glGetBooleani_v
void GL_APIENTRY glGetBooleani_v (GLenum target, GLuint index, GLboolean *data) {
#pragma pop_macro("glGetBooleani_v")
	wrapper->OnFunctionCall("glGetBooleani_v");
	GL_Impl::glGetBooleani_v(target, index, data);
	wrapper->CheckGLError("glGetBooleani_v");
}

#pragma push_macro("glMemoryBarrier")
#undef glMemoryBarrier
void GL_APIENTRY glMemoryBarrier (GLbitfield barriers) {
#pragma pop_macro("glMemoryBarrier")
	wrapper->OnFunctionCall("glMemoryBarrier");
	GL_Impl::glMemoryBarrier(barriers);
	wrapper->CheckGLError("glMemoryBarrier");
}

#pragma push_macro("glTexStorage2DMultisample")
#undef glTexStorage2DMultisample
void GL_APIENTRY glTexStorage2DMultisample (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations) {
#pragma pop_macro("glTexStorage2DMultisample")
	wrapper->OnFunctionCall("glTexStorage2DMultisample");
	GL_Impl::glTexStorage2DMultisample(target, samples, internalformat, width, height, fixedsamplelocations);
	wrapper->CheckGLError("glTexStorage2DMultisample");
}

#pragma push_macro("glGetMultisamplefv")
#undef glGetMultisamplefv
void GL_APIENTRY glGetMultisamplefv (GLenum pname, GLuint index, GLfloat *val) {
#pragma pop_macro("glGetMultisamplefv")
	wrapper->OnFunctionCall("glGetMultisamplefv");
	GL_Impl::glGetMultisamplefv(pname, index, val);
	wrapper->CheckGLError("glGetMultisamplefv");
}

#pragma push_macro("glSampleMaski")
#undef glSampleMaski
void GL_APIENTRY glSampleMaski (GLuint maskNumber, GLbitfield mask) {
#pragma pop_macro("glSampleMaski")
	wrapper->OnFunctionCall("glSampleMaski");
	GL_Impl::glSampleMaski(maskNumber, mask);
	wrapper->CheckGLError("glSampleMaski");
}

#pragma push_macro("glGetTexLevelParameteriv")
#undef glGetTexLevelParameteriv
void GL_APIENTRY glGetTexLevelParameteriv (GLenum target, GLint level, GLenum pname, GLint *params) {
#pragma pop_macro("glGetTexLevelParameteriv")
	wrapper->OnFunctionCall("glGetTexLevelParameteriv");
	GL_Impl::glGetTexLevelParameteriv(target, level, pname, params);
	wrapper->CheckGLError("glGetTexLevelParameteriv");
}

#pragma push_macro("glGetTexLevelParameterfv")
#undef glGetTexLevelParameterfv
void GL_APIENTRY glGetTexLevelParameterfv (GLenum target, GLint level, GLenum pname, GLfloat *params) {
#pragma pop_macro("glGetTexLevelParameterfv")
	wrapper->OnFunctionCall("glGetTexLevelParameterfv");
	GL_Impl::glGetTexLevelParameterfv(target, level, pname, params);
	wrapper->CheckGLError("glGetTexLevelParameterfv");
}

#pragma push_macro("glBindVertexBuffer")
#undef glBindVertexBuffer
void GL_APIENTRY glBindVertexBuffer (GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride) {
#pragma pop_macro("glBindVertexBuffer")
	wrapper->OnFunctionCall("glBindVertexBuffer");
	GL_Impl::glBindVertexBuffer(bindingindex, buffer, offset, stride);
	wrapper->CheckGLError("glBindVertexBuffer");
}

#pragma push_macro("glVertexAttribFormat")
#undef glVertexAttribFormat
void GL_APIENTRY glVertexAttribFormat (GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset) {
#pragma pop_macro("glVertexAttribFormat")
	wrapper->OnFunctionCall("glVertexAttribFormat");
	GL_Impl::glVertexAttribFormat(attribindex, size, type, normalized, relativeoffset);
	wrapper->CheckGLError("glVertexAttribFormat");
}

#pragma push_macro("glVertexAttribIFormat")
#undef glVertexAttribIFormat
void GL_APIENTRY glVertexAttribIFormat (GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset) {
#pragma pop_macro("glVertexAttribIFormat")
	wrapper->OnFunctionCall("glVertexAttribIFormat");
	GL_Impl::glVertexAttribIFormat(attribindex, size, type, relativeoffset);
	wrapper->CheckGLError("glVertexAttribIFormat");
}

#pragma push_macro("glVertexAttribBinding")
#undef glVertexAttribBinding
void GL_APIENTRY glVertexAttribBinding (GLuint attribindex, GLuint bindingindex) {
#pragma pop_macro("glVertexAttribBinding")
	wrapper->OnFunctionCall("glVertexAttribBinding");
	GL_Impl::glVertexAttribBinding(attribindex, bindingindex);
	wrapper->CheckGLError("glVertexAttribBinding");
}

#pragma push_macro("glVertexBindingDivisor")
#undef glVertexBindingDivisor
void GL_APIENTRY glVertexBindingDivisor (GLuint bindingindex, GLuint divisor) {
#pragma pop_macro("glVertexBindingDivisor")
	wrapper->OnFunctionCall("glVertexBindingDivisor");
	GL_Impl::glVertexBindingDivisor(bindingindex, divisor);
	wrapper->CheckGLError("glVertexBindingDivisor");
}

#endif
