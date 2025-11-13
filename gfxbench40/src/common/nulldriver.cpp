/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
/** \file nulldriver.cpp
Function definitions of our dummy OpenGL implementation. \see nulldriver.h
*/
#ifdef OPENGL_IMPLEMENTATION_NULL
#ifdef PLATFORM_WINDOWS
#include <windows.h>
#endif

#include <stdlib.h>

#include "nulldriver.h"

extern "C" {

GLboolean GLEW_VERSION_2_0 = 1;

GLenum glewInit ()
{
	return GLEW_OK;
}

void glValidateProgram(GLuint program)
{
}

void glDepthFunc (GLenum func)
{
}

void glGetShaderSource (GLuint shader, GLsizei bufsize, GLsizei* length, char* source)
{
}

void glBindFramebuffer (GLenum target, GLuint framebuffer)
{
}

void glShaderBinary (GLint n, const GLuint* shaders, GLenum binaryformat, const void* binary, GLint length)
{
}

GLboolean glIsShader(GLuint shader)
{
	return true;
}

void glCompressedTexImage1D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const GLvoid *data)
{
}

void glCompressedTexImage2D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void* data)
{
}


void glDeleteTextures(GLsizei n, const GLuint* textures)
{
}

void glScissor (GLint x, GLint y, GLsizei width, GLsizei height)
{
}

void glBegin (GLenum mode)
{
}

void glColorMask (GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)
{
}

void glDisableClientState (GLenum array)
{
}

void glEnableClientState (GLenum array)
{
}

void glEnd (void)
{
}

void glVertex3fv (const GLfloat *v)
{
}

void glUniformMatrix3fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
}

void glUniform4f (GLint location, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
}

void glBeginQueryARB (GLenum target, GLuint id)
{
}

void glEndQueryARB (GLenum target)
{
}

void glGenQueriesARB (GLsizei n, GLuint* ids)
{
	for (int i=0; i<n; i++)
		ids[i] = 1+i;
}

void glGetQueryObjectivARB (GLuint id, GLenum pname, GLint* params)
{
	*params = 0;
}

void glGetQueryObjectuivARB (GLuint id, GLenum pname, GLuint* params)
{
	*params = 0;
}

void glUniformMatrix3fvARB (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
}

void glBindRenderbuffer (GLenum target, GLuint renderbuffer)
{
}

void glFramebufferRenderbuffer (GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer)
{
}

void glGenRenderbuffers (GLsizei n, GLuint* renderbuffers)
{
	for (int i=0; i<n; i++)
		renderbuffers[i] = i+1;
}

void glRenderbufferStorage (GLenum target, GLenum internalformat, GLsizei width, GLsizei height)
{
}

void glDetachShader (GLuint program, GLuint shader)
{
}

void glBufferData (GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage)
{
}

GLenum glCheckFramebufferStatus (GLenum target)
{
	return GL_FRAMEBUFFER_COMPLETE_EXT; 
}

void glFramebufferTexture2D (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
}

void glGenFramebuffers (GLsizei n, GLuint* framebuffers)
{
	for (int i=0; i<n; i++)
		framebuffers[i] = 1+i;
}

void glGenBuffers(GLsizei n, GLuint* buffers)
{
	for (int i=0; i<n; i++)
		buffers[i] = 1+i;
}

void glBindBuffer(GLenum target, GLuint buffer)
{
}

void glBindBufferBase (GLenum target, GLuint index, GLuint buffer)
{
}

void glBindVertexArray (GLuint array)
{
}

void glDeleteVertexArrays (GLsizei n, const GLuint* arrays)
{
}

void glGenVertexArrays (GLsizei n, GLuint* arrays)
{
}

void glDrawArraysInstanced (GLenum mode, GLint first, GLsizei count, GLsizei instanceCount)
{
}

GLuint glGetUniformBlockIndex (GLuint program, const GLchar* uniformBlockName)
{
	return 1;
}

