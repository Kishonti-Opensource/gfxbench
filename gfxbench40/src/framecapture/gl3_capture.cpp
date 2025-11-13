/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "gl3_capture.h"
#include "gl_capture_helper.h"

#pragma push_macro("glActiveTexture")
#undef glActiveTexture
void glActiveTexture (GLenum texture) {
#pragma pop_macro("glActiveTexture")
	if( enable_logging) {
		log_cpp(" glActiveTexture( %d);\n", texture);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glActiveTexture\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glActiveTexture(texture);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glActiveTexture");
#endif
}

#pragma push_macro("glAttachShader")
#undef glAttachShader
void glAttachShader (GLuint program, GLuint shader) {
#pragma pop_macro("glAttachShader")
	if( enable_logging) {
		log_cpp(" if (( g_GLids[ %d] != -1) && ( g_GLids[ %d] != -1)) {\nglAttachShader( g_GLids[ %d], g_GLids[ %d]);\n}\n", program, shader, program, shader);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glAttachShader\", __FILE__, __LINE__);\n");
#endif
	}
	program = g_GLids[program];
	if (program==-1) return;
	shader = g_GLids[shader];
	if (shader==-1) return;
	Original_GL::glAttachShader(program, shader);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glAttachShader");
#endif
}

#pragma push_macro("glBindAttribLocation")
#undef glBindAttribLocation
void glBindAttribLocation (GLuint program, GLuint index, const GLchar* name) {
#pragma pop_macro("glBindAttribLocation")
	if( enable_logging) {
		log_cpp(" if ( g_GLids[ %d] != -1) {\nglBindAttribLocation( g_GLids[ %d], %d, \"%s\");\n}\n", program, program, index, name);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glBindAttribLocation\", __FILE__, __LINE__);\n");
#endif
	}
	program = g_GLids[program];
	if (program==-1) return;
	Original_GL::glBindAttribLocation(program, index, name);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glBindAttribLocation");
#endif
}

#pragma push_macro("glBindBuffer")
#undef glBindBuffer
void glBindBuffer (GLenum target, GLuint buffer) {
#pragma pop_macro("glBindBuffer")
	if( enable_logging) {
		log_cpp(" if ( g_GLids[ %d] != -1) {\nglBindBuffer( %d, g_GLids[ %d]);\n}\n", buffer, target, buffer);
		if( target == GL_ARRAY_BUFFER)
			current_vbo = buffer;
		if( target == GL_ELEMENT_ARRAY_BUFFER)
			current_ebo = buffer;
		if( target == GL_PIXEL_UNPACK_BUFFER)
			current_pbo = buffer;
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glBindBuffer\", __FILE__, __LINE__);\n");
#endif
	}
	buffer = g_GLids[buffer];
	if (buffer==-1) return;
	Original_GL::glBindBuffer(target, buffer);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glBindBuffer");
#endif
}

#pragma push_macro("glBindFramebuffer")
#undef glBindFramebuffer
void glBindFramebuffer (GLenum target, GLuint framebuffer) {
#pragma pop_macro("glBindFramebuffer")
	if( enable_logging) {
		log_cpp(" if ( g_GLids[ %d] != -1) {\nglBindFramebuffer( %d, g_GLids[ %d]);\n}\n", framebuffer, target, framebuffer);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glBindFramebuffer\", __FILE__, __LINE__);\n");
#endif
	}
	framebuffer = g_GLids[framebuffer];
	if (framebuffer==-1) return;
	Original_GL::glBindFramebuffer(target, framebuffer);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glBindFramebuffer");
#endif
}

#pragma push_macro("glBindRenderbuffer")
#undef glBindRenderbuffer
void glBindRenderbuffer (GLenum target, GLuint renderbuffer) {
#pragma pop_macro("glBindRenderbuffer")
	if( enable_logging) {
		log_cpp(" if ( g_GLids[ %d] != -1) {\nglBindRenderbuffer( %d, g_GLids[ %d]);\n}\n", renderbuffer, target, renderbuffer);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glBindRenderbuffer\", __FILE__, __LINE__);\n");
#endif
	}
	renderbuffer = g_GLids[renderbuffer];
	if (renderbuffer==-1) return;
	Original_GL::glBindRenderbuffer(target, renderbuffer);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glBindRenderbuffer");
#endif
}

#pragma push_macro("glBindTexture")
#undef glBindTexture
void glBindTexture (GLenum target, GLuint texture) {
#pragma pop_macro("glBindTexture")
	if( enable_logging) {
		log_cpp(" if ( g_GLids[ %d] != -1) {\nglBindTexture( %d, g_GLids[ %d]);\n}\n", texture, target, texture);
		current_to = texture;
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glBindTexture\", __FILE__, __LINE__);\n");
#endif
	}
	texture = g_GLids[texture];
	if (texture==-1) return;
	Original_GL::glBindTexture(target, texture);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glBindTexture");
#endif
}