void   glUniformBlockBinding (GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding)
{
}

void glInvalidateFramebuffer (GLenum target, GLsizei numAttachments, const GLenum* attachments)
{
}

void glColor3f (GLfloat red, GLfloat green, GLfloat blue)
{
}

GLenum glGetError (void)
{
	return GL_NO_ERROR;
}

const GLubyte *glGetString (GLenum name)
{
	if(name == GL_VENDOR)
		return (GLubyte*)"Kishonti Ltd";
	if(name == GL_RENDERER)
		return (GLubyte*)"Kishonti OpenGL Nulldriver";
	if(name == GL_VERSION)
		return (GLubyte*)"2.0";
	if(name == GL_EXTENSIONS)
		return (GLubyte*)"GL_ARB_multitexture GL_EXT_texture_env_add GL_EXT_compiled_vertex_array GL_S3_s3tc "
		"GL_ARB_depth_texture GL_ARB_fragment_program GL_ARB_fragment_program_shadow GL_ARB_fragment_shader "
		"GL_ARB_multisample GL_ARB_occlusion_query GL_ARB_point_parameters GL_ARB_point_sprite "
		"GL_ARB_shader_objects GL_ARB_shading_language_100 GL_ARB_shadow GL_ARB_shadow_ambient "
		"GL_ARB_texture_border_clamp GL_ARB_texture_compression GL_ARB_texture_cube_map GL_ARB_texture_env_add "
		"GL_ARB_texture_env_combine GL_ARB_texture_env_crossbar GL_ARB_texture_env_dot3 GL_ARB_texture_float "
		"GL_ARB_texture_mirrored_repeat GL_ARB_texture_rectangle GL_ARB_transpose_matrix GL_ARB_vertex_blend "
		"GL_ARB_vertex_buffer_object GL_ARB_pixel_buffer_object GL_ARB_vertex_program GL_ARB_vertex_shader "
		"GL_ARB_window_pos GL_ARB_draw_buffers GL_ATI_draw_buffers GL_ATI_element_array GL_ATI_envmap_bumpmap "
		"GL_ATI_fragment_shader GL_ATI_map_object_buffer GL_ATI_separate_stencil GL_ATI_shader_texture_lod "
		"GL_ATI_texture_compression_3dc GL_ATI_texture_env_combine3 GL_ATI_texture_float "
		"GL_ATI_texture_mirror_once GL_ATI_vertex_array_object GL_ATI_vertex_attrib_array_object "
		"GL_ATI_vertex_streams GL_ATIX_texture_env_combine3 GL_ATIX_texture_env_route "
		"GL_ATIX_vertex_shader_output_point_size GL_EXT_abgr GL_EXT_bgra GL_EXT_blend_color "
		"GL_EXT_blend_func_separate GL_EXT_blend_minmax GL_EXT_blend_subtract GL_EXT_clip_volume_hint "
		"GL_EXT_draw_range_elements GL_EXT_fog_coord GL_EXT_framebuffer_object GL_EXT_multi_draw_arrays "
		"GL_EXT_packed_pixels GL_EXT_point_parameters GL_EXT_rescale_normal GL_EXT_secondary_color "
		"GL_EXT_separate_specular_color GL_EXT_shadow_funcs GL_EXT_stencil_wrap GL_EXT_texgen_reflection "
		"GL_EXT_texture3D GL_EXT_texture_compression_s3tc GL_EXT_texture_cube_map GL_EXT_texture_edge_clamp "
		"GL_EXT_texture_env_combine GL_EXT_texture_env_dot3 GL_EXT_texture_filter_anisotropic "
		"GL_EXT_texture_lod_bias GL_EXT_texture_mirror_clamp GL_EXT_texture_object GL_EXT_texture_rectangle "
		"GL_EXT_vertex_array GL_EXT_vertex_shader GL_HP_occlusion_test GL_NV_blend_square GL_NV_occlusion_query "
		"GL_NV_texgen_reflection GL_SGI_color_matrix GL_SGIS_generate_mipmap GL_SGIS_multitexture "
		"GL_SGIS_texture_border_clamp GL_SGIS_texture_edge_clamp GL_SGIS_texture_lod GL_SUN_multi_draw_arrays "
		"GL_WIN_swap_hint WGL_EXT_extensions_string WGL_EXT_swap_control ";
	return (GLubyte*)"";
}

void glLoadMatrixf (const GLfloat *m)
{
}

void glMatrixMode (GLenum mode)
{
}

void glMultMatrixf (const GLfloat *m)
{
}

void glVertexPointer (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
}

void glActiveTexture (GLenum texture)
{
}

void glAttachShader (GLuint program, GLuint shader)
{
}

void glBindAttribLocation (GLuint program, GLuint index, const GLchar* name)
{
}

void glBindTexture (GLenum target, GLuint texture)
{
}

void glBlendFunc (GLenum sfactor, GLenum dfactor)
{
}

void glBlendEquation (GLenum mode)
{
}

void glClear (GLbitfield mask)
{
}

void glClearColor (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
{
}

void glCompileShader (GLuint shader)
{
}

void glCopyTexImage2D (GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border)
{
}

GLuint glCreateProgram (void)
{
	static GLuint prg = 0x8000000;
	return prg++;
}

GLuint glCreateShader (GLenum type)
{
	static GLuint shd = 0x0000000;
	return ++shd;
}

void glCullFace (GLenum mode)
{
}

void glDepthMask (GLboolean flag)
{
}

void glDeleteFramebuffers(GLsizei n, const GLuint* textures)
{
}


void glDeleteRenderbuffers(GLsizei n, const GLuint* textures)
{
}


void glBlendColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
}

void glBeginTransformFeedback (GLenum primitiveMode)
{
}

void glEndTransformFeedback (void)
{
}

void glTransformFeedbackVaryings (GLuint program, GLsizei count, const GLchar* const* varyings, GLenum bufferMode)
{
}

void glBindSampler (GLuint unit, GLuint sampler)
{
}

void glSamplerParameteri (GLuint sampler, GLenum pname, GLint param)
{
}

void glGenSamplers (GLsizei count, GLuint* samplers)
{
}

void glDeleteSamplers (GLsizei count, const GLuint* samplers)
{
}

void glDisable (GLenum cap)
{
}

void glDisableVertexAttribArray (GLuint)
{
}

void glDrawElements (GLenum mode, GLsizei count, GLenum type, const GLvoid *indices)
{
}


void glDepthRangef(GLclampf nearVal,  GLclampf farVal)
{
}


void glDrawArrays (GLenum mode, GLint first, GLsizei count)
{
}

void glEnable (GLenum cap)
{
}

void glEnableVertexAttribArray (GLuint)
{
}

void glFrontFace (GLenum mode)
{
}

void glGenTextures (GLsizei n, GLuint *textures)
{
	for (int i=0; i<n; i++)
		textures[i] = 1+i;
}

void glGetIntegerv (GLenum pname, GLint *params)
{
}

void glGetProgramInfoLog (GLuint program, GLsizei bufSize, GLsizei* length, GLchar* infoLog)
{
	infoLog[0]=0;
}

void glGetProgramiv (GLuint program, GLenum pname, GLint* param)
{
	//if (pname == GL_INFO_LOG_LENGTH)
	//	*param = 10;
	//else if (pname == GL_LINK_STATUS)
	*param = 1;
}

void glGetShaderInfoLog (GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* infoLog)
{
	infoLog[0]=0;
}

void glGetShaderiv (GLuint shader, GLenum pname, GLint* param)
{
	if(param)
		*param=1;
}

void glLinkProgram (GLuint program)
{
}

void glReadPixels (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels)
{
}

void glShaderSource (GLuint shader, GLsizei count, const GLchar** strings, const GLint* lengths)
{
}

void glTexImage1D (GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels)
{
}