#pragma push_macro("glBlendColor")
#undef glBlendColor
void glBlendColor (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) {
#pragma pop_macro("glBlendColor")
	if( enable_logging) {
		log_cpp(" glBlendColor( %#gf, %#gf, %#gf, %#gf);\n", float_sane( &red ), float_sane( &green ), float_sane( &blue ), float_sane( &alpha ));
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glBlendColor\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glBlendColor(red, green, blue, alpha);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glBlendColor");
#endif
}

#pragma push_macro("glBlendEquation")
#undef glBlendEquation
void glBlendEquation (GLenum mode) {
#pragma pop_macro("glBlendEquation")
	if( enable_logging) {
		log_cpp(" glBlendEquation( %d);\n", mode);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glBlendEquation\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glBlendEquation(mode);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glBlendEquation");
#endif
}

#pragma push_macro("glBlendEquationSeparate")
#undef glBlendEquationSeparate
void glBlendEquationSeparate (GLenum modeRGB, GLenum modeAlpha) {
#pragma pop_macro("glBlendEquationSeparate")
	if( enable_logging) {
		log_cpp(" glBlendEquationSeparate( %d, %d);\n", modeRGB, modeAlpha);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glBlendEquationSeparate\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glBlendEquationSeparate(modeRGB, modeAlpha);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glBlendEquationSeparate");
#endif
}

#pragma push_macro("glBlendFunc")
#undef glBlendFunc
void glBlendFunc (GLenum sfactor, GLenum dfactor) {
#pragma pop_macro("glBlendFunc")
	if( enable_logging) {
		log_cpp(" glBlendFunc( %d, %d);\n", sfactor, dfactor);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glBlendFunc\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glBlendFunc(sfactor, dfactor);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glBlendFunc");
#endif
}

#pragma push_macro("glBlendFuncSeparate")
#undef glBlendFuncSeparate
void glBlendFuncSeparate (GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha) {
#pragma pop_macro("glBlendFuncSeparate")
	if( enable_logging) {
		log_cpp(" glBlendFuncSeparate( %d, %d, %d, %d);\n", srcRGB, dstRGB, srcAlpha, dstAlpha);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glBlendFuncSeparate\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glBlendFuncSeparate(srcRGB, dstRGB, srcAlpha, dstAlpha);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glBlendFuncSeparate");
#endif
}

#pragma push_macro("glBufferData")
#undef glBufferData
void glBufferData (GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage) {
#pragma pop_macro("glBufferData")
	if( enable_logging) {
		if (data==NULL)
		{
			log_cpp("delete dr; dr = 0;\n");
			log_cpp("dr = new DataReader( 0, %d);\n", size);
		} else {
			int i = -1;
			if( target == GL_ARRAY_BUFFER)
				i = DataWrite( current_vbo , size, (const char*)data );
			if( target == GL_ELEMENT_ARRAY_BUFFER)
				i = DataWrite( current_ebo , size, (const char*)data );
			if( target == GL_PIXEL_UNPACK_BUFFER)
				i = DataWrite( current_pbo , size, (const char*)data );
			log_cpp("delete dr; dr = 0;\n");
			log_cpp("dr = new DataReader( %d);\n", i);
		}
		log_cpp(" glBufferData( %d, dr->Size(), dr->Data(), %d);\n", target, usage);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glBufferData\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glBufferData(target, size, data, usage);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glBufferData");
#endif
}

#pragma push_macro("glBufferSubData")
#undef glBufferSubData
void glBufferSubData (GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid* data) {
#pragma pop_macro("glBufferSubData")
	if( enable_logging) {
		int i = -1;
		if( target == GL_ARRAY_BUFFER)
			i = DataWrite( current_vbo , size, (const char*)data );
		if( target == GL_ELEMENT_ARRAY_BUFFER)
			i = DataWrite( current_ebo , size, (const char*)data );
		if( target == GL_PIXEL_UNPACK_BUFFER)
			i = DataWrite( current_pbo , size, (const char*)data );
		log_cpp("delete dr; dr = 0;\n");
		log_cpp("dr = new DataReader( %d);\n", i);
		log_cpp(" glBufferSubData( %d, %d, dr->Size(), dr->Data());\n", target, offset);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glBufferSubData\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glBufferSubData(target, offset, size, data);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glBufferSubData");
#endif
}

#pragma push_macro("glCheckFramebufferStatus")
#undef glCheckFramebufferStatus
GLenum glCheckFramebufferStatus (GLenum target) {
#pragma pop_macro("glCheckFramebufferStatus")
	if( enable_logging) {
		log_cpp(" glCheckFramebufferStatus( %d);\n", target);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glCheckFramebufferStatus\", __FILE__, __LINE__);\n");
#endif
	}
	GLenum retval = (GLenum)Original_GL::glCheckFramebufferStatus(target);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glCheckFramebufferStatus");
#endif
	return retval;
}

#pragma push_macro("glClear")
#undef glClear
void glClear (GLbitfield mask) {
#pragma pop_macro("glClear")
	if( enable_logging) {
		log_cpp(" glClear( %d);\n", mask);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glClear\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glClear(mask);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glClear");
#endif
}

#pragma push_macro("glClearColor")
#undef glClearColor
void glClearColor (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) {
#pragma pop_macro("glClearColor")
	if( enable_logging) {
		log_cpp(" glClearColor( %#gf, %#gf, %#gf, %#gf);\n", float_sane( &red ), float_sane( &green ), float_sane( &blue ), float_sane( &alpha ));
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glClearColor\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glClearColor(red, green, blue, alpha);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glClearColor");
#endif
}

#pragma push_macro("glClearDepthf")
#undef glClearDepthf
void glClearDepthf (GLfloat depth) {
#pragma pop_macro("glClearDepthf")
	if( enable_logging) {
		log_cpp(" glClearDepthf( %#gf);\n", float_sane( &depth ));
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glClearDepthf\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glClearDepthf(depth);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glClearDepthf");
#endif
}

#pragma push_macro("glClearStencil")
#undef glClearStencil
void glClearStencil (GLint s) {
#pragma pop_macro("glClearStencil")
	if( enable_logging) {
		log_cpp(" glClearStencil( %d);\n", s);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glClearStencil\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glClearStencil(s);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glClearStencil");
#endif
}

#pragma push_macro("glColorMask")
#undef glColorMask
void glColorMask (GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha) {
#pragma pop_macro("glColorMask")
	if( enable_logging) {
		log_cpp(" glColorMask( %d, %d, %d, %d);\n", red, green, blue, alpha);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glColorMask\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glColorMask(red, green, blue, alpha);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glColorMask");
#endif
}

#pragma push_macro("glCompileShader")
#undef glCompileShader
void glCompileShader (GLuint shader) {
#pragma pop_macro("glCompileShader")
	if( enable_logging) {
		log_cpp(" if ( g_GLids[ %d] != -1) {\nglCompileShader( g_GLids[ %d]);\n}\n", shader, shader);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glCompileShader\", __FILE__, __LINE__);\n");
#endif
	}
	shader = g_GLids[shader];
	if (shader==-1) return;
	Original_GL::glCompileShader(shader);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glCompileShader");
#endif
}

#pragma push_macro("glCompressedTexImage2D")
#undef glCompressedTexImage2D
void glCompressedTexImage2D (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid* data) {
#pragma pop_macro("glCompressedTexImage2D")
	if( enable_logging) {
		if( data && !current_pbo)
		{
			int i = DataWrite( current_to , imageSize, (const char*)data);
			log_cpp( "delete dr;\n");
			log_cpp( "dr = new DataReader( %d);\n", i);
			log_cpp( "glCompressedTexImage2D( %d, %d, %d, %d, %d, %d, dr->Size(), dr->Data());\n", target, level, internalformat, width, height, border);
		} else {
			log_cpp( "glCompressedTexImage2D( %d, %d, %d, %d, %d, %d, %d, (void*)0x%x);\n", target, level, internalformat, width, height, border, imageSize, data);
		}
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glCompressedTexImage2D\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glCompressedTexImage2D(target, level, internalformat, width, height, border, imageSize, data);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glCompressedTexImage2D");
#endif
}

#pragma push_macro("glCompressedTexSubImage2D")
#undef glCompressedTexSubImage2D
void glCompressedTexSubImage2D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid* data) {
#pragma pop_macro("glCompressedTexSubImage2D")
	if( enable_logging) {
		if( data && !current_pbo)
		{
			int i = DataWrite( current_to , imageSize, (const char*)data);
			log_cpp( "delete dr;\n");
			log_cpp( "dr = new DataReader( %d);\n", i);
			log_cpp( "glCompressedTexSubImage2D( %d, %d, %d, %d, %d, %d, %d, dr->Size(), dr->Data());\n", target, level, xoffset ,yoffset, width, height, format);
		} else {
			log_cpp( "glCompressedTexSubImage2D( %d, %d, %d, %d, %d, %d, %d, %d, (void*)0x%x);\n", target, level, xoffset, yoffset, width, height, format, imageSize, data);
		}
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glCompressedTexSubImage2D\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glCompressedTexSubImage2D(target, level, xoffset, yoffset, width, height, format, imageSize, data);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glCompressedTexSubImage2D");
#endif
}

#pragma push_macro("glCopyTexImage2D")
#undef glCopyTexImage2D
void glCopyTexImage2D (GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border) {
#pragma pop_macro("glCopyTexImage2D")
	if( enable_logging) {
		log_cpp(" glCopyTexImage2D( %d, %d, %d, %d, %d, %d, %d, %d);\n", target, level, internalformat, x, y, width, height, border);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glCopyTexImage2D\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glCopyTexImage2D(target, level, internalformat, x, y, width, height, border);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glCopyTexImage2D");
#endif
}

#pragma push_macro("glCopyTexSubImage2D")
#undef glCopyTexSubImage2D
void glCopyTexSubImage2D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height) {
#pragma pop_macro("glCopyTexSubImage2D")
	if( enable_logging) {
		log_cpp(" glCopyTexSubImage2D( %d, %d, %d, %d, %d, %d, %d, %d);\n", target, level, xoffset, yoffset, x, y, width, height);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glCopyTexSubImage2D\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glCopyTexSubImage2D(target, level, xoffset, yoffset, x, y, width, height);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glCopyTexSubImage2D");
#endif
}

#pragma push_macro("glCreateProgram")
#undef glCreateProgram
GLuint glCreateProgram (void) {
#pragma pop_macro("glCreateProgram")
	if( enable_logging) {
		log_cpp(" g_GLids.push_back( glCreateProgram() );\n");
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glCreateProgram\", __FILE__, __LINE__);\n");
#endif
	}
	GLint tmp = Original_GL::glCreateProgram();
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glCreateProgram");
#endif
	g_GLids.push_back( tmp);
	return g_GLids.size() - 1;
}

#pragma push_macro("glCreateShader")
#undef glCreateShader
GLuint glCreateShader (GLenum type) {
#pragma pop_macro("glCreateShader")
	if( enable_logging) {
		log_cpp(" g_GLids.push_back( glCreateShader( %d) );\n", type);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glCreateShader\", __FILE__, __LINE__);\n");
#endif
	}
	GLint tmp = Original_GL::glCreateShader(type);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glCreateShader");
#endif
	g_GLids.push_back( tmp);
	return g_GLids.size() - 1;
}

#pragma push_macro("glCullFace")
#undef glCullFace
void glCullFace (GLenum mode) {
#pragma pop_macro("glCullFace")
	if( enable_logging) {
		log_cpp(" glCullFace( %d);\n", mode);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glCullFace\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glCullFace(mode);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glCullFace");
#endif
}

#pragma push_macro("glDeleteBuffers")
#undef glDeleteBuffers
void glDeleteBuffers (GLsizei n, const GLuint* buffers) {
#pragma pop_macro("glDeleteBuffers")
	//Original_GL::glDeleteBuffers(n, buffers);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glDeleteBuffers");
#endif
}

#pragma push_macro("glDeleteFramebuffers")
#undef glDeleteFramebuffers
void glDeleteFramebuffers (GLsizei n, const GLuint* framebuffers) {
#pragma pop_macro("glDeleteFramebuffers")
	if( enable_logging) {
		log_cpp(" glDeleteFramebuffers( %d, (void*)0x%x);\n", n, framebuffers);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glDeleteFramebuffers\", __FILE__, __LINE__);\n");
#endif
	}
	//Original_GL::glDeleteFramebuffers(n, framebuffers);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glDeleteFramebuffers");
#endif
}

#pragma push_macro("glDeleteProgram")
#undef glDeleteProgram
void glDeleteProgram (GLuint program) {
#pragma pop_macro("glDeleteProgram")
	//Original_GL::glDeleteProgram(program);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glDeleteProgram");
#endif
}

#pragma push_macro("glDeleteRenderbuffers")
#undef glDeleteRenderbuffers
void glDeleteRenderbuffers (GLsizei n, const GLuint* renderbuffers) {
#pragma pop_macro("glDeleteRenderbuffers")
	if( enable_logging) {
		log_cpp(" glDeleteRenderbuffers( %d, (void*)0x%x);\n", n, renderbuffers);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glDeleteRenderbuffers\", __FILE__, __LINE__);\n");
#endif
	}
	//Original_GL::glDeleteRenderbuffers(n, renderbuffers);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glDeleteRenderbuffers");
#endif
}

#pragma push_macro("glDeleteShader")
#undef glDeleteShader
void glDeleteShader (GLuint shader) {
#pragma pop_macro("glDeleteShader")
	//Original_GL::glDeleteShader(shader);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glDeleteShader");
#endif
}

#pragma push_macro("glDeleteTextures")
#undef glDeleteTextures
void glDeleteTextures (GLsizei n, const GLuint* textures) {
#pragma pop_macro("glDeleteTextures")
	//Original_GL::glDeleteTextures(n, textures);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glDeleteTextures");
#endif
}

#pragma push_macro("glDepthFunc")
#undef glDepthFunc
void glDepthFunc (GLenum func) {
#pragma pop_macro("glDepthFunc")
	if( enable_logging) {
		log_cpp(" glDepthFunc( %d);\n", func);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glDepthFunc\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glDepthFunc(func);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glDepthFunc");
#endif
}

#pragma push_macro("glDepthMask")
#undef glDepthMask
void glDepthMask (GLboolean flag) {
#pragma pop_macro("glDepthMask")
	if( enable_logging) {
		log_cpp(" glDepthMask( %d);\n", flag);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glDepthMask\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glDepthMask(flag);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glDepthMask");
#endif
}

#pragma push_macro("glDepthRangef")
#undef glDepthRangef
void glDepthRangef (GLfloat n, GLfloat f) {
#pragma pop_macro("glDepthRangef")
	if( enable_logging) {
		log_cpp(" glDepthRangef( %#gf, %#gf);\n", float_sane( &n ), float_sane( &f ));
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glDepthRangef\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glDepthRangef(n, f);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glDepthRangef");
#endif
}

#pragma push_macro("glDetachShader")
#undef glDetachShader
void glDetachShader (GLuint program, GLuint shader) {
#pragma pop_macro("glDetachShader")
	if( enable_logging) {
		log_cpp(" if (( g_GLids[ %d] != -1) && ( g_GLids[ %d] != -1)) {\nglDetachShader( g_GLids[ %d], g_GLids[ %d]);\n}\n", program, shader, program, shader);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glDetachShader\", __FILE__, __LINE__);\n");
#endif
	}
	program = g_GLids[program];
	if (program==-1) return;
	shader = g_GLids[shader];
	if (shader==-1) return;
	Original_GL::glDetachShader(program, shader);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glDetachShader");
#endif
}

#pragma push_macro("glDisable")
#undef glDisable
void glDisable (GLenum cap) {
#pragma pop_macro("glDisable")
	if( enable_logging) {
		log_cpp(" glDisable( %d);\n", cap);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glDisable\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glDisable(cap);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glDisable");
#endif
}

#pragma push_macro("glDisableVertexAttribArray")
#undef glDisableVertexAttribArray
void glDisableVertexAttribArray (GLuint index) {
#pragma pop_macro("glDisableVertexAttribArray")
	if( enable_logging) {
		log_cpp(" if ( g_GLids[ %d] != -1) {\nglDisableVertexAttribArray( g_GLids[ %d]);\n}\n", index, index);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glDisableVertexAttribArray\", __FILE__, __LINE__);\n");
#endif
	}
	index = g_GLids[index];
	if (index==-1) return;
	Original_GL::glDisableVertexAttribArray(index);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glDisableVertexAttribArray");
#endif
}

#pragma push_macro("glDrawArrays")
#undef glDrawArrays
void glDrawArrays (GLenum mode, GLint first, GLsizei count) {
#pragma pop_macro("glDrawArrays")
	if( enable_logging) {
		log_cpp(" glDrawArrays( %d, %d, %d);\n", mode, first, count);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glDrawArrays\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glDrawArrays(mode, first, count);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glDrawArrays");
#endif
}

#pragma push_macro("glDrawElements")
#undef glDrawElements
void glDrawElements (GLenum mode, GLsizei count, GLenum type, const GLvoid* indices) {
#pragma pop_macro("glDrawElements")
	if( enable_logging) {
		log_cpp(" glDrawElements( %d, %d, %d, (void*)0x%x);\n", mode, count, type, indices);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glDrawElements\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glDrawElements(mode, count, type, indices);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glDrawElements");
#endif
}

#pragma push_macro("glEnable")
#undef glEnable
void glEnable (GLenum cap) {
#pragma pop_macro("glEnable")
	if( enable_logging) {
		log_cpp(" glEnable( %d);\n", cap);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glEnable\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glEnable(cap);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glEnable");
#endif
}

#pragma push_macro("glEnableVertexAttribArray")
#undef glEnableVertexAttribArray
void glEnableVertexAttribArray (GLuint index) {
#pragma pop_macro("glEnableVertexAttribArray")
	if( enable_logging) {
		log_cpp(" if ( g_GLids[ %d] != -1) {\nglEnableVertexAttribArray( g_GLids[ %d]);\n}\n", index, index);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glEnableVertexAttribArray\", __FILE__, __LINE__);\n");
#endif
	}
	index = g_GLids[index];
	if (index==-1) return;
	Original_GL::glEnableVertexAttribArray(index);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glEnableVertexAttribArray");
#endif
}

#pragma push_macro("glFinish")
#undef glFinish
void glFinish (void) {
#pragma pop_macro("glFinish")
	if( enable_logging) {
		log_cpp(" glFinish();\n");
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glFinish\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glFinish();
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glFinish");
#endif
}

#pragma push_macro("glFlush")
#undef glFlush
void glFlush (void) {
#pragma pop_macro("glFlush")
	if( enable_logging) {
		log_cpp(" glFlush();\n");
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glFlush\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glFlush();
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glFlush");
#endif
}

#pragma push_macro("glFramebufferRenderbuffer")
#undef glFramebufferRenderbuffer
void glFramebufferRenderbuffer (GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer) {
#pragma pop_macro("glFramebufferRenderbuffer")
	if( enable_logging) {
		log_cpp(" if ( g_GLids[ %d] != -1) {\nglFramebufferRenderbuffer( %d, %d, %d, g_GLids[ %d]);\n}\n", renderbuffer, target, attachment, renderbuffertarget, renderbuffer);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glFramebufferRenderbuffer\", __FILE__, __LINE__);\n");
#endif
	}
	renderbuffer = g_GLids[renderbuffer];
	if (renderbuffer==-1) return;
	Original_GL::glFramebufferRenderbuffer(target, attachment, renderbuffertarget, renderbuffer);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glFramebufferRenderbuffer");
#endif
}

#pragma push_macro("glFramebufferTexture2D")
#undef glFramebufferTexture2D
void glFramebufferTexture2D (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level) {
#pragma pop_macro("glFramebufferTexture2D")
	if( enable_logging) {
		log_cpp(" if ( g_GLids[ %d] != -1) {\nglFramebufferTexture2D( %d, %d, %d, g_GLids[ %d], %d);\n}\n", texture, target, attachment, textarget, texture, level);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glFramebufferTexture2D\", __FILE__, __LINE__);\n");
#endif
	}
	texture = g_GLids[texture];
	if (texture==-1) return;
	Original_GL::glFramebufferTexture2D(target, attachment, textarget, texture, level);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glFramebufferTexture2D");
#endif
}

#pragma push_macro("glFrontFace")
#undef glFrontFace
void glFrontFace (GLenum mode) {
#pragma pop_macro("glFrontFace")
	if( enable_logging) {
		log_cpp(" glFrontFace( %d);\n", mode);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glFrontFace\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glFrontFace(mode);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glFrontFace");
#endif
}

#pragma push_macro("glGenBuffers")
#undef glGenBuffers
void glGenBuffers (GLsizei n, GLuint* buffers) {
#pragma pop_macro("glGenBuffers")
	if( enable_logging) {
		log_cpp(" for( size_t i=0; i<%d; i++)\n{\nGLuint tmp;\nglGenBuffers( 1, &tmp);\ng_GLids.push_back( tmp);\n}\n ", n);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glGenBuffers\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glGenBuffers(n, buffers);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glGenBuffers");
#endif
	for( GLsizei i=0; i<n; i++)
	{
		g_GLids.push_back(buffers[i]); 
		buffers[i] = g_GLids.size() - 1; 
	}
}

#pragma push_macro("glGenerateMipmap")
#undef glGenerateMipmap
void glGenerateMipmap (GLenum target) {
#pragma pop_macro("glGenerateMipmap")
	if( enable_logging) {
		log_cpp(" glGenerateMipmap( %d);\n", target);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glGenerateMipmap\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glGenerateMipmap(target);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glGenerateMipmap");
#endif
}

#pragma push_macro("glGenFramebuffers")
#undef glGenFramebuffers
void glGenFramebuffers (GLsizei n, GLuint* framebuffers) {
#pragma pop_macro("glGenFramebuffers")
	if( enable_logging) {
		log_cpp(" for( size_t i=0; i<%d; i++)\n{\nGLuint tmp;\nglGenFramebuffers( 1, &tmp);\ng_GLids.push_back( tmp);\n}\n ", n);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glGenFramebuffers\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glGenFramebuffers(n, framebuffers);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glGenFramebuffers");
#endif
	for( GLsizei i=0; i<n; i++)
	{
		g_GLids.push_back(framebuffers[i]); 
		framebuffers[i] = g_GLids.size() - 1; 
	}
}

#pragma push_macro("glGenRenderbuffers")
#undef glGenRenderbuffers
void glGenRenderbuffers (GLsizei n, GLuint* renderbuffers) {
#pragma pop_macro("glGenRenderbuffers")
	if( enable_logging) {
		log_cpp(" for( size_t i=0; i<%d; i++)\n{\nGLuint tmp;\nglGenRenderbuffers( 1, &tmp);\ng_GLids.push_back( tmp);\n}\n ", n);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glGenRenderbuffers\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glGenRenderbuffers(n, renderbuffers);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glGenRenderbuffers");
#endif
	for( GLsizei i=0; i<n; i++)
	{
		g_GLids.push_back(renderbuffers[i]); 
		renderbuffers[i] = g_GLids.size() - 1; 
	}
}

#pragma push_macro("glGenTextures")
#undef glGenTextures
void glGenTextures (GLsizei n, GLuint* textures) {
#pragma pop_macro("glGenTextures")
	if( enable_logging) {
		log_cpp(" for( size_t i=0; i<%d; i++)\n{\nGLuint tmp;\nglGenTextures( 1, &tmp);\ng_GLids.push_back( tmp);\n}\n ", n);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glGenTextures\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glGenTextures(n, textures);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glGenTextures");
#endif
	for( GLsizei i=0; i<n; i++)
	{
		g_GLids.push_back(textures[i]); 
		textures[i] = g_GLids.size() - 1; 
	}
}

#pragma push_macro("glGetActiveAttrib")
#undef glGetActiveAttrib
void glGetActiveAttrib (GLuint program, GLuint index, GLsizei bufsize, GLsizei* length, GLint* size, GLenum* type, GLchar* name) {
#pragma pop_macro("glGetActiveAttrib")
	if( enable_logging) {
		log_cpp(" glGetActiveAttrib( %d, %d, %d, (void*)0x%x, (void*)0x%x, (void*)0x%x, \"%s\");\n", program, index, bufsize, length, size, type, name);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glGetActiveAttrib\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glGetActiveAttrib(program, index, bufsize, length, size, type, name);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glGetActiveAttrib");
#endif
}

#pragma push_macro("glGetActiveUniform")
#undef glGetActiveUniform
void glGetActiveUniform (GLuint program, GLuint index, GLsizei bufsize, GLsizei* length, GLint* size, GLenum* type, GLchar* name) {
#pragma pop_macro("glGetActiveUniform")
	program = g_GLids[program];
	if (program==-1) return;
	Original_GL::glGetActiveUniform(program, index, bufsize, length, size, type, name);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glGetActiveUniform");
#endif
}

#pragma push_macro("glGetAttachedShaders")
#undef glGetAttachedShaders
void glGetAttachedShaders (GLuint program, GLsizei maxcount, GLsizei* count, GLuint* shaders) {
#pragma pop_macro("glGetAttachedShaders")
	program = g_GLids[program];
	if (program==-1) return;
	Original_GL::glGetAttachedShaders(program, maxcount, count, shaders);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glGetAttachedShaders");
#endif
}

#pragma push_macro("glGetAttribLocation")
#undef glGetAttribLocation
GLint glGetAttribLocation (GLuint program, const GLchar* name) {
#pragma pop_macro("glGetAttribLocation")
	if( enable_logging) {
		log_cpp(" if ( g_GLids[ %d] != -1) {\ng_GLids.push_back( glGetAttribLocation( g_GLids[ %d], \"%s\") );\n}\n", program, program, name);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glGetAttribLocation\", __FILE__, __LINE__);\n");
#endif
	}
	program = g_GLids[program];
	if (program==-1) return -1;
	GLint tmp = Original_GL::glGetAttribLocation(program, name);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glGetAttribLocation");
#endif
	g_GLids.push_back( tmp);
	return g_GLids.size() - 1;
}

#pragma push_macro("glGetBooleanv")
#undef glGetBooleanv
void glGetBooleanv (GLenum pname, GLboolean* params) {
#pragma pop_macro("glGetBooleanv")
	Original_GL::glGetBooleanv(pname, params);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glGetBooleanv");
#endif
}

#pragma push_macro("glGetBufferParameteriv")
#undef glGetBufferParameteriv
void glGetBufferParameteriv (GLenum target, GLenum pname, GLint* params) {
#pragma pop_macro("glGetBufferParameteriv")
	Original_GL::glGetBufferParameteriv(target, pname, params);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glGetBufferParameteriv");
#endif
}

#pragma push_macro("glGetError")
#undef glGetError
GLenum glGetError (void) {
#pragma pop_macro("glGetError")
	GLenum retval = (GLenum)Original_GL::glGetError();
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glGetError");
#endif
	return retval;
}

#pragma push_macro("glGetFloatv")
#undef glGetFloatv
void glGetFloatv (GLenum pname, GLfloat* params) {
#pragma pop_macro("glGetFloatv")
	Original_GL::glGetFloatv(pname, params);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glGetFloatv");
#endif
}

#pragma push_macro("glGetFramebufferAttachmentParameteriv")
#undef glGetFramebufferAttachmentParameteriv
void glGetFramebufferAttachmentParameteriv (GLenum target, GLenum attachment, GLenum pname, GLint* params) {
#pragma pop_macro("glGetFramebufferAttachmentParameteriv")
	Original_GL::glGetFramebufferAttachmentParameteriv(target, attachment, pname, params);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glGetFramebufferAttachmentParameteriv");
#endif
}

#pragma push_macro("glGetIntegerv")
#undef glGetIntegerv
void glGetIntegerv (GLenum pname, GLint* params) {
#pragma pop_macro("glGetIntegerv")
	Original_GL::glGetIntegerv(pname, params);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glGetIntegerv");
#endif
}

#pragma push_macro("glGetProgramiv")
#undef glGetProgramiv
void glGetProgramiv (GLuint program, GLenum pname, GLint* params) {
#pragma pop_macro("glGetProgramiv")
	program = g_GLids[program];
	if (program==-1) return;
	Original_GL::glGetProgramiv(program, pname, params);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glGetProgramiv");
#endif
}

#pragma push_macro("glGetProgramInfoLog")
#undef glGetProgramInfoLog
void glGetProgramInfoLog (GLuint program, GLsizei bufsize, GLsizei* length, GLchar* infolog) {
#pragma pop_macro("glGetProgramInfoLog")
	program = g_GLids[program];
	if (program==-1) return;
	Original_GL::glGetProgramInfoLog(program, bufsize, length, infolog);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glGetProgramInfoLog");
#endif
}

#pragma push_macro("glGetRenderbufferParameteriv")
#undef glGetRenderbufferParameteriv
void glGetRenderbufferParameteriv (GLenum target, GLenum pname, GLint* params) {
#pragma pop_macro("glGetRenderbufferParameteriv")
	if( enable_logging) {
		log_cpp(" glGetRenderbufferParameteriv( %d, %d, (void*)0x%x);\n", target, pname, params);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glGetRenderbufferParameteriv\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glGetRenderbufferParameteriv(target, pname, params);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glGetRenderbufferParameteriv");
#endif
}

#pragma push_macro("glGetShaderiv")
#undef glGetShaderiv
void glGetShaderiv (GLuint shader, GLenum pname, GLint* params) {
#pragma pop_macro("glGetShaderiv")
	shader = g_GLids[shader];
	if (shader==-1) return;
	Original_GL::glGetShaderiv(shader, pname, params);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glGetShaderiv");
#endif
}

#pragma push_macro("glGetShaderInfoLog")
#undef glGetShaderInfoLog
void glGetShaderInfoLog (GLuint shader, GLsizei bufsize, GLsizei* length, GLchar* infolog) {
#pragma pop_macro("glGetShaderInfoLog")
	shader = g_GLids[shader];
	if (shader==-1) return;
	Original_GL::glGetShaderInfoLog(shader, bufsize, length, infolog);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glGetShaderInfoLog");
#endif
}

#pragma push_macro("glGetShaderPrecisionFormat")
#undef glGetShaderPrecisionFormat
void glGetShaderPrecisionFormat (GLenum shadertype, GLenum precisiontype, GLint* range, GLint* precision) {
#pragma pop_macro("glGetShaderPrecisionFormat")
	if( enable_logging) {
		log_cpp(" glGetShaderPrecisionFormat( %d, %d, (void*)0x%x, (void*)0x%x);\n", shadertype, precisiontype, range, precision);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glGetShaderPrecisionFormat\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glGetShaderPrecisionFormat(shadertype, precisiontype, range, precision);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glGetShaderPrecisionFormat");
#endif
}

#pragma push_macro("glGetShaderSource")
#undef glGetShaderSource
void glGetShaderSource (GLuint shader, GLsizei bufsize, GLsizei* length, GLchar* source) {
#pragma pop_macro("glGetShaderSource")
	if( enable_logging) {
		log_cpp(" if ( g_GLids[ %d] != -1) {\nglGetShaderSource( g_GLids[ %d], %d, (void*)0x%x, \"%s\");\n}\n", shader, shader, bufsize, length, source);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glGetShaderSource\", __FILE__, __LINE__);\n");
#endif
	}
	shader = g_GLids[shader];
	if (shader==-1) return;
	Original_GL::glGetShaderSource(shader, bufsize, length, source);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glGetShaderSource");
#endif
}

#pragma push_macro("glGetString")
#undef glGetString
const GLubyte* glGetString (GLenum name) {
#pragma pop_macro("glGetString")
	GLubyte* retval = (GLubyte*)Original_GL::glGetString(name);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glGetString");
#endif
	return retval;
}

#pragma push_macro("glGetTexParameterfv")
#undef glGetTexParameterfv
void glGetTexParameterfv (GLenum target, GLenum pname, GLfloat* params) {
#pragma pop_macro("glGetTexParameterfv")
	if( enable_logging) {
		log_cpp(" glGetTexParameterfv( %d, %d, (void*)0x%x);\n", target, pname, params);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glGetTexParameterfv\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glGetTexParameterfv(target, pname, params);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glGetTexParameterfv");
#endif
}

#pragma push_macro("glGetTexParameteriv")
#undef glGetTexParameteriv
void glGetTexParameteriv (GLenum target, GLenum pname, GLint* params) {
#pragma pop_macro("glGetTexParameteriv")
	if( enable_logging) {
		log_cpp(" glGetTexParameteriv( %d, %d, (void*)0x%x);\n", target, pname, params);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glGetTexParameteriv\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glGetTexParameteriv(target, pname, params);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glGetTexParameteriv");
#endif
}

#pragma push_macro("glGetUniformfv")
#undef glGetUniformfv
void glGetUniformfv (GLuint program, GLint location, GLfloat* params) {
#pragma pop_macro("glGetUniformfv")
	if( enable_logging) {
		log_cpp(" if ( g_GLids[ %d] != -1) {\nglGetUniformfv( g_GLids[ %d], %d, (void*)0x%x);\n}\n", program, program, location, params);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glGetUniformfv\", __FILE__, __LINE__);\n");
#endif
	}
	program = g_GLids[program];
	if (program==-1) return;
	Original_GL::glGetUniformfv(program, location, params);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glGetUniformfv");
#endif
}

#pragma push_macro("glGetUniformiv")
#undef glGetUniformiv
void glGetUniformiv (GLuint program, GLint location, GLint* params) {
#pragma pop_macro("glGetUniformiv")
	if( enable_logging) {
		log_cpp(" if ( g_GLids[ %d] != -1) {\nglGetUniformiv( g_GLids[ %d], %d, (void*)0x%x);\n}\n", program, program, location, params);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glGetUniformiv\", __FILE__, __LINE__);\n");
#endif
	}
	program = g_GLids[program];
	if (program==-1) return;
	Original_GL::glGetUniformiv(program, location, params);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glGetUniformiv");
#endif
}

#pragma push_macro("glGetUniformLocation")
#undef glGetUniformLocation
GLint glGetUniformLocation (GLuint program, const GLchar* name) {
#pragma pop_macro("glGetUniformLocation")
	if( enable_logging) {
		log_cpp(" if ( g_GLids[ %d] != -1) {\ng_GLids.push_back( glGetUniformLocation( g_GLids[ %d], \"%s\") );\n}\n", program, program, name);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glGetUniformLocation\", __FILE__, __LINE__);\n");
#endif
	}
	program = g_GLids[program];
	if (program==-1) return -1;
	GLint tmp = Original_GL::glGetUniformLocation(program, name);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glGetUniformLocation");
#endif
	g_GLids.push_back( tmp);
	return g_GLids.size() - 1;
}

#pragma push_macro("glGetVertexAttribfv")
#undef glGetVertexAttribfv
void glGetVertexAttribfv (GLuint index, GLenum pname, GLfloat* params) {
#pragma pop_macro("glGetVertexAttribfv")
	if( enable_logging) {
		log_cpp(" glGetVertexAttribfv( %d, %d, (void*)0x%x);\n", index, pname, params);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glGetVertexAttribfv\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glGetVertexAttribfv(index, pname, params);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glGetVertexAttribfv");
#endif
}

#pragma push_macro("glGetVertexAttribiv")
#undef glGetVertexAttribiv
void glGetVertexAttribiv (GLuint index, GLenum pname, GLint* params) {
#pragma pop_macro("glGetVertexAttribiv")
	if( enable_logging) {
		log_cpp(" glGetVertexAttribiv( %d, %d, (void*)0x%x);\n", index, pname, params);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glGetVertexAttribiv\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glGetVertexAttribiv(index, pname, params);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glGetVertexAttribiv");
#endif
}

#pragma push_macro("glGetVertexAttribPointerv")
#undef glGetVertexAttribPointerv
void glGetVertexAttribPointerv (GLuint index, GLenum pname, GLvoid** pointer) {
#pragma pop_macro("glGetVertexAttribPointerv")
	if( enable_logging) {
		log_cpp(" glGetVertexAttribPointerv( %d, %d, (void*)0x%x);\n", index, pname, pointer);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glGetVertexAttribPointerv\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glGetVertexAttribPointerv(index, pname, pointer);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glGetVertexAttribPointerv");
#endif
}

#pragma push_macro("glHint")
#undef glHint
void glHint (GLenum target, GLenum mode) {
#pragma pop_macro("glHint")
	if( enable_logging) {
		log_cpp(" glHint( %d, %d);\n", target, mode);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glHint\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glHint(target, mode);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glHint");
#endif
}

#pragma push_macro("glIsBuffer")
#undef glIsBuffer
GLboolean glIsBuffer (GLuint buffer) {
#pragma pop_macro("glIsBuffer")
	buffer = g_GLids[buffer];
	if (buffer==-1) return false;
	GLboolean retval = (GLboolean)Original_GL::glIsBuffer(buffer);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glIsBuffer");
#endif
	return retval;
}

#pragma push_macro("glIsEnabled")
#undef glIsEnabled
GLboolean glIsEnabled (GLenum cap) {
#pragma pop_macro("glIsEnabled")
	GLboolean retval = (GLboolean)Original_GL::glIsEnabled(cap);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glIsEnabled");
#endif
	return retval;
}

#pragma push_macro("glIsFramebuffer")
#undef glIsFramebuffer
GLboolean glIsFramebuffer (GLuint framebuffer) {
#pragma pop_macro("glIsFramebuffer")
	framebuffer = g_GLids[framebuffer];
	if (framebuffer==-1) return false;
	GLboolean retval = (GLboolean)Original_GL::glIsFramebuffer(framebuffer);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glIsFramebuffer");
#endif
	return retval;
}

#pragma push_macro("glIsProgram")
#undef glIsProgram
GLboolean glIsProgram (GLuint program) {
#pragma pop_macro("glIsProgram")
	GLboolean retval = (GLboolean)Original_GL::glIsProgram(program);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glIsProgram");
#endif
	return retval;
}

#pragma push_macro("glIsRenderbuffer")
#undef glIsRenderbuffer
GLboolean glIsRenderbuffer (GLuint renderbuffer) {
#pragma pop_macro("glIsRenderbuffer")
	renderbuffer = g_GLids[renderbuffer];
	if (renderbuffer==-1) return false;
	GLboolean retval = (GLboolean)Original_GL::glIsRenderbuffer(renderbuffer);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glIsRenderbuffer");
#endif
	return retval;
}

#pragma push_macro("glIsShader")
#undef glIsShader
GLboolean glIsShader (GLuint shader) {
#pragma pop_macro("glIsShader")
	GLboolean retval = (GLboolean)Original_GL::glIsShader(shader);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glIsShader");
#endif
	return retval;
}

#pragma push_macro("glIsTexture")
#undef glIsTexture
GLboolean glIsTexture (GLuint texture) {
#pragma pop_macro("glIsTexture")
	texture = g_GLids[texture];
	if (texture==-1) return false;
	GLboolean retval = (GLboolean)Original_GL::glIsTexture(texture);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glIsTexture");
#endif
	return retval;
}

#pragma push_macro("glLineWidth")
#undef glLineWidth
void glLineWidth (GLfloat width) {
#pragma pop_macro("glLineWidth")
	if( enable_logging) {
		log_cpp(" glLineWidth( %#gf);\n", float_sane( &width ));
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glLineWidth\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glLineWidth(width);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glLineWidth");
#endif
}

#pragma push_macro("glLinkProgram")
#undef glLinkProgram
void glLinkProgram (GLuint program) {
#pragma pop_macro("glLinkProgram")
	if( enable_logging) {
		log_cpp(" if ( g_GLids[ %d] != -1) {\nglLinkProgram( g_GLids[ %d]);\n}\n", program, program);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glLinkProgram\", __FILE__, __LINE__);\n");
#endif
	}
	program = g_GLids[program];
	if (program==-1) return;
	Original_GL::glLinkProgram(program);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glLinkProgram");
#endif
}

#pragma push_macro("glPixelStorei")
#undef glPixelStorei
void glPixelStorei (GLenum pname, GLint param) {
#pragma pop_macro("glPixelStorei")
	if( enable_logging) {
		log_cpp(" glPixelStorei( %d, %d);\n", pname, param);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glPixelStorei\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glPixelStorei(pname, param);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glPixelStorei");
#endif
}

#pragma push_macro("glPolygonOffset")
#undef glPolygonOffset
void glPolygonOffset (GLfloat factor, GLfloat units) {
#pragma pop_macro("glPolygonOffset")
	if( enable_logging) {
		log_cpp(" glPolygonOffset( %#gf, %#gf);\n", float_sane( &factor ), float_sane( &units ));
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glPolygonOffset\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glPolygonOffset(factor, units);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glPolygonOffset");
#endif
}

#pragma push_macro("glReadPixels")
#undef glReadPixels
void glReadPixels (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid* pixels) {
#pragma pop_macro("glReadPixels")
	if( enable_logging) {
		log_cpp(" glReadPixels( %d, %d, %d, %d, %d, %d, (void*)0x%x);\n", x, y, width, height, format, type, pixels);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glReadPixels\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glReadPixels(x, y, width, height, format, type, pixels);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glReadPixels");
#endif
}

#pragma push_macro("glReleaseShaderCompiler")
#undef glReleaseShaderCompiler
void glReleaseShaderCompiler (void) {
#pragma pop_macro("glReleaseShaderCompiler")
	if( enable_logging) {
		log_cpp(" glReleaseShaderCompiler();\n");
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glReleaseShaderCompiler\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glReleaseShaderCompiler();
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glReleaseShaderCompiler");
#endif
}

#pragma push_macro("glRenderbufferStorage")
#undef glRenderbufferStorage
void glRenderbufferStorage (GLenum target, GLenum internalformat, GLsizei width, GLsizei height) {
#pragma pop_macro("glRenderbufferStorage")
	if( enable_logging) {
		log_cpp(" glRenderbufferStorage( %d, %d, %d, %d);\n", target, internalformat, width, height);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glRenderbufferStorage\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glRenderbufferStorage(target, internalformat, width, height);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glRenderbufferStorage");
#endif
}

#pragma push_macro("glSampleCoverage")
#undef glSampleCoverage
void glSampleCoverage (GLfloat value, GLboolean invert) {
#pragma pop_macro("glSampleCoverage")
	if( enable_logging) {
		log_cpp(" glSampleCoverage( %#gf, %d);\n", float_sane( &value ), invert);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glSampleCoverage\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glSampleCoverage(value, invert);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glSampleCoverage");
#endif
}

#pragma push_macro("glScissor")
#undef glScissor
void glScissor (GLint x, GLint y, GLsizei width, GLsizei height) {
#pragma pop_macro("glScissor")
	if( enable_logging) {
		log_cpp(" glScissor( %d, %d, %d, %d);\n", x, y, width, height);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glScissor\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glScissor(x, y, width, height);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glScissor");
#endif
}

#pragma push_macro("glShaderBinary")
#undef glShaderBinary
void glShaderBinary (GLsizei n, const GLuint* shaders, GLenum binaryformat, const GLvoid* binary, GLsizei length) {
#pragma pop_macro("glShaderBinary")
	if( enable_logging) {
		log_cpp(" glShaderBinary( %d, (void*)0x%x, %d, (void*)0x%x, %d);\n", n, shaders, binaryformat, binary, length);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glShaderBinary\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glShaderBinary(n, shaders, binaryformat, binary, length);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glShaderBinary");
#endif
}

#pragma push_macro("glShaderSource")
#undef glShaderSource
void glShaderSource (GLuint shader, GLsizei count, const GLchar* const* string, const GLint* length) {
#pragma pop_macro("glShaderSource")
	if( enable_logging) {
		std::string concat;
		for(GLsizei c = 0; c < count; ++c)
		{
			concat += std::string(string[c]);
		}
		int i = DataWrite( shader, concat.length() + 1, concat.c_str() );
		log_cpp( "delete dr;\n");
		log_cpp( "dr = new DataReader( %d);\n", i);
		log_cpp( "drd = dr->Data();\n");
		log_cpp( "drs = dr->Size();\n");
		log_cpp(" if ( g_GLids[ %d] != -1) {\nglShaderSource( g_GLids[ %d], 1, &drd, &drs); }\n", shader, shader);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glShaderSource\", __FILE__, __LINE__);\n");
#endif
	}
	shader = g_GLids[shader];
	if (shader==-1) return;
	Original_GL::glShaderSource(shader, count, (const Original_GL::GLchar **)string, length);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glShaderSource");
#endif
}

#pragma push_macro("glStencilFunc")
#undef glStencilFunc
void glStencilFunc (GLenum func, GLint ref, GLuint mask) {
#pragma pop_macro("glStencilFunc")
	if( enable_logging) {
		log_cpp(" glStencilFunc( %d, %d, %d);\n", func, ref, mask);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glStencilFunc\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glStencilFunc(func, ref, mask);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glStencilFunc");
#endif
}

#pragma push_macro("glStencilFuncSeparate")
#undef glStencilFuncSeparate
void glStencilFuncSeparate (GLenum face, GLenum func, GLint ref, GLuint mask) {
#pragma pop_macro("glStencilFuncSeparate")
	if( enable_logging) {
		log_cpp(" glStencilFuncSeparate( %d, %d, %d, %d);\n", face, func, ref, mask);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glStencilFuncSeparate\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glStencilFuncSeparate(face, func, ref, mask);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glStencilFuncSeparate");
#endif
}

#pragma push_macro("glStencilMask")
#undef glStencilMask
void glStencilMask (GLuint mask) {
#pragma pop_macro("glStencilMask")
	if( enable_logging) {
		log_cpp(" glStencilMask( %d);\n", mask);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glStencilMask\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glStencilMask(mask);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glStencilMask");
#endif
}

#pragma push_macro("glStencilMaskSeparate")
#undef glStencilMaskSeparate
void glStencilMaskSeparate (GLenum face, GLuint mask) {
#pragma pop_macro("glStencilMaskSeparate")
	if( enable_logging) {
		log_cpp(" glStencilMaskSeparate( %d, %d);\n", face, mask);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glStencilMaskSeparate\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glStencilMaskSeparate(face, mask);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glStencilMaskSeparate");
#endif
}

#pragma push_macro("glStencilOp")
#undef glStencilOp
void glStencilOp (GLenum fail, GLenum zfail, GLenum zpass) {
#pragma pop_macro("glStencilOp")
	if( enable_logging) {
		log_cpp(" glStencilOp( %d, %d, %d);\n", fail, zfail, zpass);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glStencilOp\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glStencilOp(fail, zfail, zpass);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glStencilOp");
#endif
}

#pragma push_macro("glStencilOpSeparate")
#undef glStencilOpSeparate
void glStencilOpSeparate (GLenum face, GLenum fail, GLenum zfail, GLenum zpass) {
#pragma pop_macro("glStencilOpSeparate")
	if( enable_logging) {
		log_cpp(" glStencilOpSeparate( %d, %d, %d, %d);\n", face, fail, zfail, zpass);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glStencilOpSeparate\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glStencilOpSeparate(face, fail, zfail, zpass);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glStencilOpSeparate");
#endif
}

#pragma push_macro("glTexImage2D")
#undef glTexImage2D
void glTexImage2D (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid* pixels) {
#pragma pop_macro("glTexImage2D")
	if( enable_logging) {
		if( pixels && !current_pbo)
		{
			int i = DataWrite( current_to , getSizeOf(width,height,1,format,type), (const char*)pixels);
			log_cpp( "delete dr;\n");
			log_cpp( "dr = new DataReader( %d);\n", i);
			log_cpp( "glTexImage2D( %d, %d, %d, %d, %d, %d, %d, %d, dr->Data());\n", target, level, internalformat, width, height, border, format, type);
		} else {
			log_cpp( "glTexImage2D( %d, %d, %d, %d, %d, %d, %d, %d, (void*)0x%x);\n", target, level, internalformat, width, height, border, format, type, pixels);
		}
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glTexImage2D\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glTexImage2D(target, level, internalformat, width, height, border, format, type, pixels);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glTexImage2D");
#endif
}

#pragma push_macro("glTexParameterf")
#undef glTexParameterf
void glTexParameterf (GLenum target, GLenum pname, GLfloat param) {
#pragma pop_macro("glTexParameterf")
	if( enable_logging) {
		log_cpp(" glTexParameterf( %d, %d, %#gf);\n", target, pname, float_sane( &param ));
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glTexParameterf\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glTexParameterf(target, pname, param);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glTexParameterf");
#endif
}

#pragma push_macro("glTexParameterfv")
#undef glTexParameterfv
void glTexParameterfv (GLenum target, GLenum pname, const GLfloat* params) {
#pragma pop_macro("glTexParameterfv")
	if( enable_logging) {
		log_cpp(" glTexParameterfv( %d, %d, (void*)0x%x);\n", target, pname, params);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glTexParameterfv\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glTexParameterfv(target, pname, params);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glTexParameterfv");
#endif
}

#pragma push_macro("glTexParameteri")
#undef glTexParameteri
void glTexParameteri (GLenum target, GLenum pname, GLint param) {
#pragma pop_macro("glTexParameteri")
	if( enable_logging) {
		log_cpp(" glTexParameteri( %d, %d, %d);\n", target, pname, param);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glTexParameteri\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glTexParameteri(target, pname, param);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glTexParameteri");
#endif
}

#pragma push_macro("glTexParameteriv")
#undef glTexParameteriv
void glTexParameteriv (GLenum target, GLenum pname, const GLint* params) {
#pragma pop_macro("glTexParameteriv")
	if( enable_logging) {
		log_cpp(" glTexParameteriv( %d, %d, (void*)0x%x);\n", target, pname, params);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glTexParameteriv\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glTexParameteriv(target, pname, params);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glTexParameteriv");
#endif
}

#pragma push_macro("glTexSubImage2D")
#undef glTexSubImage2D
void glTexSubImage2D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid* pixels) {
#pragma pop_macro("glTexSubImage2D")
	if( enable_logging) {
		if( pixels && !current_pbo) {
			int i = DataWrite( current_to , getSizeOf(width,height,1,format,type), (const char*)pixels);
			log_cpp( "delete dr;\n");
			log_cpp( "dr = new DataReader( %d);\n", i);
			log_cpp( "glTexSubImage2D( %d, %d, %d, %d, %d, %d, %d, %d, dr->Data());\n", target, level, xoffset, yoffset, width, height, format, type);
		} else {
			log_cpp( "glTexSubImage2D( %d, %d, %d, %d, %d, %d, %d, %d, (void*)0x%x);\n", target, level, xoffset, yoffset, width, height, format, type, pixels);
		}
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glTexSubImage2D\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glTexSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glTexSubImage2D");
#endif
}

#pragma push_macro("glUniform1f")
#undef glUniform1f
void glUniform1f (GLint location, GLfloat x) {
#pragma pop_macro("glUniform1f")
	if( enable_logging) {
		log_cpp(" if ( g_GLids[ %d] != -1) {\nglUniform1f( g_GLids[ %d], %#gf);\n}\n", location, location, float_sane( &x ));
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glUniform1f\", __FILE__, __LINE__);\n");
#endif
	}
	location = g_GLids[location];
	if (location==-1) return;
	Original_GL::glUniform1f(location, x);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glUniform1f");
#endif
}

#pragma push_macro("glUniform1fv")
#undef glUniform1fv
void glUniform1fv (GLint location, GLsizei count, const GLfloat* v) {
#pragma pop_macro("glUniform1fv")
	if( enable_logging) {
		log_cpp(" if ( g_GLids[ %d] != -1) {\nglUniform1fv( g_GLids[ %d], %d, (void*)0x%x);\n}\n", location, location, count, v);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glUniform1fv\", __FILE__, __LINE__);\n");
#endif
	}
	location = g_GLids[location];
	if (location==-1) return;
	Original_GL::glUniform1fv(location, count, v);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glUniform1fv");
#endif
}

#pragma push_macro("glUniform1i")
#undef glUniform1i
void glUniform1i (GLint location, GLint x) {
#pragma pop_macro("glUniform1i")
	if( enable_logging) {
		log_cpp(" if ( g_GLids[ %d] != -1) {\nglUniform1i( g_GLids[ %d], %d);\n}\n", location, location, x);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glUniform1i\", __FILE__, __LINE__);\n");
#endif
	}
	location = g_GLids[location];
	if (location==-1) return;
	Original_GL::glUniform1i(location, x);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glUniform1i");
#endif
}

#pragma push_macro("glUniform1iv")
#undef glUniform1iv
void glUniform1iv (GLint location, GLsizei count, const GLint* v) {
#pragma pop_macro("glUniform1iv")
	if( enable_logging) {
		log_cpp(" if ( g_GLids[ %d] != -1) {\nglUniform1iv( g_GLids[ %d], %d, (void*)0x%x);\n}\n", location, location, count, v);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glUniform1iv\", __FILE__, __LINE__);\n");
#endif
	}
	location = g_GLids[location];
	if (location==-1) return;
	Original_GL::glUniform1iv(location, count, v);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glUniform1iv");
#endif
}

#pragma push_macro("glUniform2f")
#undef glUniform2f
void glUniform2f (GLint location, GLfloat x, GLfloat y) {
#pragma pop_macro("glUniform2f")
	if( enable_logging) {
		log_cpp(" if ( g_GLids[ %d] != -1) {\nglUniform2f( g_GLids[ %d], %#gf, %#gf);\n}\n", location, location, float_sane( &x ), float_sane( &y ));
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glUniform2f\", __FILE__, __LINE__);\n");
#endif
	}
	location = g_GLids[location];
	if (location==-1) return;
	Original_GL::glUniform2f(location, x, y);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glUniform2f");
#endif
}

#pragma push_macro("glUniform2fv")
#undef glUniform2fv
void glUniform2fv (GLint location, GLsizei count, const GLfloat* v) {
#pragma pop_macro("glUniform2fv")
	if (1 > count ) return;
	if( enable_logging) {
		int float_idx=putFloats( count * 2 , v );
		log_cpp(" if ( g_GLids[ %d] != -1) {\nglUniform2fv( g_GLids[ %d], %d, floats+%d);\n}\n", location, location, count, float_idx);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glUniform2fv\", __FILE__, __LINE__);\n");
#endif
	}
	location = g_GLids[location];
	if (location==-1) return;
	Original_GL::glUniform2fv(location, count, v);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glUniform2fv");
#endif
}

#pragma push_macro("glUniform2i")
#undef glUniform2i
void glUniform2i (GLint location, GLint x, GLint y) {
#pragma pop_macro("glUniform2i")
	if( enable_logging) {
		log_cpp(" if ( g_GLids[ %d] != -1) {\nglUniform2i( g_GLids[ %d], %d, %d);\n}\n", location, location, x, y);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glUniform2i\", __FILE__, __LINE__);\n");
#endif
	}
	location = g_GLids[location];
	if (location==-1) return;
	Original_GL::glUniform2i(location, x, y);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glUniform2i");
#endif
}

#pragma push_macro("glUniform2iv")
#undef glUniform2iv
void glUniform2iv (GLint location, GLsizei count, const GLint* v) {
#pragma pop_macro("glUniform2iv")
	if( enable_logging) {
		log_cpp(" if ( g_GLids[ %d] != -1) {\nglUniform2iv( g_GLids[ %d], %d, (void*)0x%x);\n}\n", location, location, count, v);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glUniform2iv\", __FILE__, __LINE__);\n");
#endif
	}
	location = g_GLids[location];
	if (location==-1) return;
	Original_GL::glUniform2iv(location, count, v);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glUniform2iv");
#endif
}

#pragma push_macro("glUniform3f")
#undef glUniform3f
void glUniform3f (GLint location, GLfloat x, GLfloat y, GLfloat z) {
#pragma pop_macro("glUniform3f")
	if( enable_logging) {
		log_cpp(" if ( g_GLids[ %d] != -1) {\nglUniform3f( g_GLids[ %d], %#gf, %#gf, %#gf);\n}\n", location, location, float_sane( &x ), float_sane( &y ), float_sane( &z ));
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glUniform3f\", __FILE__, __LINE__);\n");
#endif
	}
	location = g_GLids[location];
	if (location==-1) return;
	Original_GL::glUniform3f(location, x, y, z);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glUniform3f");
#endif
}

#pragma push_macro("glUniform3fv")
#undef glUniform3fv
void glUniform3fv (GLint location, GLsizei count, const GLfloat* v) {
#pragma pop_macro("glUniform3fv")
	if (1 > count ) return;
	if( enable_logging) {
		int float_idx=putFloats( count * 3 , v );
		log_cpp(" if ( g_GLids[ %d] != -1) {\nglUniform3fv( g_GLids[ %d], %d, floats+%d);\n}\n", location, location, count, float_idx);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glUniform3fv\", __FILE__, __LINE__);\n");
#endif
	}
	location = g_GLids[location];
	if (location==-1) return;
	Original_GL::glUniform3fv(location, count, v);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glUniform3fv");
#endif
}

#pragma push_macro("glUniform3i")
#undef glUniform3i
void glUniform3i (GLint location, GLint x, GLint y, GLint z) {
#pragma pop_macro("glUniform3i")
	if( enable_logging) {
		log_cpp(" if ( g_GLids[ %d] != -1) {\nglUniform3i( g_GLids[ %d], %d, %d, %d);\n}\n", location, location, x, y, z);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glUniform3i\", __FILE__, __LINE__);\n");
#endif
	}
	location = g_GLids[location];
	if (location==-1) return;
	Original_GL::glUniform3i(location, x, y, z);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glUniform3i");
#endif
}

#pragma push_macro("glUniform3iv")
#undef glUniform3iv
void glUniform3iv (GLint location, GLsizei count, const GLint* v) {
#pragma pop_macro("glUniform3iv")
	if( enable_logging) {
		log_cpp(" if ( g_GLids[ %d] != -1) {\nglUniform3iv( g_GLids[ %d], %d, (void*)0x%x);\n}\n", location, location, count, v);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glUniform3iv\", __FILE__, __LINE__);\n");
#endif
	}
	location = g_GLids[location];
	if (location==-1) return;
	Original_GL::glUniform3iv(location, count, v);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glUniform3iv");
#endif
}

#pragma push_macro("glUniform4f")
#undef glUniform4f
void glUniform4f (GLint location, GLfloat x, GLfloat y, GLfloat z, GLfloat w) {
#pragma pop_macro("glUniform4f")
	if( enable_logging) {
		log_cpp(" if ( g_GLids[ %d] != -1) {\nglUniform4f( g_GLids[ %d], %#gf, %#gf, %#gf, %#gf);\n}\n", location, location, float_sane( &x ), float_sane( &y ), float_sane( &z ), float_sane( &w ));
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glUniform4f\", __FILE__, __LINE__);\n");
#endif
	}
	location = g_GLids[location];
	if (location==-1) return;
	Original_GL::glUniform4f(location, x, y, z, w);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glUniform4f");
#endif
}

#pragma push_macro("glUniform4fv")
#undef glUniform4fv
void glUniform4fv (GLint location, GLsizei count, const GLfloat* v) {
#pragma pop_macro("glUniform4fv")
	if (1 > count ) return;
	if( enable_logging) {
		int float_idx=putFloats( count * 4 , v );
		log_cpp(" if ( g_GLids[ %d] != -1) {\nglUniform4fv( g_GLids[ %d], %d, floats+%d);\n}\n", location, location, count, float_idx);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glUniform4fv\", __FILE__, __LINE__);\n");
#endif
	}
	location = g_GLids[location];
	if (location==-1) return;
	Original_GL::glUniform4fv(location, count, v);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glUniform4fv");
#endif
}

#pragma push_macro("glUniform4i")
#undef glUniform4i
void glUniform4i (GLint location, GLint x, GLint y, GLint z, GLint w) {
#pragma pop_macro("glUniform4i")
	if( enable_logging) {
		log_cpp(" if ( g_GLids[ %d] != -1) {\nglUniform4i( g_GLids[ %d], %d, %d, %d, %d);\n}\n", location, location, x, y, z, w);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glUniform4i\", __FILE__, __LINE__);\n");
#endif
	}
	location = g_GLids[location];
	if (location==-1) return;
	Original_GL::glUniform4i(location, x, y, z, w);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glUniform4i");
#endif
}

#pragma push_macro("glUniform4iv")
#undef glUniform4iv
void glUniform4iv (GLint location, GLsizei count, const GLint* v) {
#pragma pop_macro("glUniform4iv")
	if( enable_logging) {
		log_cpp(" if ( g_GLids[ %d] != -1) {\nglUniform4iv( g_GLids[ %d], %d, (void*)0x%x);\n}\n", location, location, count, v);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glUniform4iv\", __FILE__, __LINE__);\n");
#endif
	}
	location = g_GLids[location];
	if (location==-1) return;
	Original_GL::glUniform4iv(location, count, v);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glUniform4iv");
#endif
}

#pragma push_macro("glUniformMatrix2fv")
#undef glUniformMatrix2fv
void glUniformMatrix2fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) {
#pragma pop_macro("glUniformMatrix2fv")
	if( enable_logging) {
		log_cpp(" if ( g_GLids[ %d] != -1) {\nglUniformMatrix2fv( g_GLids[ %d], %d, %d, (void*)0x%x);\n}\n", location, location, count, transpose, value);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glUniformMatrix2fv\", __FILE__, __LINE__);\n");
#endif
	}
	location = g_GLids[location];
	if (location==-1) return;
	Original_GL::glUniformMatrix2fv(location, count, transpose, value);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glUniformMatrix2fv");
#endif
}

#pragma push_macro("glUniformMatrix3fv")
#undef glUniformMatrix3fv
void glUniformMatrix3fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) {
#pragma pop_macro("glUniformMatrix3fv")
	if( enable_logging) {
		log_cpp(" if ( g_GLids[ %d] != -1) {\nglUniformMatrix3fv( g_GLids[ %d], %d, %d, (void*)0x%x);\n}\n", location, location, count, transpose, value);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glUniformMatrix3fv\", __FILE__, __LINE__);\n");
#endif
	}
	location = g_GLids[location];
	if (location==-1) return;
	Original_GL::glUniformMatrix3fv(location, count, transpose, value);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glUniformMatrix3fv");
#endif
}

#pragma push_macro("glUniformMatrix4fv")
#undef glUniformMatrix4fv
void glUniformMatrix4fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) {
#pragma pop_macro("glUniformMatrix4fv")
	if (1 > count ) return;
	if( enable_logging) {
		int float_idx=putFloats( count * 16 , value );
		log_cpp(" if ( g_GLids[ %d] != -1) {\nglUniformMatrix4fv( g_GLids[ %d], %d, %d, floats+%d);\n}\n", location, location, count, transpose, float_idx);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glUniformMatrix4fv\", __FILE__, __LINE__);\n");
#endif
	}
	location = g_GLids[location];
	if (location==-1) return;
	Original_GL::glUniformMatrix4fv(location, count, transpose, value);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glUniformMatrix4fv");
#endif
}

#pragma push_macro("glUseProgram")
#undef glUseProgram
void glUseProgram (GLuint program) {
#pragma pop_macro("glUseProgram")
	if( enable_logging) {
		log_cpp(" if ( g_GLids[ %d] != -1) {\nglUseProgram( g_GLids[ %d]);\n}\n", program, program);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glUseProgram\", __FILE__, __LINE__);\n");
#endif
	}
	program = g_GLids[program];
	if (program==-1) return;
	Original_GL::glUseProgram(program);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glUseProgram");
#endif
}

#pragma push_macro("glValidateProgram")
#undef glValidateProgram
void glValidateProgram (GLuint program) {
#pragma pop_macro("glValidateProgram")
	program = g_GLids[program];
	if (program==-1) return;
	Original_GL::glValidateProgram(program);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glValidateProgram");
#endif
}

#pragma push_macro("glVertexAttrib1f")
#undef glVertexAttrib1f
void glVertexAttrib1f (GLuint indx, GLfloat x) {
#pragma pop_macro("glVertexAttrib1f")
	if( enable_logging) {
		log_cpp(" glVertexAttrib1f( %d, %#gf);\n", indx, float_sane( &x ));
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glVertexAttrib1f\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glVertexAttrib1f(indx, x);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glVertexAttrib1f");
#endif
}

#pragma push_macro("glVertexAttrib1fv")
#undef glVertexAttrib1fv
void glVertexAttrib1fv (GLuint indx, const GLfloat* values) {
#pragma pop_macro("glVertexAttrib1fv")
	if( enable_logging) {
		log_cpp(" glVertexAttrib1fv( %d, (void*)0x%x);\n", indx, values);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glVertexAttrib1fv\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glVertexAttrib1fv(indx, values);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glVertexAttrib1fv");
#endif
}

#pragma push_macro("glVertexAttrib2f")
#undef glVertexAttrib2f
void glVertexAttrib2f (GLuint indx, GLfloat x, GLfloat y) {
#pragma pop_macro("glVertexAttrib2f")
	if( enable_logging) {
		log_cpp(" glVertexAttrib2f( %d, %#gf, %#gf);\n", indx, float_sane( &x ), float_sane( &y ));
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glVertexAttrib2f\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glVertexAttrib2f(indx, x, y);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glVertexAttrib2f");
#endif
}

#pragma push_macro("glVertexAttrib2fv")
#undef glVertexAttrib2fv
void glVertexAttrib2fv (GLuint indx, const GLfloat* values) {
#pragma pop_macro("glVertexAttrib2fv")
	if( enable_logging) {
		log_cpp(" glVertexAttrib2fv( %d, (void*)0x%x);\n", indx, values);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glVertexAttrib2fv\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glVertexAttrib2fv(indx, values);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glVertexAttrib2fv");
#endif
}

#pragma push_macro("glVertexAttrib3f")
#undef glVertexAttrib3f
void glVertexAttrib3f (GLuint indx, GLfloat x, GLfloat y, GLfloat z) {
#pragma pop_macro("glVertexAttrib3f")
	if( enable_logging) {
		log_cpp(" glVertexAttrib3f( %d, %#gf, %#gf, %#gf);\n", indx, float_sane( &x ), float_sane( &y ), float_sane( &z ));
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glVertexAttrib3f\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glVertexAttrib3f(indx, x, y, z);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glVertexAttrib3f");
#endif
}

#pragma push_macro("glVertexAttrib3fv")
#undef glVertexAttrib3fv
void glVertexAttrib3fv (GLuint indx, const GLfloat* values) {
#pragma pop_macro("glVertexAttrib3fv")
	if( enable_logging) {
		log_cpp(" glVertexAttrib3fv( %d, (void*)0x%x);\n", indx, values);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glVertexAttrib3fv\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glVertexAttrib3fv(indx, values);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glVertexAttrib3fv");
#endif
}

#pragma push_macro("glVertexAttrib4f")
#undef glVertexAttrib4f
void glVertexAttrib4f (GLuint indx, GLfloat x, GLfloat y, GLfloat z, GLfloat w) {
#pragma pop_macro("glVertexAttrib4f")
	if( enable_logging) {
		log_cpp(" glVertexAttrib4f( %d, %#gf, %#gf, %#gf, %#gf);\n", indx, float_sane( &x ), float_sane( &y ), float_sane( &z ), float_sane( &w ));
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glVertexAttrib4f\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glVertexAttrib4f(indx, x, y, z, w);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glVertexAttrib4f");
#endif
}

#pragma push_macro("glVertexAttrib4fv")
#undef glVertexAttrib4fv
void glVertexAttrib4fv (GLuint indx, const GLfloat* values) {
#pragma pop_macro("glVertexAttrib4fv")
	if( enable_logging) {
		log_cpp(" glVertexAttrib4fv( %d, (void*)0x%x);\n", indx, values);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glVertexAttrib4fv\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glVertexAttrib4fv(indx, values);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glVertexAttrib4fv");
#endif
}

#pragma push_macro("glVertexAttribPointer")
#undef glVertexAttribPointer
void glVertexAttribPointer (GLuint indx, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* ptr) {
#pragma pop_macro("glVertexAttribPointer")
	if( enable_logging) {
		log_cpp(" if ( g_GLids[ %d] != -1) {\nglVertexAttribPointer( g_GLids[ %d], %d, %d, %d, %d, (void*)0x%x);\n}\n", indx, indx, size, type, normalized, stride, ptr);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glVertexAttribPointer\", __FILE__, __LINE__);\n");
#endif
	}
	indx = g_GLids[indx];
	if (indx==-1) return;
	Original_GL::glVertexAttribPointer(indx, size, type, normalized, stride, ptr);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glVertexAttribPointer");
#endif
}

#pragma push_macro("glViewport")
#undef glViewport
void glViewport (GLint x, GLint y, GLsizei width, GLsizei height) {
#pragma pop_macro("glViewport")
	if( enable_logging) {
		log_cpp(" glViewport( %d, %d, %d, %d);\n", x, y, width, height);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glViewport\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glViewport(x, y, width, height);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glViewport");
#endif
}

#pragma push_macro("glReadBuffer")
#undef glReadBuffer
void glReadBuffer (GLenum mode) {
#pragma pop_macro("glReadBuffer")
	if( enable_logging) {
		log_cpp(" #ifdef HAVE_GLEW\nglReadBuffer( %d);\n#endif\n", mode);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glReadBuffer\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glReadBuffer(mode);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glReadBuffer");
#endif
}

#pragma push_macro("glDrawRangeElements")
#undef glDrawRangeElements
void glDrawRangeElements (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid* indices) {
#pragma pop_macro("glDrawRangeElements")
	if( enable_logging) {
		log_cpp(" glDrawRangeElements( %d, %d, %d, %d, %d, (void*)0x%x);\n", mode, start, end, count, type, indices);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glDrawRangeElements\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glDrawRangeElements(mode, start, end, count, type, indices);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glDrawRangeElements");
#endif
}

#pragma push_macro("glTexImage3D")
#undef glTexImage3D
void glTexImage3D (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid* pixels) {
#pragma pop_macro("glTexImage3D")
	if( enable_logging) {
		if( pixels && !current_pbo)
		{
			int i = DataWrite( current_to , getSizeOf(width,height,depth,format,type), (const char*)pixels);
			log_cpp( "delete dr;\n");
			log_cpp( "dr = new DataReader( %d);\n", i);
			log_cpp( "glTexImage3D( %d, %d, %d, %d, %d, %d, %d, %d, %d, dr->Data());\n", target, level, internalformat, width, height, depth, border, format, type);
		} else {
			log_cpp( "glTexImage3D( %d, %d, %d, %d, %d, %d, %d, %d, %d, (void*)0x%x);\n", target, level, internalformat, width, height, depth, border, format, type, pixels);
		}
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glTexImage3D\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glTexImage3D(target, level, internalformat, width, height, depth, border, format, type, pixels);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glTexImage3D");
#endif
}

#pragma push_macro("glTexSubImage3D")
#undef glTexSubImage3D
void glTexSubImage3D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid* pixels) {
#pragma pop_macro("glTexSubImage3D")
	if( enable_logging) {
		if( pixels && !current_pbo)
		{
			int i = DataWrite( current_to*(zoffset+1)%32768 , getSizeOf(width,height,depth,format,type), (const char*)pixels);
			log_cpp( "delete dr;\n");
			log_cpp( "dr = new DataReader( %d);\n", i);
			log_cpp( "glTexSubImage3D( %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, dr->Data());\n", target, level, xoffset, yoffset, zoffset, width, height, depth, format, type);
		} else {
			log_cpp( "glTexSubImage3D( %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, (void*)0x%x);\n", target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels);
		}
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glTexSubImage3D\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glTexSubImage3D(target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glTexSubImage3D");
#endif
}

#pragma push_macro("glCopyTexSubImage3D")
#undef glCopyTexSubImage3D
void glCopyTexSubImage3D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height) {
#pragma pop_macro("glCopyTexSubImage3D")
	if( enable_logging) {
		log_cpp(" glCopyTexSubImage3D( %d, %d, %d, %d, %d, %d, %d, %d, %d);\n", target, level, xoffset, yoffset, zoffset, x, y, width, height);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glCopyTexSubImage3D\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glCopyTexSubImage3D(target, level, xoffset, yoffset, zoffset, x, y, width, height);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glCopyTexSubImage3D");
#endif
}

#pragma push_macro("glCompressedTexImage3D")
#undef glCompressedTexImage3D
void glCompressedTexImage3D (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid* data) {
#pragma pop_macro("glCompressedTexImage3D")
	if( enable_logging) {
		if( data && !current_pbo)
		{
			int i = DataWrite( current_to , imageSize, (const char*)data);
			log_cpp( "delete dr;\n");
			log_cpp( "dr = new DataReader( %d);\n", i);
			log_cpp( "glCompressedTexImage3D( %d, %d, %d, %d, %d, %d, %d, dr->Size(), dr->Data());\n", target, level, internalformat, width, height, depth, border);
		} else {
			log_cpp( "glCompressedTexImage3D( %d, %d, %d, %d, %d, %d, %d, %d, (void*)0x%x);\n", target, level, internalformat, width, height, depth, border, imageSize, data);
		}
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glCompressedTexImage3D\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glCompressedTexImage3D(target, level, internalformat, width, height, depth, border, imageSize, data);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glCompressedTexImage3D");
#endif
}

#pragma push_macro("glCompressedTexSubImage3D")
#undef glCompressedTexSubImage3D
void glCompressedTexSubImage3D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid* data) {
#pragma pop_macro("glCompressedTexSubImage3D")
	if( enable_logging) {
		if( data && !current_pbo)
		{
			int i = DataWrite( current_to*(zoffset+1)%32768 , imageSize, (const char*)data);
			log_cpp( "delete dr;\n");
			log_cpp( "dr = new DataReader( %d);\n", i);
			log_cpp( "glCompressedTexSubImage3D( %d, %d, %d, %d, %d, %d, %d, %d, %d, dr->Size(), dr->Data());\n", target, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, data);
		} else {
			log_cpp( "glCompressedTexSubImage3D( %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, (void*)0x%x);\n", target, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, data);
		}
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glCompressedTexSubImage3D\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glCompressedTexSubImage3D(target, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, data);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glCompressedTexSubImage3D");
#endif
}

#pragma push_macro("glGenQueries")
#undef glGenQueries
void glGenQueries (GLsizei n, GLuint* ids) {
#pragma pop_macro("glGenQueries")
	if( enable_logging) {
		log_cpp(" for( size_t i=0; i<%d; i++)\n{\nGLuint tmp;\nglGenQueries( 1, &tmp);\ng_GLids.push_back( tmp);\n}\n ", n);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glGenQueries\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glGenQueries(n, ids);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glGenQueries");
#endif
	for( GLsizei i=0; i<n; i++)
	{
		g_GLids.push_back(ids[i]); 
		ids[i] = g_GLids.size() - 1; 
	}
}

#pragma push_macro("glDeleteQueries")
#undef glDeleteQueries
void glDeleteQueries (GLsizei n, const GLuint* ids) {
#pragma pop_macro("glDeleteQueries")
	//Original_GL::glDeleteQueries(n, ids);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glDeleteQueries");
#endif
}

#pragma push_macro("glIsQuery")
#undef glIsQuery
GLboolean glIsQuery (GLuint id) {
#pragma pop_macro("glIsQuery")
	id = g_GLids[id];
	if (id==-1) return false;
	GLboolean retval = (GLboolean)Original_GL::glIsQuery(id);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glIsQuery");
#endif
	return retval;
}

#pragma push_macro("glBeginQuery")
#undef glBeginQuery
void glBeginQuery (GLenum target, GLuint id) {
#pragma pop_macro("glBeginQuery")
	id = g_GLids[id];
	if (id==-1) return;
	Original_GL::glBeginQuery(target, id);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glBeginQuery");
#endif
}

#pragma push_macro("glEndQuery")
#undef glEndQuery
void glEndQuery (GLenum target) {
#pragma pop_macro("glEndQuery")
	Original_GL::glEndQuery(target);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glEndQuery");
#endif
}

#pragma push_macro("glGetQueryiv")
#undef glGetQueryiv
void glGetQueryiv (GLenum target, GLenum pname, GLint* params) {
#pragma pop_macro("glGetQueryiv")
	Original_GL::glGetQueryiv(target, pname, params);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glGetQueryiv");
#endif
}

#pragma push_macro("glGetQueryObjectuiv")
#undef glGetQueryObjectuiv
void glGetQueryObjectuiv (GLuint id, GLenum pname, GLuint* params) {
#pragma pop_macro("glGetQueryObjectuiv")
	id = g_GLids[id];
	if (id==-1) return;
	Original_GL::glGetQueryObjectuiv(id, pname, params);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glGetQueryObjectuiv");
#endif
}

#pragma push_macro("glUnmapBuffer")
#undef glUnmapBuffer
GLboolean glUnmapBuffer (GLenum target) {
#pragma pop_macro("glUnmapBuffer")
	if( enable_logging) {
		log_cpp(" glUnmapBuffer( %d);\n", target);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glUnmapBuffer\", __FILE__, __LINE__);\n");
#endif
	}
	GLboolean retval = (GLboolean)Original_GL::glUnmapBuffer(target);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glUnmapBuffer");
#endif
	return retval;
}

#pragma push_macro("glGetBufferPointerv")
#undef glGetBufferPointerv
void glGetBufferPointerv (GLenum target, GLenum pname, GLvoid** params) {
#pragma pop_macro("glGetBufferPointerv")
	if( enable_logging) {
		log_cpp(" glGetBufferPointerv( %d, %d, (void*)0x%x);\n", target, pname, params);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glGetBufferPointerv\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glGetBufferPointerv(target, pname, params);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glGetBufferPointerv");
#endif
}

#pragma push_macro("glDrawBuffers")
#undef glDrawBuffers
void glDrawBuffers (GLsizei n, const GLenum* bufs) {
#pragma pop_macro("glDrawBuffers")
	if (1 > n ) return;
	if( enable_logging) {
		int enum_idx=putEnums( n , bufs );
		log_cpp(" glDrawBuffers( %d, enums+%d);\n", n, enum_idx);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glDrawBuffers\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glDrawBuffers(n, bufs);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glDrawBuffers");
#endif
}

#pragma push_macro("glUniformMatrix2x3fv")
#undef glUniformMatrix2x3fv
void glUniformMatrix2x3fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) {
#pragma pop_macro("glUniformMatrix2x3fv")
	if( enable_logging) {
		log_cpp(" if ( g_GLids[ %d] != -1) {\nglUniformMatrix2x3fv( g_GLids[ %d], %d, %d, (void*)0x%x);\n}\n", location, location, count, transpose, value);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glUniformMatrix2x3fv\", __FILE__, __LINE__);\n");
#endif
	}
	location = g_GLids[location];
	if (location==-1) return;
	Original_GL::glUniformMatrix2x3fv(location, count, transpose, value);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glUniformMatrix2x3fv");
#endif
}

#pragma push_macro("glUniformMatrix3x2fv")
#undef glUniformMatrix3x2fv
void glUniformMatrix3x2fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) {
#pragma pop_macro("glUniformMatrix3x2fv")
	if( enable_logging) {
		log_cpp(" if ( g_GLids[ %d] != -1) {\nglUniformMatrix3x2fv( g_GLids[ %d], %d, %d, (void*)0x%x);\n}\n", location, location, count, transpose, value);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glUniformMatrix3x2fv\", __FILE__, __LINE__);\n");
#endif
	}
	location = g_GLids[location];
	if (location==-1) return;
	Original_GL::glUniformMatrix3x2fv(location, count, transpose, value);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glUniformMatrix3x2fv");
#endif
}

#pragma push_macro("glUniformMatrix2x4fv")
#undef glUniformMatrix2x4fv
void glUniformMatrix2x4fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) {
#pragma pop_macro("glUniformMatrix2x4fv")
	if( enable_logging) {
		log_cpp(" if ( g_GLids[ %d] != -1) {\nglUniformMatrix2x4fv( g_GLids[ %d], %d, %d, (void*)0x%x);\n}\n", location, location, count, transpose, value);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glUniformMatrix2x4fv\", __FILE__, __LINE__);\n");
#endif
	}
	location = g_GLids[location];
	if (location==-1) return;
	Original_GL::glUniformMatrix2x4fv(location, count, transpose, value);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glUniformMatrix2x4fv");
#endif
}

#pragma push_macro("glUniformMatrix4x2fv")
#undef glUniformMatrix4x2fv
void glUniformMatrix4x2fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) {
#pragma pop_macro("glUniformMatrix4x2fv")
	if( enable_logging) {
		log_cpp(" if ( g_GLids[ %d] != -1) {\nglUniformMatrix4x2fv( g_GLids[ %d], %d, %d, (void*)0x%x);\n}\n", location, location, count, transpose, value);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glUniformMatrix4x2fv\", __FILE__, __LINE__);\n");
#endif
	}
	location = g_GLids[location];
	if (location==-1) return;
	Original_GL::glUniformMatrix4x2fv(location, count, transpose, value);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glUniformMatrix4x2fv");
#endif
}

#pragma push_macro("glUniformMatrix3x4fv")
#undef glUniformMatrix3x4fv
void glUniformMatrix3x4fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) {
#pragma pop_macro("glUniformMatrix3x4fv")
	if( enable_logging) {
		log_cpp(" if ( g_GLids[ %d] != -1) {\nglUniformMatrix3x4fv( g_GLids[ %d], %d, %d, (void*)0x%x);\n}\n", location, location, count, transpose, value);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glUniformMatrix3x4fv\", __FILE__, __LINE__);\n");
#endif
	}
	location = g_GLids[location];
	if (location==-1) return;
	Original_GL::glUniformMatrix3x4fv(location, count, transpose, value);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glUniformMatrix3x4fv");
#endif
}

#pragma push_macro("glUniformMatrix4x3fv")
#undef glUniformMatrix4x3fv
void glUniformMatrix4x3fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) {
#pragma pop_macro("glUniformMatrix4x3fv")
	if( enable_logging) {
		log_cpp(" if ( g_GLids[ %d] != -1) {\nglUniformMatrix4x3fv( g_GLids[ %d], %d, %d, (void*)0x%x);\n}\n", location, location, count, transpose, value);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glUniformMatrix4x3fv\", __FILE__, __LINE__);\n");
#endif
	}
	location = g_GLids[location];
	if (location==-1) return;
	Original_GL::glUniformMatrix4x3fv(location, count, transpose, value);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glUniformMatrix4x3fv");
#endif
}

#pragma push_macro("glBlitFramebuffer")
#undef glBlitFramebuffer
void glBlitFramebuffer (GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter) {
#pragma pop_macro("glBlitFramebuffer")
	if( enable_logging) {
		log_cpp(" glBlitFramebuffer( %d, %d, %d, %d, %d, %d, %d, %d, %d, %d);\n", srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glBlitFramebuffer\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glBlitFramebuffer(srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glBlitFramebuffer");
#endif
}

#pragma push_macro("glRenderbufferStorageMultisample")
#undef glRenderbufferStorageMultisample
void glRenderbufferStorageMultisample (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height) {
#pragma pop_macro("glRenderbufferStorageMultisample")
	if( enable_logging) {
		log_cpp(" glRenderbufferStorageMultisample( %d, %d, %d, %d, %d);\n", target, samples, internalformat, width, height);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glRenderbufferStorageMultisample\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glRenderbufferStorageMultisample(target, samples, internalformat, width, height);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glRenderbufferStorageMultisample");
#endif
}

#pragma push_macro("glFramebufferTextureLayer")
#undef glFramebufferTextureLayer
void glFramebufferTextureLayer (GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer) {
#pragma pop_macro("glFramebufferTextureLayer")
	if( enable_logging) {
		log_cpp(" if ( g_GLids[ %d] != -1) {\nglFramebufferTextureLayer( %d, %d, g_GLids[ %d], %d, %d);\n}\n", texture, target, attachment, texture, level, layer);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glFramebufferTextureLayer\", __FILE__, __LINE__);\n");
#endif
	}
	texture = g_GLids[texture];
	if (texture==-1) return;
	Original_GL::glFramebufferTextureLayer(target, attachment, texture, level, layer);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glFramebufferTextureLayer");
#endif
}

#pragma push_macro("glMapBufferRange")
#undef glMapBufferRange
GLvoid* glMapBufferRange (GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access) {
#pragma pop_macro("glMapBufferRange")
	if( enable_logging) {
		log_cpp(" glMapBufferRange( %d, %d, %d, %d);\n", target, offset, length, access);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glMapBufferRange\", __FILE__, __LINE__);\n");
#endif
	}
	GLvoid* retval = (GLvoid*)Original_GL::glMapBufferRange(target, offset, length, access);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glMapBufferRange");
#endif
	return retval;
}

#pragma push_macro("glFlushMappedBufferRange")
#undef glFlushMappedBufferRange
void glFlushMappedBufferRange (GLenum target, GLintptr offset, GLsizeiptr length) {
#pragma pop_macro("glFlushMappedBufferRange")
	if( enable_logging) {
		log_cpp(" glFlushMappedBufferRange( %d, %d, %d);\n", target, offset, length);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glFlushMappedBufferRange\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glFlushMappedBufferRange(target, offset, length);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glFlushMappedBufferRange");
#endif
}

#pragma push_macro("glBindVertexArray")
#undef glBindVertexArray
void glBindVertexArray (GLuint array) {
#pragma pop_macro("glBindVertexArray")
	if( enable_logging) {
		log_cpp(" if ( g_GLids[ %d] != -1) {\nglBindVertexArray( g_GLids[ %d]);\n}\n", array, array);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glBindVertexArray\", __FILE__, __LINE__);\n");
#endif
	}
	array = g_GLids[array];
	if (array==-1) return;
	Original_GL::glBindVertexArray(array);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glBindVertexArray");
#endif
}

#pragma push_macro("glDeleteVertexArrays")
#undef glDeleteVertexArrays
void glDeleteVertexArrays (GLsizei n, const GLuint* arrays) {
#pragma pop_macro("glDeleteVertexArrays")
	//Original_GL::glDeleteVertexArrays(n, arrays);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glDeleteVertexArrays");
#endif
}

#pragma push_macro("glGenVertexArrays")
#undef glGenVertexArrays
void glGenVertexArrays (GLsizei n, GLuint* arrays) {
#pragma pop_macro("glGenVertexArrays")
	if( enable_logging) {
		log_cpp(" for( size_t i=0; i<%d; i++)\n{\nGLuint tmp;\nglGenVertexArrays( 1, &tmp);\ng_GLids.push_back( tmp);\n}\n ", n);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glGenVertexArrays\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glGenVertexArrays(n, arrays);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glGenVertexArrays");
#endif
	for( GLsizei i=0; i<n; i++)
	{
		g_GLids.push_back(arrays[i]); 
		arrays[i] = g_GLids.size() - 1; 
	}
}

#pragma push_macro("glIsVertexArray")
#undef glIsVertexArray
GLboolean glIsVertexArray (GLuint array) {
#pragma pop_macro("glIsVertexArray")
	array = g_GLids[array];
	if (array==-1) return false;
	GLboolean retval = (GLboolean)Original_GL::glIsVertexArray(array);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glIsVertexArray");
#endif
	return retval;
}

#pragma push_macro("glGetIntegeri_v")
#undef glGetIntegeri_v
void glGetIntegeri_v (GLenum target, GLuint index, GLint* data) {
#pragma pop_macro("glGetIntegeri_v")
	Original_GL::glGetIntegeri_v(target, index, data);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glGetIntegeri_v");
#endif
}

#pragma push_macro("glBeginTransformFeedback")
#undef glBeginTransformFeedback
void glBeginTransformFeedback (GLenum primitiveMode) {
#pragma pop_macro("glBeginTransformFeedback")
	if( enable_logging) {
		log_cpp(" glBeginTransformFeedback( %d);\n", primitiveMode);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glBeginTransformFeedback\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glBeginTransformFeedback(primitiveMode);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glBeginTransformFeedback");
#endif
}

#pragma push_macro("glEndTransformFeedback")
#undef glEndTransformFeedback
void glEndTransformFeedback (void) {
#pragma pop_macro("glEndTransformFeedback")
	if( enable_logging) {
		log_cpp(" glEndTransformFeedback();\n");
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glEndTransformFeedback\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glEndTransformFeedback();
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glEndTransformFeedback");
#endif
}

#pragma push_macro("glBindBufferRange")
#undef glBindBufferRange
void glBindBufferRange (GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size) {
#pragma pop_macro("glBindBufferRange")
	if( enable_logging) {
		log_cpp(" if ( g_GLids[ %d] != -1) {\nglBindBufferRange( %d, %d, g_GLids[ %d], %d, %d);\n}\n", buffer, target, index, buffer, offset, size);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glBindBufferRange\", __FILE__, __LINE__);\n");
#endif
	}
	buffer = g_GLids[buffer];
	if (buffer==-1) return;
	Original_GL::glBindBufferRange(target, index, buffer, offset, size);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glBindBufferRange");
#endif
}

#pragma push_macro("glBindBufferBase")
#undef glBindBufferBase
void glBindBufferBase (GLenum target, GLuint index, GLuint buffer) {
#pragma pop_macro("glBindBufferBase")
	if( enable_logging) {
		log_cpp(" if ( g_GLids[ %d] != -1) {\nglBindBufferBase( %d, %d, g_GLids[ %d]);\n}\n", buffer, target, index, buffer);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glBindBufferBase\", __FILE__, __LINE__);\n");
#endif
	}
	buffer = g_GLids[buffer];
	if (buffer==-1) return;
	Original_GL::glBindBufferBase(target, index, buffer);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glBindBufferBase");
#endif
}

#pragma push_macro("glTransformFeedbackVaryings")
#undef glTransformFeedbackVaryings
void glTransformFeedbackVaryings (GLuint program, GLsizei count, const GLchar* const* varyings, GLenum bufferMode) {
#pragma pop_macro("glTransformFeedbackVaryings")
	if( enable_logging) {
		log_cpp(" if ( g_GLids[ %d] != -1) {\nglTransformFeedbackVaryings( g_GLids[ %d], %d, (void*)0x%x, %d);\n}\n", program, program, count, varyings, bufferMode);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glTransformFeedbackVaryings\", __FILE__, __LINE__);\n");
#endif
	}
	program = g_GLids[program];
	if (program==-1) return;
	Original_GL::glTransformFeedbackVaryings(program, count, (const Original_GL::GLchar **)varyings, bufferMode);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glTransformFeedbackVaryings");
#endif
}

#pragma push_macro("glGetTransformFeedbackVarying")
#undef glGetTransformFeedbackVarying
void glGetTransformFeedbackVarying (GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLsizei* size, GLenum* type, GLchar* name) {
#pragma pop_macro("glGetTransformFeedbackVarying")
	if( enable_logging) {
		log_cpp(" if ( g_GLids[ %d] != -1) {\nglGetTransformFeedbackVarying( g_GLids[ %d], %d, %d, (void*)0x%x, (void*)0x%x, (void*)0x%x, \"%s\");\n}\n", program, program, index, bufSize, length, size, type, name);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glGetTransformFeedbackVarying\", __FILE__, __LINE__);\n");
#endif
	}
	program = g_GLids[program];
	if (program==-1) return;
	Original_GL::glGetTransformFeedbackVarying(program, index, bufSize, length, size, type, name);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glGetTransformFeedbackVarying");
#endif
}

#pragma push_macro("glVertexAttribIPointer")
#undef glVertexAttribIPointer
void glVertexAttribIPointer (GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid* pointer) {
#pragma pop_macro("glVertexAttribIPointer")
	if( enable_logging) {
		log_cpp(" glVertexAttribIPointer( %d, %d, %d, %d, (void*)0x%x);\n", index, size, type, stride, pointer);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glVertexAttribIPointer\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glVertexAttribIPointer(index, size, type, stride, pointer);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glVertexAttribIPointer");
#endif
}

#pragma push_macro("glGetVertexAttribIiv")
#undef glGetVertexAttribIiv
void glGetVertexAttribIiv (GLuint index, GLenum pname, GLint* params) {
#pragma pop_macro("glGetVertexAttribIiv")
	if( enable_logging) {
		log_cpp(" glGetVertexAttribIiv( %d, %d, (void*)0x%x);\n", index, pname, params);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glGetVertexAttribIiv\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glGetVertexAttribIiv(index, pname, params);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glGetVertexAttribIiv");
#endif
}

#pragma push_macro("glGetVertexAttribIuiv")
#undef glGetVertexAttribIuiv
void glGetVertexAttribIuiv (GLuint index, GLenum pname, GLuint* params) {
#pragma pop_macro("glGetVertexAttribIuiv")
	Original_GL::glGetVertexAttribIuiv(index, pname, params);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glGetVertexAttribIuiv");
#endif
}

#pragma push_macro("glVertexAttribI4i")
#undef glVertexAttribI4i
void glVertexAttribI4i (GLuint index, GLint x, GLint y, GLint z, GLint w) {
#pragma pop_macro("glVertexAttribI4i")
	if( enable_logging) {
		log_cpp(" glVertexAttribI4i( %d, %d, %d, %d, %d);\n", index, x, y, z, w);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glVertexAttribI4i\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glVertexAttribI4i(index, x, y, z, w);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glVertexAttribI4i");
#endif
}

#pragma push_macro("glVertexAttribI4ui")
#undef glVertexAttribI4ui
void glVertexAttribI4ui (GLuint index, GLuint x, GLuint y, GLuint z, GLuint w) {
#pragma pop_macro("glVertexAttribI4ui")
	if( enable_logging) {
		log_cpp(" glVertexAttribI4ui( %d, %d, %d, %d, %d);\n", index, x, y, z, w);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glVertexAttribI4ui\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glVertexAttribI4ui(index, x, y, z, w);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glVertexAttribI4ui");
#endif
}

#pragma push_macro("glVertexAttribI4iv")
#undef glVertexAttribI4iv
void glVertexAttribI4iv (GLuint index, const GLint* v) {
#pragma pop_macro("glVertexAttribI4iv")
	if( enable_logging) {
		log_cpp(" glVertexAttribI4iv( %d, (void*)0x%x);\n", index, v);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glVertexAttribI4iv\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glVertexAttribI4iv(index, v);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glVertexAttribI4iv");
#endif
}

#pragma push_macro("glVertexAttribI4uiv")
#undef glVertexAttribI4uiv
void glVertexAttribI4uiv (GLuint index, const GLuint* v) {
#pragma pop_macro("glVertexAttribI4uiv")
	if( enable_logging) {
		log_cpp(" glVertexAttribI4uiv( %d, (void*)0x%x);\n", index, v);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glVertexAttribI4uiv\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glVertexAttribI4uiv(index, v);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glVertexAttribI4uiv");
#endif
}

#pragma push_macro("glGetUniformuiv")
#undef glGetUniformuiv
void glGetUniformuiv (GLuint program, GLint location, GLuint* params) {
#pragma pop_macro("glGetUniformuiv")
	if( enable_logging) {
		log_cpp(" glGetUniformuiv( %d, %d, (void*)0x%x);\n", program, location, params);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glGetUniformuiv\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glGetUniformuiv(program, location, params);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glGetUniformuiv");
#endif
}

#pragma push_macro("glGetFragDataLocation")
#undef glGetFragDataLocation
GLint glGetFragDataLocation (GLuint program, const GLchar *name) {
#pragma pop_macro("glGetFragDataLocation")
	if( enable_logging) {
		log_cpp(" if ( g_GLids[ %d] != -1) {\nglGetFragDataLocation( g_GLids[ %d], (void*)0x%x);\n}\n", program, program, name);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glGetFragDataLocation\", __FILE__, __LINE__);\n");
#endif
	}
	program = g_GLids[program];
	if (program==-1) return -1;
	GLint retval = (GLint)Original_GL::glGetFragDataLocation(program, name);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glGetFragDataLocation");
#endif
	return retval;
}

#pragma push_macro("glUniform1ui")
#undef glUniform1ui
void glUniform1ui (GLint location, GLuint v0) {
#pragma pop_macro("glUniform1ui")
	if( enable_logging) {
		log_cpp(" if ( g_GLids[ %d] != -1) {\nglUniform1ui( g_GLids[ %d], %d);\n}\n", location, location, v0);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glUniform1ui\", __FILE__, __LINE__);\n");
#endif
	}
	location = g_GLids[location];
	if (location==-1) return;
	Original_GL::glUniform1ui(location, v0);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glUniform1ui");
#endif
}

#pragma push_macro("glUniform2ui")
#undef glUniform2ui
void glUniform2ui (GLint location, GLuint v0, GLuint v1) {
#pragma pop_macro("glUniform2ui")
	if( enable_logging) {
		log_cpp(" if ( g_GLids[ %d] != -1) {\nglUniform2ui( g_GLids[ %d], %d, %d);\n}\n", location, location, v0, v1);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glUniform2ui\", __FILE__, __LINE__);\n");
#endif
	}
	location = g_GLids[location];
	if (location==-1) return;
	Original_GL::glUniform2ui(location, v0, v1);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glUniform2ui");
#endif
}

#pragma push_macro("glUniform3ui")
#undef glUniform3ui
void glUniform3ui (GLint location, GLuint v0, GLuint v1, GLuint v2) {
#pragma pop_macro("glUniform3ui")
	if( enable_logging) {
		log_cpp(" if ( g_GLids[ %d] != -1) {\nglUniform3ui( g_GLids[ %d], %d, %d, %d);\n}\n", location, location, v0, v1, v2);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glUniform3ui\", __FILE__, __LINE__);\n");
#endif
	}
	location = g_GLids[location];
	if (location==-1) return;
	Original_GL::glUniform3ui(location, v0, v1, v2);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glUniform3ui");
#endif
}

#pragma push_macro("glUniform4ui")
#undef glUniform4ui
void glUniform4ui (GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3) {
#pragma pop_macro("glUniform4ui")
	if( enable_logging) {
		log_cpp(" if ( g_GLids[ %d] != -1) {\nglUniform4ui( g_GLids[ %d], %d, %d, %d, %d);\n}\n", location, location, v0, v1, v2, v3);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glUniform4ui\", __FILE__, __LINE__);\n");
#endif
	}
	location = g_GLids[location];
	if (location==-1) return;
	Original_GL::glUniform4ui(location, v0, v1, v2, v3);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glUniform4ui");
#endif
}

#pragma push_macro("glUniform1uiv")
#undef glUniform1uiv
void glUniform1uiv (GLint location, GLsizei count, const GLuint* value) {
#pragma pop_macro("glUniform1uiv")
	if( enable_logging) {
		log_cpp(" if ( g_GLids[ %d] != -1) {\nglUniform1uiv( g_GLids[ %d], %d, (void*)0x%x);\n}\n", location, location, count, value);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glUniform1uiv\", __FILE__, __LINE__);\n");
#endif
	}
	location = g_GLids[location];
	if (location==-1) return;
	Original_GL::glUniform1uiv(location, count, value);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glUniform1uiv");
#endif
}

#pragma push_macro("glUniform2uiv")
#undef glUniform2uiv
void glUniform2uiv (GLint location, GLsizei count, const GLuint* value) {
#pragma pop_macro("glUniform2uiv")
	if( enable_logging) {
		log_cpp(" if ( g_GLids[ %d] != -1) {\nglUniform2uiv( g_GLids[ %d], %d, (void*)0x%x);\n}\n", location, location, count, value);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glUniform2uiv\", __FILE__, __LINE__);\n");
#endif
	}
	location = g_GLids[location];
	if (location==-1) return;
	Original_GL::glUniform2uiv(location, count, value);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glUniform2uiv");
#endif
}

#pragma push_macro("glUniform3uiv")
#undef glUniform3uiv
void glUniform3uiv (GLint location, GLsizei count, const GLuint* value) {
#pragma pop_macro("glUniform3uiv")
	if( enable_logging) {
		log_cpp(" if ( g_GLids[ %d] != -1) {\nglUniform3uiv( g_GLids[ %d], %d, (void*)0x%x);\n}\n", location, location, count, value);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glUniform3uiv\", __FILE__, __LINE__);\n");
#endif
	}
	location = g_GLids[location];
	if (location==-1) return;
	Original_GL::glUniform3uiv(location, count, value);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glUniform3uiv");
#endif
}

#pragma push_macro("glUniform4uiv")
#undef glUniform4uiv
void glUniform4uiv (GLint location, GLsizei count, const GLuint* value) {
#pragma pop_macro("glUniform4uiv")
	if( enable_logging) {
		log_cpp(" if ( g_GLids[ %d] != -1) {\nglUniform4uiv( g_GLids[ %d], %d, (void*)0x%x);\n}\n", location, location, count, value);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glUniform4uiv\", __FILE__, __LINE__);\n");
#endif
	}
	location = g_GLids[location];
	if (location==-1) return;
	Original_GL::glUniform4uiv(location, count, value);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glUniform4uiv");
#endif
}

#pragma push_macro("glClearBufferiv")
#undef glClearBufferiv
void glClearBufferiv (GLenum buffer, GLint drawbuffer, const GLint* value) {
#pragma pop_macro("glClearBufferiv")
	if( enable_logging) {
		log_cpp(" glClearBufferiv( %d, %d, (void*)0x%x);\n", buffer, drawbuffer, value);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glClearBufferiv\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glClearBufferiv(buffer, drawbuffer, value);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glClearBufferiv");
#endif
}

#pragma push_macro("glClearBufferuiv")
#undef glClearBufferuiv
void glClearBufferuiv (GLenum buffer, GLint drawbuffer, const GLuint* value) {
#pragma pop_macro("glClearBufferuiv")
	if( enable_logging) {
		log_cpp(" glClearBufferuiv( %d, %d, (void*)0x%x);\n", buffer, drawbuffer, value);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glClearBufferuiv\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glClearBufferuiv(buffer, drawbuffer, value);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glClearBufferuiv");
#endif
}

#pragma push_macro("glClearBufferfv")
#undef glClearBufferfv
void glClearBufferfv (GLenum buffer, GLint drawbuffer, const GLfloat* value) {
#pragma pop_macro("glClearBufferfv")
	if( enable_logging) {
		log_cpp(" glClearBufferfv( %d, %d, (void*)0x%x);\n", buffer, drawbuffer, value);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glClearBufferfv\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glClearBufferfv(buffer, drawbuffer, value);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glClearBufferfv");
#endif
}

#pragma push_macro("glClearBufferfi")
#undef glClearBufferfi
void glClearBufferfi (GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil) {
#pragma pop_macro("glClearBufferfi")
	if( enable_logging) {
		log_cpp(" glClearBufferfi( %d, %d, %#gf, %d);\n", buffer, drawbuffer, float_sane( &depth ), stencil);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glClearBufferfi\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glClearBufferfi(buffer, drawbuffer, depth, stencil);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glClearBufferfi");
#endif
}

#pragma push_macro("glGetStringi")
#undef glGetStringi
const GLubyte* glGetStringi (GLenum name, GLuint index) {
#pragma pop_macro("glGetStringi")
	GLubyte* retval = (GLubyte*)Original_GL::glGetStringi(name, index);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glGetStringi");
#endif
	return retval;
}

#pragma push_macro("glCopyBufferSubData")
#undef glCopyBufferSubData
void glCopyBufferSubData (GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size) {
#pragma pop_macro("glCopyBufferSubData")
	if( enable_logging) {
		log_cpp(" glCopyBufferSubData( %d, %d, %d, %d, %d);\n", readTarget, writeTarget, readOffset, writeOffset, size);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glCopyBufferSubData\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glCopyBufferSubData(readTarget, writeTarget, readOffset, writeOffset, size);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glCopyBufferSubData");
#endif
}

#pragma push_macro("glGetUniformIndices")
#undef glGetUniformIndices
void glGetUniformIndices (GLuint program, GLsizei uniformCount, const GLchar* const* uniformNames, GLuint* uniformIndices) {
#pragma pop_macro("glGetUniformIndices")
	program = g_GLids[program];
	if (program==-1) return;
	Original_GL::glGetUniformIndices(program, uniformCount, (const Original_GL::GLchar **)uniformNames, uniformIndices);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glGetUniformIndices");
#endif
}

#pragma push_macro("glGetActiveUniformsiv")
#undef glGetActiveUniformsiv
void glGetActiveUniformsiv (GLuint program, GLsizei uniformCount, const GLuint* uniformIndices, GLenum pname, GLint* params) {
#pragma pop_macro("glGetActiveUniformsiv")
	program = g_GLids[program];
	if (program==-1) return;
	Original_GL::glGetActiveUniformsiv(program, uniformCount, uniformIndices, pname, params);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glGetActiveUniformsiv");
#endif
}

#pragma push_macro("glGetUniformBlockIndex")
#undef glGetUniformBlockIndex
GLuint glGetUniformBlockIndex (GLuint program, const GLchar* uniformBlockName) {
#pragma pop_macro("glGetUniformBlockIndex")
	if( enable_logging) {
		log_cpp(" if ( g_GLids[ %d] != -1) {\nglGetUniformBlockIndex( g_GLids[ %d], \"%s\");\n}\n", program, program, uniformBlockName);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glGetUniformBlockIndex\", __FILE__, __LINE__);\n");
#endif
	}
	program = g_GLids[program];
	if (program==-1) return -1;
	GLuint retval = (GLuint)Original_GL::glGetUniformBlockIndex(program, uniformBlockName);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glGetUniformBlockIndex");
#endif
	return retval;
}

#pragma push_macro("glGetActiveUniformBlockiv")
#undef glGetActiveUniformBlockiv
void glGetActiveUniformBlockiv (GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint* params) {
#pragma pop_macro("glGetActiveUniformBlockiv")
	if( enable_logging) {
		log_cpp(" if ( g_GLids[ %d] != -1) {\nglGetActiveUniformBlockiv( g_GLids[ %d], %d, %d, (void*)0x%x);\n}\n", program, program, uniformBlockIndex, pname, params);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glGetActiveUniformBlockiv\", __FILE__, __LINE__);\n");
#endif
	}
	program = g_GLids[program];
	if (program==-1) return;
	Original_GL::glGetActiveUniformBlockiv(program, uniformBlockIndex, pname, params);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glGetActiveUniformBlockiv");
#endif
}

#pragma push_macro("glGetActiveUniformBlockName")
#undef glGetActiveUniformBlockName
void glGetActiveUniformBlockName (GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei* length, GLchar* uniformBlockName) {
#pragma pop_macro("glGetActiveUniformBlockName")
	if( enable_logging) {
		log_cpp(" if ( g_GLids[ %d] != -1) {\nglGetActiveUniformBlockName( g_GLids[ %d], %d, %d, (void*)0x%x, \"%s\");\n}\n", program, program, uniformBlockIndex, bufSize, length, uniformBlockName);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glGetActiveUniformBlockName\", __FILE__, __LINE__);\n");
#endif
	}
	program = g_GLids[program];
	if (program==-1) return;
	Original_GL::glGetActiveUniformBlockName(program, uniformBlockIndex, bufSize, length, uniformBlockName);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glGetActiveUniformBlockName");
#endif
}

#pragma push_macro("glUniformBlockBinding")
#undef glUniformBlockBinding
void glUniformBlockBinding (GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding) {
#pragma pop_macro("glUniformBlockBinding")
	if( enable_logging) {
		log_cpp(" if ( g_GLids[ %d] != -1) {\nglUniformBlockBinding( g_GLids[ %d], %d, %d);\n}\n", program, program, uniformBlockIndex, uniformBlockBinding);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glUniformBlockBinding\", __FILE__, __LINE__);\n");
#endif
	}
	program = g_GLids[program];
	if (program==-1) return;
	Original_GL::glUniformBlockBinding(program, uniformBlockIndex, uniformBlockBinding);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glUniformBlockBinding");
#endif
}

#pragma push_macro("glDrawArraysInstanced")
#undef glDrawArraysInstanced
void glDrawArraysInstanced (GLenum mode, GLint first, GLsizei count, GLsizei instanceCount) {
#pragma pop_macro("glDrawArraysInstanced")
	if( enable_logging) {
		log_cpp(" glDrawArraysInstanced( %d, %d, %d, %d);\n", mode, first, count, instanceCount);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glDrawArraysInstanced\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glDrawArraysInstanced(mode, first, count, instanceCount);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glDrawArraysInstanced");
#endif
}

#pragma push_macro("glDrawElementsInstanced")
#undef glDrawElementsInstanced
void glDrawElementsInstanced (GLenum mode, GLsizei count, GLenum type, const GLvoid* indices, GLsizei instanceCount) {
#pragma pop_macro("glDrawElementsInstanced")
	if( enable_logging) {
		log_cpp(" glDrawElementsInstanced( %d, %d, %d, (void*)0x%x, %d);\n", mode, count, type, indices, instanceCount);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glDrawElementsInstanced\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glDrawElementsInstanced(mode, count, type, indices, instanceCount);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glDrawElementsInstanced");
#endif
}

#pragma push_macro("glGetInteger64v")
#undef glGetInteger64v
void glGetInteger64v (GLenum pname, GLint64* params) {
#pragma pop_macro("glGetInteger64v")
	if( enable_logging) {
		log_cpp(" glGetInteger64v( %d, (void*)0x%x);\n", pname, params);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glGetInteger64v\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glGetInteger64v(pname, params);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glGetInteger64v");
#endif
}

#pragma push_macro("glGetInteger64i_v")
#undef glGetInteger64i_v
void glGetInteger64i_v (GLenum target, GLuint index, GLint64* data) {
#pragma pop_macro("glGetInteger64i_v")
	if( enable_logging) {
		log_cpp(" glGetInteger64i_v( %d, %d, (void*)0x%x);\n", target, index, data);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glGetInteger64i_v\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glGetInteger64i_v(target, index, data);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glGetInteger64i_v");
#endif
}

#pragma push_macro("glGetBufferParameteri64v")
#undef glGetBufferParameteri64v
void glGetBufferParameteri64v (GLenum target, GLenum pname, GLint64* params) {
#pragma pop_macro("glGetBufferParameteri64v")
	if( enable_logging) {
		log_cpp(" glGetBufferParameteri64v( %d, %d, (void*)0x%x);\n", target, pname, params);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glGetBufferParameteri64v\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glGetBufferParameteri64v(target, pname, params);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glGetBufferParameteri64v");
#endif
}

#pragma push_macro("glGenSamplers")
#undef glGenSamplers
void glGenSamplers (GLsizei count, GLuint* samplers) {
#pragma pop_macro("glGenSamplers")
	if( enable_logging) {
		log_cpp(" for( size_t i=0; i<%d; i++)\n{\nGLuint tmp;\nglGenSamplers( 1, &tmp);\ng_GLids.push_back( tmp);\n}\n ", count);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glGenSamplers\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glGenSamplers(count, samplers);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glGenSamplers");
#endif
	for( GLsizei i=0; i<count; i++)
	{
		g_GLids.push_back(samplers[i]); 
		samplers[i] = g_GLids.size() - 1; 
	}
}

#pragma push_macro("glDeleteSamplers")
#undef glDeleteSamplers
void glDeleteSamplers (GLsizei count, const GLuint* samplers) {
#pragma pop_macro("glDeleteSamplers")
	//Original_GL::glDeleteSamplers(count, samplers);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glDeleteSamplers");
#endif
}

#pragma push_macro("glIsSampler")
#undef glIsSampler
GLboolean glIsSampler (GLuint sampler) {
#pragma pop_macro("glIsSampler")
	if( enable_logging) {
		log_cpp(" if ( g_GLids[ %d] != -1) {\nglIsSampler( g_GLids[ %d]);\n}\n", sampler, sampler);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glIsSampler\", __FILE__, __LINE__);\n");
#endif
	}
	sampler = g_GLids[sampler];
	if (sampler==-1) return false;
	GLboolean retval = (GLboolean)Original_GL::glIsSampler(sampler);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glIsSampler");
#endif
	return retval;
}

#pragma push_macro("glBindSampler")
#undef glBindSampler
void glBindSampler (GLuint unit, GLuint sampler) {
#pragma pop_macro("glBindSampler")
	if( enable_logging) {
		log_cpp(" if ( g_GLids[ %d] != -1) {\nglBindSampler( %d, g_GLids[ %d]);\n}\n", sampler, unit, sampler);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glBindSampler\", __FILE__, __LINE__);\n");
#endif
	}
	sampler = g_GLids[sampler];
	if (sampler==-1) return;
	Original_GL::glBindSampler(unit, sampler);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glBindSampler");
#endif
}

#pragma push_macro("glSamplerParameteri")
#undef glSamplerParameteri
void glSamplerParameteri (GLuint sampler, GLenum pname, GLint param) {
#pragma pop_macro("glSamplerParameteri")
	if( enable_logging) {
		log_cpp(" if ( g_GLids[ %d] != -1) {\nglSamplerParameteri( g_GLids[ %d], %d, %d);\n}\n", sampler, sampler, pname, param);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glSamplerParameteri\", __FILE__, __LINE__);\n");
#endif
	}
	sampler = g_GLids[sampler];
	if (sampler==-1) return;
	Original_GL::glSamplerParameteri(sampler, pname, param);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glSamplerParameteri");
#endif
}

#pragma push_macro("glSamplerParameteriv")
#undef glSamplerParameteriv
void glSamplerParameteriv (GLuint sampler, GLenum pname, const GLint* param) {
#pragma pop_macro("glSamplerParameteriv")
	if( enable_logging) {
		log_cpp(" if ( g_GLids[ %d] != -1) {\nglSamplerParameteriv( g_GLids[ %d], %d, (void*)0x%x);\n}\n", sampler, sampler, pname, param);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glSamplerParameteriv\", __FILE__, __LINE__);\n");
#endif
	}
	sampler = g_GLids[sampler];
	if (sampler==-1) return;
	Original_GL::glSamplerParameteriv(sampler, pname, param);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glSamplerParameteriv");
#endif
}

#pragma push_macro("glSamplerParameterf")
#undef glSamplerParameterf
void glSamplerParameterf (GLuint sampler, GLenum pname, GLfloat param) {
#pragma pop_macro("glSamplerParameterf")
	if( enable_logging) {
		log_cpp(" if ( g_GLids[ %d] != -1) {\nglSamplerParameterf( g_GLids[ %d], %d, %#gf);\n}\n", sampler, sampler, pname, float_sane( &param ));
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glSamplerParameterf\", __FILE__, __LINE__);\n");
#endif
	}
	sampler = g_GLids[sampler];
	if (sampler==-1) return;
	Original_GL::glSamplerParameterf(sampler, pname, param);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glSamplerParameterf");
#endif
}

#pragma push_macro("glSamplerParameterfv")
#undef glSamplerParameterfv
void glSamplerParameterfv (GLuint sampler, GLenum pname, const GLfloat* param) {
#pragma pop_macro("glSamplerParameterfv")
	if( enable_logging) {
		log_cpp(" if ( g_GLids[ %d] != -1) {\nglSamplerParameterfv( g_GLids[ %d], %d, (void*)0x%x);\n}\n", sampler, sampler, pname, param);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glSamplerParameterfv\", __FILE__, __LINE__);\n");
#endif
	}
	sampler = g_GLids[sampler];
	if (sampler==-1) return;
	Original_GL::glSamplerParameterfv(sampler, pname, param);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glSamplerParameterfv");
#endif
}

#pragma push_macro("glGetSamplerParameteriv")
#undef glGetSamplerParameteriv
void glGetSamplerParameteriv (GLuint sampler, GLenum pname, GLint* params) {
#pragma pop_macro("glGetSamplerParameteriv")
	if( enable_logging) {
		log_cpp(" if ( g_GLids[ %d] != -1) {\nglGetSamplerParameteriv( g_GLids[ %d], %d, (void*)0x%x);\n}\n", sampler, sampler, pname, params);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glGetSamplerParameteriv\", __FILE__, __LINE__);\n");
#endif
	}
	sampler = g_GLids[sampler];
	if (sampler==-1) return;
	Original_GL::glGetSamplerParameteriv(sampler, pname, params);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glGetSamplerParameteriv");
#endif
}

#pragma push_macro("glGetSamplerParameterfv")
#undef glGetSamplerParameterfv
void glGetSamplerParameterfv (GLuint sampler, GLenum pname, GLfloat* params) {
#pragma pop_macro("glGetSamplerParameterfv")
	if( enable_logging) {
		log_cpp(" if ( g_GLids[ %d] != -1) {\nglGetSamplerParameterfv( g_GLids[ %d], %d, (void*)0x%x);\n}\n", sampler, sampler, pname, params);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glGetSamplerParameterfv\", __FILE__, __LINE__);\n");
#endif
	}
	sampler = g_GLids[sampler];
	if (sampler==-1) return;
	Original_GL::glGetSamplerParameterfv(sampler, pname, params);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glGetSamplerParameterfv");
#endif
}

#pragma push_macro("glVertexAttribDivisor")
#undef glVertexAttribDivisor
void glVertexAttribDivisor (GLuint index, GLuint divisor) {
#pragma pop_macro("glVertexAttribDivisor")
	if( enable_logging) {
		log_cpp(" glVertexAttribDivisor( %d, %d);\n", index, divisor);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glVertexAttribDivisor\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glVertexAttribDivisor(index, divisor);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glVertexAttribDivisor");
#endif
}

#pragma push_macro("glBindTransformFeedback")
#undef glBindTransformFeedback
void glBindTransformFeedback (GLenum target, GLuint id) {
#pragma pop_macro("glBindTransformFeedback")
	if( enable_logging) {
		log_cpp(" if ( g_GLids[ %d] != -1) {\nglBindTransformFeedback( %d, g_GLids[ %d]);\n}\n", id, target, id);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glBindTransformFeedback\", __FILE__, __LINE__);\n");
#endif
	}
	id = g_GLids[id];
	if (id==-1) return;
	Original_GL::glBindTransformFeedback(target, id);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glBindTransformFeedback");
#endif
}

#pragma push_macro("glDeleteTransformFeedbacks")
#undef glDeleteTransformFeedbacks
void glDeleteTransformFeedbacks (GLsizei n, const GLuint* ids) {
#pragma pop_macro("glDeleteTransformFeedbacks")
	if( enable_logging) {
		log_cpp(" glDeleteTransformFeedbacks( %d, (void*)0x%x);\n", n, ids);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glDeleteTransformFeedbacks\", __FILE__, __LINE__);\n");
#endif
	}
	//Original_GL::glDeleteTransformFeedbacks(n, ids);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glDeleteTransformFeedbacks");
#endif
}

#pragma push_macro("glGenTransformFeedbacks")
#undef glGenTransformFeedbacks
void glGenTransformFeedbacks (GLsizei n, GLuint* ids) {
#pragma pop_macro("glGenTransformFeedbacks")
	if( enable_logging) {
		log_cpp(" for( size_t i=0; i<%d; i++)\n{\nGLuint tmp;\nglGenTransformFeedbacks( 1, &tmp);\ng_GLids.push_back( tmp);\n}\n ", n);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glGenTransformFeedbacks\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glGenTransformFeedbacks(n, ids);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glGenTransformFeedbacks");
#endif
	for( GLsizei i=0; i<n; i++)
	{
		g_GLids.push_back(ids[i]); 
		ids[i] = g_GLids.size() - 1; 
	}
}

#pragma push_macro("glIsTransformFeedback")
#undef glIsTransformFeedback
GLboolean glIsTransformFeedback (GLuint id) {
#pragma pop_macro("glIsTransformFeedback")
	if( enable_logging) {
		log_cpp(" if ( g_GLids[ %d] != -1) {\nglIsTransformFeedback( g_GLids[ %d]);\n}\n", id, id);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glIsTransformFeedback\", __FILE__, __LINE__);\n");
#endif
	}
	id = g_GLids[id];
	if (id==-1) return false;
	GLboolean retval = (GLboolean)Original_GL::glIsTransformFeedback(id);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glIsTransformFeedback");
#endif
	return retval;
}

#pragma push_macro("glPauseTransformFeedback")
#undef glPauseTransformFeedback
void glPauseTransformFeedback (void) {
#pragma pop_macro("glPauseTransformFeedback")
	if( enable_logging) {
		log_cpp(" glPauseTransformFeedback();\n");
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glPauseTransformFeedback\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glPauseTransformFeedback();
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glPauseTransformFeedback");
#endif
}

#pragma push_macro("glResumeTransformFeedback")
#undef glResumeTransformFeedback
void glResumeTransformFeedback (void) {
#pragma pop_macro("glResumeTransformFeedback")
	if( enable_logging) {
		log_cpp(" glResumeTransformFeedback();\n");
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glResumeTransformFeedback\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glResumeTransformFeedback();
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glResumeTransformFeedback");
#endif
}

#pragma push_macro("glGetProgramBinary")
#undef glGetProgramBinary
void glGetProgramBinary (GLuint program, GLsizei bufSize, GLsizei* length, GLenum* binaryFormat, GLvoid* binary) {
#pragma pop_macro("glGetProgramBinary")
	if( enable_logging) {
		log_cpp(" if ( g_GLids[ %d] != -1) {\nglGetProgramBinary( g_GLids[ %d], %d, (void*)0x%x, (void*)0x%x, (void*)0x%x);\n}\n", program, program, bufSize, length, binaryFormat, binary);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glGetProgramBinary\", __FILE__, __LINE__);\n");
#endif
	}
	program = g_GLids[program];
	if (program==-1) return;
	Original_GL::glGetProgramBinary(program, bufSize, length, binaryFormat, binary);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glGetProgramBinary");
#endif
}

#pragma push_macro("glProgramBinary")
#undef glProgramBinary
void glProgramBinary (GLuint program, GLenum binaryFormat, const GLvoid* binary, GLsizei length) {
#pragma pop_macro("glProgramBinary")
	if( enable_logging) {
		log_cpp(" if ( g_GLids[ %d] != -1) {\nglProgramBinary( g_GLids[ %d], %d, (void*)0x%x, %d);\n}\n", program, program, binaryFormat, binary, length);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glProgramBinary\", __FILE__, __LINE__);\n");
#endif
	}
	program = g_GLids[program];
	if (program==-1) return;
	Original_GL::glProgramBinary(program, binaryFormat, binary, length);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glProgramBinary");
#endif
}

#pragma push_macro("glProgramParameteri")
#undef glProgramParameteri
void glProgramParameteri (GLuint program, GLenum pname, GLint value) {
#pragma pop_macro("glProgramParameteri")
	if( enable_logging) {
		log_cpp(" if ( g_GLids[ %d] != -1) {\nglProgramParameteri( g_GLids[ %d], %d, %d);\n}\n", program, program, pname, value);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glProgramParameteri\", __FILE__, __LINE__);\n");
#endif
	}
	program = g_GLids[program];
	if (program==-1) return;
	Original_GL::glProgramParameteri(program, pname, value);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glProgramParameteri");
#endif
}

#pragma push_macro("glInvalidateFramebuffer")
#undef glInvalidateFramebuffer
void glInvalidateFramebuffer (GLenum target, GLsizei numAttachments, const GLenum* attachments) {
#pragma pop_macro("glInvalidateFramebuffer")
	if( enable_logging) {
		log_cpp(" glInvalidateFramebuffer( %d, %d, (void*)0x%x);\n", target, numAttachments, attachments);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glInvalidateFramebuffer\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glInvalidateFramebuffer(target, numAttachments, attachments);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glInvalidateFramebuffer");
#endif
}

#pragma push_macro("glInvalidateSubFramebuffer")
#undef glInvalidateSubFramebuffer
void glInvalidateSubFramebuffer (GLenum target, GLsizei numAttachments, const GLenum* attachments, GLint x, GLint y, GLsizei width, GLsizei height) {
#pragma pop_macro("glInvalidateSubFramebuffer")
	if( enable_logging) {
		log_cpp(" glInvalidateSubFramebuffer( %d, %d, (void*)0x%x, %d, %d, %d, %d);\n", target, numAttachments, attachments, x, y, width, height);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glInvalidateSubFramebuffer\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glInvalidateSubFramebuffer(target, numAttachments, attachments, x, y, width, height);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glInvalidateSubFramebuffer");
#endif
}

#pragma push_macro("glTexStorage2D")
#undef glTexStorage2D
void glTexStorage2D (GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height) {
#pragma pop_macro("glTexStorage2D")
	if( enable_logging) {
		log_cpp(" glTexStorage2D( %d, %d, %d, %d, %d);\n", target, levels, internalformat, width, height);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glTexStorage2D\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glTexStorage2D(target, levels, internalformat, width, height);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glTexStorage2D");
#endif
}

#pragma push_macro("glTexStorage3D")
#undef glTexStorage3D
void glTexStorage3D (GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth) {
#pragma pop_macro("glTexStorage3D")
	if( enable_logging) {
		log_cpp(" glTexStorage3D( %d, %d, %d, %d, %d, %d);\n", target, levels, internalformat, width, height, depth);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glTexStorage3D\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glTexStorage3D(target, levels, internalformat, width, height, depth);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glTexStorage3D");
#endif
}

#pragma push_macro("glGetInternalformativ")
#undef glGetInternalformativ
void glGetInternalformativ (GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLint* params) {
#pragma pop_macro("glGetInternalformativ")
	if( enable_logging) {
		log_cpp(" glGetInternalformativ( %d, %d, %d, %d, (void*)0x%x);\n", target, internalformat, pname, bufSize, params);
#if defined FRAMEPLAYER_CHECK_ERRORS
		log_cpp("getGlError(\"glGetInternalformativ\", __FILE__, __LINE__);\n");
#endif
	}
	Original_GL::glGetInternalformativ(target, internalformat, pname, bufSize, params);
#if defined FRAMECAPTURE_CHECK_ERRORS
	getGlError("glGetInternalformativ");
#endif
}