void glTexImage2D (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels)
{
}


void glTexParameterf (GLenum target, GLenum pname, GLfloat param)
{
}

void glTexStorage2D (GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height)
{
}

void glTexParameteri (GLenum target, GLenum pname, GLint param)
{
}

void glUniform1f (GLint location, GLfloat v0)
{
}

void glUniform3f (GLint location, GLfloat v0, GLfloat v1, GLfloat v2)
{
}

void glUniform1i (GLint location, GLint v0)
{
}

void glUniform2f (GLint location, GLfloat x, GLfloat y)
{
}

void glUniform2fv (GLint location, GLsizei count, const GLfloat* value)
{
}

void glUniform3fv (GLint location, GLsizei count, const GLfloat* value)
{
}

void glUniform4i (GLint location, GLint x, GLint y, GLint z, GLint w)
{
}

void glUniform4fv (GLint location, GLsizei count, const GLfloat* value)
{
}

void glUniformMatrix4fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
}

void glUseProgram (GLuint program)
{
}

void glVertexAttribPointer (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* pointer)
{
}

void glViewport (GLint x, GLint y, GLsizei width, GLsizei height)
{
}


GLint glGetAttribLocation (GLuint program, const GLchar* name)
{
	return 1;
}

GLint glGetUniformLocation(GLint programObj, const GLchar* name)
{
	return 1;
}

void glGenerateMipmap(GLenum target)
{
}


void glGenQueries (GLsizei n, GLuint* ids)
{
}

void glDeleteQueries (GLsizei n, const GLuint* ids)
{
}

GLboolean glIsQuery (GLuint id)
{
	return true;
}

void glBeginQuery (GLenum target, GLuint id)
{
}

void glEndQuery (GLenum target)
{
}

void glGetQueryiv (GLenum target, GLenum pname, GLint* params)
{
}

void glGetQueryObjectuiv (GLuint id, GLenum pname, GLuint* params)
{
}

void glDrawElementsInstanced (GLenum mode, GLsizei count, GLenum type, const GLvoid* indices, GLsizei instanceCount)
{
}

void glVertexAttribDivisor (GLuint index, GLuint divisor)
{
}

void glDrawBuffers (GLsizei n, const GLenum* bufs)
{
}

void glTexSubImage2D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid* pixels)
{
}

GLvoid* glMapBufferRange (GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access)
{
	return 0;
}

GLboolean glUnmapBuffer (GLenum target)
{
	return true;
}

void glTexImage3D (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid* pixels)
{
}

void glTexSubImage3D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid* pixels)
{
}

void glDeleteBuffers(GLsizei n, const GLuint* buffers)
{
}

void glDeleteProgram (GLuint program)
{
}

GLboolean glIsProgram (GLuint program)
{
	return true;
}

void glBufferSubData (GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid* data)
{
}

void glGetBufferParameteriv (GLenum target, GLenum pname, GLint* params)
{
}

void glPixelStorei (GLenum pname, GLint param)
{
}

void glPolygonOffset (GLfloat factor, GLfloat units)
{
}

void glUniform1iv (GLint location, GLsizei count, const GLint* v)
{
}

void glDeleteShader (GLuint shader)
{
}

void glGetAttachedShaders (GLuint program, GLsizei maxCount, GLsizei* count, GLuint* shaders)
{
	if (count) *count = 0;
}

void glGetTexLevelParameteriv (GLenum target, GLint level, GLenum pname, GLint *params)
{
	*params = 0;
}

void glGetTexImage (GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels)
{
}

void glGetBooleanv(GLenum cap, GLboolean *param)
{
	if (cap == GL_SHADER_COMPILER)
		*param = 1;
}

void glFinish()
{
}

void glFlush()
{
}

GLboolean glIsEnabled (GLenum cap)
{
	return GL_TRUE;
}


void glGetVertexAttribiv (GLuint index, GLenum pname, GLint* params)
{
	params[0] = 0;
}



}


#endif
