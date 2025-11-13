#include <GLES3/gl3.h>

#ifdef GL_ES_VERSION_2_0
GL_APICALL void GL_APIENTRY glActiveTexture (GLenum texture) { return; }
GL_APICALL void GL_APIENTRY glAttachShader (GLuint program, GLuint shader) { return; }
GL_APICALL void GL_APIENTRY glBindAttribLocation (GLuint program, GLuint index, const GLchar *name) { return; }
GL_APICALL void GL_APIENTRY glBindBuffer (GLenum target, GLuint buffer) { return; }
GL_APICALL void GL_APIENTRY glBindFramebuffer (GLenum target, GLuint framebuffer) { return; }
GL_APICALL void GL_APIENTRY glBindRenderbuffer (GLenum target, GLuint renderbuffer) { return; }
GL_APICALL void GL_APIENTRY glBindTexture (GLenum target, GLuint texture) { return; }
GL_APICALL void GL_APIENTRY glBlendColor (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) { return; }
GL_APICALL void GL_APIENTRY glBlendEquation (GLenum mode) { return; }
GL_APICALL void GL_APIENTRY glBlendEquationSeparate (GLenum modeRGB, GLenum modeAlpha) { return; }
GL_APICALL void GL_APIENTRY glBlendFunc (GLenum sfactor, GLenum dfactor) { return; }
GL_APICALL void GL_APIENTRY glBlendFuncSeparate (GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha) { return; }
GL_APICALL void GL_APIENTRY glBufferData (GLenum target, GLsizeiptr size, const void *data, GLenum usage) { return; }
GL_APICALL void GL_APIENTRY glBufferSubData (GLenum target, GLintptr offset, GLsizeiptr size, const void *data) { return; }
GL_APICALL GLenum GL_APIENTRY glCheckFramebufferStatus (GLenum target) { return; }
GL_APICALL void GL_APIENTRY glClear (GLbitfield mask) { return; }
GL_APICALL void GL_APIENTRY glClearColor (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) { return; }
GL_APICALL void GL_APIENTRY glClearDepthf (GLfloat d) { return; }
GL_APICALL void GL_APIENTRY glClearStencil (GLint s) { return; }
GL_APICALL void GL_APIENTRY glColorMask (GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha) { return; }
GL_APICALL void GL_APIENTRY glCompileShader (GLuint shader) { return; }
GL_APICALL void GL_APIENTRY glCompressedTexImage2D (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void *data) { return; }
GL_APICALL void GL_APIENTRY glCompressedTexSubImage2D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void *data) { return; }
GL_APICALL void GL_APIENTRY glCopyTexImage2D (GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border) { return; }
GL_APICALL void GL_APIENTRY glCopyTexSubImage2D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height) { return; }
GL_APICALL GLuint GL_APIENTRY glCreateProgram (void) { return; }
GL_APICALL GLuint GL_APIENTRY glCreateShader (GLenum type) { return; }
GL_APICALL void GL_APIENTRY glCullFace (GLenum mode) { return; }
GL_APICALL void GL_APIENTRY glDeleteBuffers (GLsizei n, const GLuint *buffers) { return; }
GL_APICALL void GL_APIENTRY glDeleteFramebuffers (GLsizei n, const GLuint *framebuffers) { return; }
GL_APICALL void GL_APIENTRY glDeleteProgram (GLuint program) { return; }
GL_APICALL void GL_APIENTRY glDeleteRenderbuffers (GLsizei n, const GLuint *renderbuffers) { return; }
GL_APICALL void GL_APIENTRY glDeleteShader (GLuint shader) { return; }
GL_APICALL void GL_APIENTRY glDeleteTextures (GLsizei n, const GLuint *textures) { return; }
GL_APICALL void GL_APIENTRY glDepthFunc (GLenum func) { return; }
GL_APICALL void GL_APIENTRY glDepthMask (GLboolean flag) { return; }
GL_APICALL void GL_APIENTRY glDepthRangef (GLfloat n, GLfloat f) { return; }
GL_APICALL void GL_APIENTRY glDetachShader (GLuint program, GLuint shader) { return; }
GL_APICALL void GL_APIENTRY glDisable (GLenum cap) { return; }
GL_APICALL void GL_APIENTRY glDisableVertexAttribArray (GLuint index) { return; }
GL_APICALL void GL_APIENTRY glDrawArrays (GLenum mode, GLint first, GLsizei count) { return; }
GL_APICALL void GL_APIENTRY glDrawElements (GLenum mode, GLsizei count, GLenum type, const void *indices) { return; }
GL_APICALL void GL_APIENTRY glEnable (GLenum cap) { return; }
GL_APICALL void GL_APIENTRY glEnableVertexAttribArray (GLuint index) { return; }
GL_APICALL void GL_APIENTRY glFinish (void) { return; }
GL_APICALL void GL_APIENTRY glFlush (void) { return; }
GL_APICALL void GL_APIENTRY glFramebufferRenderbuffer (GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer) { return; }
GL_APICALL void GL_APIENTRY glFramebufferTexture2D (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level) { return; }
GL_APICALL void GL_APIENTRY glFrontFace (GLenum mode) { return; }
GL_APICALL void GL_APIENTRY glGenBuffers (GLsizei n, GLuint *buffers) { return; }
GL_APICALL void GL_APIENTRY glGenerateMipmap (GLenum target) { return; }
GL_APICALL void GL_APIENTRY glGenFramebuffers (GLsizei n, GLuint *framebuffers) { return; }
GL_APICALL void GL_APIENTRY glGenRenderbuffers (GLsizei n, GLuint *renderbuffers) { return; }
GL_APICALL void GL_APIENTRY glGenTextures (GLsizei n, GLuint *textures) { return; }
GL_APICALL void GL_APIENTRY glGetActiveAttrib (GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name) { return; }
GL_APICALL void GL_APIENTRY glGetActiveUniform (GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name) { return; }
GL_APICALL void GL_APIENTRY glGetAttachedShaders (GLuint program, GLsizei maxCount, GLsizei *count, GLuint *shaders) { return; }
GL_APICALL GLint GL_APIENTRY glGetAttribLocation (GLuint program, const GLchar *name) { return; }
GL_APICALL void GL_APIENTRY glGetBooleanv (GLenum pname, GLboolean *data) { return; }
GL_APICALL void GL_APIENTRY glGetBufferParameteriv (GLenum target, GLenum pname, GLint *params) { return; }
GL_APICALL GLenum GL_APIENTRY glGetError (void) { return; }
GL_APICALL void GL_APIENTRY glGetFloatv (GLenum pname, GLfloat *data) { return; }
GL_APICALL void GL_APIENTRY glGetFramebufferAttachmentParameteriv (GLenum target, GLenum attachment, GLenum pname, GLint *params) { return; }
GL_APICALL void GL_APIENTRY glGetIntegerv (GLenum pname, GLint *data) { return; }
GL_APICALL void GL_APIENTRY glGetProgramiv (GLuint program, GLenum pname, GLint *params) { return; }
GL_APICALL void GL_APIENTRY glGetProgramInfoLog (GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog) { return; }
GL_APICALL void GL_APIENTRY glGetRenderbufferParameteriv (GLenum target, GLenum pname, GLint *params) { return; }
GL_APICALL void GL_APIENTRY glGetShaderiv (GLuint shader, GLenum pname, GLint *params) { return; }
GL_APICALL void GL_APIENTRY glGetShaderInfoLog (GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog) { return; }
GL_APICALL void GL_APIENTRY glGetShaderPrecisionFormat (GLenum shadertype, GLenum precisiontype, GLint *range, GLint *precision) { return; }
GL_APICALL void GL_APIENTRY glGetShaderSource (GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *source) { return; }
GL_APICALL const GLubyte *GL_APIENTRY glGetString (GLenum name) { return; }
GL_APICALL void GL_APIENTRY glGetTexParameterfv (GLenum target, GLenum pname, GLfloat *params) { return; }
GL_APICALL void GL_APIENTRY glGetTexParameteriv (GLenum target, GLenum pname, GLint *params) { return; }
GL_APICALL void GL_APIENTRY glGetUniformfv (GLuint program, GLint location, GLfloat *params) { return; }
GL_APICALL void GL_APIENTRY glGetUniformiv (GLuint program, GLint location, GLint *params) { return; }
GL_APICALL GLint GL_APIENTRY glGetUniformLocation (GLuint program, const GLchar *name) { return; }
GL_APICALL void GL_APIENTRY glGetVertexAttribfv (GLuint index, GLenum pname, GLfloat *params) { return; }
GL_APICALL void GL_APIENTRY glGetVertexAttribiv (GLuint index, GLenum pname, GLint *params) { return; }
GL_APICALL void GL_APIENTRY glGetVertexAttribPointerv (GLuint index, GLenum pname, void **pointer) { return; }
GL_APICALL void GL_APIENTRY glHint (GLenum target, GLenum mode) { return; }
GL_APICALL GLboolean GL_APIENTRY glIsBuffer (GLuint buffer) { return; }
GL_APICALL GLboolean GL_APIENTRY glIsEnabled (GLenum cap) { return; }
GL_APICALL GLboolean GL_APIENTRY glIsFramebuffer (GLuint framebuffer) { return; }
GL_APICALL GLboolean GL_APIENTRY glIsProgram (GLuint program) { return; }
GL_APICALL GLboolean GL_APIENTRY glIsRenderbuffer (GLuint renderbuffer) { return; }
GL_APICALL GLboolean GL_APIENTRY glIsShader (GLuint shader) { return; }
GL_APICALL GLboolean GL_APIENTRY glIsTexture (GLuint texture) { return; }
GL_APICALL void GL_APIENTRY glLineWidth (GLfloat width) { return; }
GL_APICALL void GL_APIENTRY glLinkProgram (GLuint program) { return; }
GL_APICALL void GL_APIENTRY glPixelStorei (GLenum pname, GLint param) { return; }
GL_APICALL void GL_APIENTRY glPolygonOffset (GLfloat factor, GLfloat units) { return; }
GL_APICALL void GL_APIENTRY glReadPixels (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void *pixels) { return; }
GL_APICALL void GL_APIENTRY glReleaseShaderCompiler (void) { return; }
GL_APICALL void GL_APIENTRY glRenderbufferStorage (GLenum target, GLenum internalformat, GLsizei width, GLsizei height) { return; }
GL_APICALL void GL_APIENTRY glSampleCoverage (GLfloat value, GLboolean invert) { return; }
GL_APICALL void GL_APIENTRY glScissor (GLint x, GLint y, GLsizei width, GLsizei height) { return; }
GL_APICALL void GL_APIENTRY glShaderBinary (GLsizei count, const GLuint *shaders, GLenum binaryformat, const void *binary, GLsizei length) { return; }
GL_APICALL void GL_APIENTRY glShaderSource (GLuint shader, GLsizei count, const GLchar *const*string, const GLint *length) { return; }
GL_APICALL void GL_APIENTRY glStencilFunc (GLenum func, GLint ref, GLuint mask) { return; }
GL_APICALL void GL_APIENTRY glStencilFuncSeparate (GLenum face, GLenum func, GLint ref, GLuint mask) { return; }
GL_APICALL void GL_APIENTRY glStencilMask (GLuint mask) { return; }
GL_APICALL void GL_APIENTRY glStencilMaskSeparate (GLenum face, GLuint mask) { return; }
GL_APICALL void GL_APIENTRY glStencilOp (GLenum fail, GLenum zfail, GLenum zpass) { return; }
GL_APICALL void GL_APIENTRY glStencilOpSeparate (GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass) { return; }
GL_APICALL void GL_APIENTRY glTexImage2D (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels) { return; }
GL_APICALL void GL_APIENTRY glTexParameterf (GLenum target, GLenum pname, GLfloat param) { return; }
GL_APICALL void GL_APIENTRY glTexParameterfv (GLenum target, GLenum pname, const GLfloat *params) { return; }
GL_APICALL void GL_APIENTRY glTexParameteri (GLenum target, GLenum pname, GLint param) { return; }
GL_APICALL void GL_APIENTRY glTexParameteriv (GLenum target, GLenum pname, const GLint *params) { return; }
GL_APICALL void GL_APIENTRY glTexSubImage2D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels) { return; }
GL_APICALL void GL_APIENTRY glUniform1f (GLint location, GLfloat v0) { return; }
GL_APICALL void GL_APIENTRY glUniform1fv (GLint location, GLsizei count, const GLfloat *value) { return; }
GL_APICALL void GL_APIENTRY glUniform1i (GLint location, GLint v0) { return; }
GL_APICALL void GL_APIENTRY glUniform1iv (GLint location, GLsizei count, const GLint *value) { return; }
GL_APICALL void GL_APIENTRY glUniform2f (GLint location, GLfloat v0, GLfloat v1) { return; }
GL_APICALL void GL_APIENTRY glUniform2fv (GLint location, GLsizei count, const GLfloat *value) { return; }
GL_APICALL void GL_APIENTRY glUniform2i (GLint location, GLint v0, GLint v1) { return; }
GL_APICALL void GL_APIENTRY glUniform2iv (GLint location, GLsizei count, const GLint *value) { return; }
GL_APICALL void GL_APIENTRY glUniform3f (GLint location, GLfloat v0, GLfloat v1, GLfloat v2) { return; }
GL_APICALL void GL_APIENTRY glUniform3fv (GLint location, GLsizei count, const GLfloat *value) { return; }
GL_APICALL void GL_APIENTRY glUniform3i (GLint location, GLint v0, GLint v1, GLint v2) { return; }
GL_APICALL void GL_APIENTRY glUniform3iv (GLint location, GLsizei count, const GLint *value) { return; }
GL_APICALL void GL_APIENTRY glUniform4f (GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3) { return; }
GL_APICALL void GL_APIENTRY glUniform4fv (GLint location, GLsizei count, const GLfloat *value) { return; }
GL_APICALL void GL_APIENTRY glUniform4i (GLint location, GLint v0, GLint v1, GLint v2, GLint v3) { return; }
GL_APICALL void GL_APIENTRY glUniform4iv (GLint location, GLsizei count, const GLint *value) { return; }
GL_APICALL void GL_APIENTRY glUniformMatrix2fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) { return; }
GL_APICALL void GL_APIENTRY glUniformMatrix3fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) { return; }
GL_APICALL void GL_APIENTRY glUniformMatrix4fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) { return; }
GL_APICALL void GL_APIENTRY glUseProgram (GLuint program) { return; }
GL_APICALL void GL_APIENTRY glValidateProgram (GLuint program) { return; }
GL_APICALL void GL_APIENTRY glVertexAttrib1f (GLuint index, GLfloat x) { return; }
GL_APICALL void GL_APIENTRY glVertexAttrib1fv (GLuint index, const GLfloat *v) { return; }
GL_APICALL void GL_APIENTRY glVertexAttrib2f (GLuint index, GLfloat x, GLfloat y) { return; }
GL_APICALL void GL_APIENTRY glVertexAttrib2fv (GLuint index, const GLfloat *v) { return; }
GL_APICALL void GL_APIENTRY glVertexAttrib3f (GLuint index, GLfloat x, GLfloat y, GLfloat z) { return; }
GL_APICALL void GL_APIENTRY glVertexAttrib3fv (GLuint index, const GLfloat *v) { return; }
GL_APICALL void GL_APIENTRY glVertexAttrib4f (GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w) { return; }
GL_APICALL void GL_APIENTRY glVertexAttrib4fv (GLuint index, const GLfloat *v) { return; }
GL_APICALL void GL_APIENTRY glVertexAttribPointer (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer) { return; }
GL_APICALL void GL_APIENTRY glViewport (GLint x, GLint y, GLsizei width, GLsizei height) { return; }
#endif /* GL_ES_VERSION_2_0 */

#ifdef GL_ES_VERSION_3_0
GL_APICALL void GL_APIENTRY glReadBuffer (GLenum src) { return; }
GL_APICALL void GL_APIENTRY glDrawRangeElements (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void *indices) { return; }
GL_APICALL void GL_APIENTRY glTexImage3D (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void *pixels) { return; }
GL_APICALL void GL_APIENTRY glTexSubImage3D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void *pixels) { return; }
GL_APICALL void GL_APIENTRY glCopyTexSubImage3D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height) { return; }
GL_APICALL void GL_APIENTRY glCompressedTexImage3D (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const void *data) { return; }
GL_APICALL void GL_APIENTRY glCompressedTexSubImage3D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void *data) { return; }
GL_APICALL void GL_APIENTRY glGenQueries (GLsizei n, GLuint *ids) { return; }
GL_APICALL void GL_APIENTRY glDeleteQueries (GLsizei n, const GLuint *ids) { return; }
GL_APICALL GLboolean GL_APIENTRY glIsQuery (GLuint id) { return; }
GL_APICALL void GL_APIENTRY glBeginQuery (GLenum target, GLuint id) { return; }
GL_APICALL void GL_APIENTRY glEndQuery (GLenum target) { return; }
GL_APICALL void GL_APIENTRY glGetQueryiv (GLenum target, GLenum pname, GLint *params) { return; }
GL_APICALL void GL_APIENTRY glGetQueryObjectuiv (GLuint id, GLenum pname, GLuint *params) { return; }
GL_APICALL GLboolean GL_APIENTRY glUnmapBuffer (GLenum target) { return; }
GL_APICALL void GL_APIENTRY glGetBufferPointerv (GLenum target, GLenum pname, void **params) { return; }
GL_APICALL void GL_APIENTRY glDrawBuffers (GLsizei n, const GLenum *bufs) { return; }
GL_APICALL void GL_APIENTRY glUniformMatrix2x3fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) { return; }
GL_APICALL void GL_APIENTRY glUniformMatrix3x2fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) { return; }
GL_APICALL void GL_APIENTRY glUniformMatrix2x4fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) { return; }
GL_APICALL void GL_APIENTRY glUniformMatrix4x2fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) { return; }
GL_APICALL void GL_APIENTRY glUniformMatrix3x4fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) { return; }
GL_APICALL void GL_APIENTRY glUniformMatrix4x3fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) { return; }
GL_APICALL void GL_APIENTRY glBlitFramebuffer (GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter) { return; }
GL_APICALL void GL_APIENTRY glRenderbufferStorageMultisample (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height) { return; }
GL_APICALL void GL_APIENTRY glFramebufferTextureLayer (GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer) { return; }
GL_APICALL void *GL_APIENTRY glMapBufferRange (GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access) { return; }
GL_APICALL void GL_APIENTRY glFlushMappedBufferRange (GLenum target, GLintptr offset, GLsizeiptr length) { return; }
GL_APICALL void GL_APIENTRY glBindVertexArray (GLuint array) { return; }
GL_APICALL void GL_APIENTRY glDeleteVertexArrays (GLsizei n, const GLuint *arrays) { return; }
GL_APICALL void GL_APIENTRY glGenVertexArrays (GLsizei n, GLuint *arrays) { return; }
GL_APICALL GLboolean GL_APIENTRY glIsVertexArray (GLuint array) { return; }
GL_APICALL void GL_APIENTRY glGetIntegeri_v (GLenum target, GLuint index, GLint *data) { return; }
GL_APICALL void GL_APIENTRY glBeginTransformFeedback (GLenum primitiveMode) { return; }
GL_APICALL void GL_APIENTRY glEndTransformFeedback (void) { return; }
GL_APICALL void GL_APIENTRY glBindBufferRange (GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size) { return; }
GL_APICALL void GL_APIENTRY glBindBufferBase (GLenum target, GLuint index, GLuint buffer) { return; }
GL_APICALL void GL_APIENTRY glTransformFeedbackVaryings (GLuint program, GLsizei count, const GLchar *const*varyings, GLenum bufferMode) { return; }
GL_APICALL void GL_APIENTRY glGetTransformFeedbackVarying (GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLsizei *size, GLenum *type, GLchar *name) { return; }
GL_APICALL void GL_APIENTRY glVertexAttribIPointer (GLuint index, GLint size, GLenum type, GLsizei stride, const void *pointer) { return; }
GL_APICALL void GL_APIENTRY glGetVertexAttribIiv (GLuint index, GLenum pname, GLint *params) { return; }
GL_APICALL void GL_APIENTRY glGetVertexAttribIuiv (GLuint index, GLenum pname, GLuint *params) { return; }
GL_APICALL void GL_APIENTRY glVertexAttribI4i (GLuint index, GLint x, GLint y, GLint z, GLint w) { return; }
GL_APICALL void GL_APIENTRY glVertexAttribI4ui (GLuint index, GLuint x, GLuint y, GLuint z, GLuint w) { return; }
GL_APICALL void GL_APIENTRY glVertexAttribI4iv (GLuint index, const GLint *v) { return; }
GL_APICALL void GL_APIENTRY glVertexAttribI4uiv (GLuint index, const GLuint *v) { return; }
GL_APICALL void GL_APIENTRY glGetUniformuiv (GLuint program, GLint location, GLuint *params) { return; }
GL_APICALL GLint GL_APIENTRY glGetFragDataLocation (GLuint program, const GLchar *name) { return; }
GL_APICALL void GL_APIENTRY glUniform1ui (GLint location, GLuint v0) { return; }
GL_APICALL void GL_APIENTRY glUniform2ui (GLint location, GLuint v0, GLuint v1) { return; }
GL_APICALL void GL_APIENTRY glUniform3ui (GLint location, GLuint v0, GLuint v1, GLuint v2) { return; }
GL_APICALL void GL_APIENTRY glUniform4ui (GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3) { return; }
GL_APICALL void GL_APIENTRY glUniform1uiv (GLint location, GLsizei count, const GLuint *value) { return; }
GL_APICALL void GL_APIENTRY glUniform2uiv (GLint location, GLsizei count, const GLuint *value) { return; }
GL_APICALL void GL_APIENTRY glUniform3uiv (GLint location, GLsizei count, const GLuint *value) { return; }
GL_APICALL void GL_APIENTRY glUniform4uiv (GLint location, GLsizei count, const GLuint *value) { return; }
GL_APICALL void GL_APIENTRY glClearBufferiv (GLenum buffer, GLint drawbuffer, const GLint *value) { return; }
GL_APICALL void GL_APIENTRY glClearBufferuiv (GLenum buffer, GLint drawbuffer, const GLuint *value) { return; }
GL_APICALL void GL_APIENTRY glClearBufferfv (GLenum buffer, GLint drawbuffer, const GLfloat *value) { return; }
GL_APICALL void GL_APIENTRY glClearBufferfi (GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil) { return; }
GL_APICALL const GLubyte *GL_APIENTRY glGetStringi (GLenum name, GLuint index) { return; }
GL_APICALL void GL_APIENTRY glCopyBufferSubData (GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size) { return; }
GL_APICALL void GL_APIENTRY glGetUniformIndices (GLuint program, GLsizei uniformCount, const GLchar *const*uniformNames, GLuint *uniformIndices) { return; }
GL_APICALL void GL_APIENTRY glGetActiveUniformsiv (GLuint program, GLsizei uniformCount, const GLuint *uniformIndices, GLenum pname, GLint *params) { return; }
GL_APICALL GLuint GL_APIENTRY glGetUniformBlockIndex (GLuint program, const GLchar *uniformBlockName) { return; }
GL_APICALL void GL_APIENTRY glGetActiveUniformBlockiv (GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint *params) { return; }
GL_APICALL void GL_APIENTRY glGetActiveUniformBlockName (GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei *length, GLchar *uniformBlockName) { return; }
GL_APICALL void GL_APIENTRY glUniformBlockBinding (GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding) { return; }
GL_APICALL void GL_APIENTRY glDrawArraysInstanced (GLenum mode, GLint first, GLsizei count, GLsizei instancecount) { return; }
GL_APICALL void GL_APIENTRY glDrawElementsInstanced (GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount) { return; }
GL_APICALL GLsync GL_APIENTRY glFenceSync (GLenum condition, GLbitfield flags) { return; }
GL_APICALL GLboolean GL_APIENTRY glIsSync (GLsync sync) { return; }
GL_APICALL void GL_APIENTRY glDeleteSync (GLsync sync) { return; }
GL_APICALL GLenum GL_APIENTRY glClientWaitSync (GLsync sync, GLbitfield flags, GLuint64 timeout) { return; }
GL_APICALL void GL_APIENTRY glWaitSync (GLsync sync, GLbitfield flags, GLuint64 timeout) { return; }
GL_APICALL void GL_APIENTRY glGetInteger64v (GLenum pname, GLint64 *data) { return; }
GL_APICALL void GL_APIENTRY glGetSynciv (GLsync sync, GLenum pname, GLsizei bufSize, GLsizei *length, GLint *values) { return; }
GL_APICALL void GL_APIENTRY glGetInteger64i_v (GLenum target, GLuint index, GLint64 *data) { return; }
GL_APICALL void GL_APIENTRY glGetBufferParameteri64v (GLenum target, GLenum pname, GLint64 *params) { return; }
GL_APICALL void GL_APIENTRY glGenSamplers (GLsizei count, GLuint *samplers) { return; }
GL_APICALL void GL_APIENTRY glDeleteSamplers (GLsizei count, const GLuint *samplers) { return; }
GL_APICALL GLboolean GL_APIENTRY glIsSampler (GLuint sampler) { return; }
GL_APICALL void GL_APIENTRY glBindSampler (GLuint unit, GLuint sampler) { return; }
GL_APICALL void GL_APIENTRY glSamplerParameteri (GLuint sampler, GLenum pname, GLint param) { return; }
GL_APICALL void GL_APIENTRY glSamplerParameteriv (GLuint sampler, GLenum pname, const GLint *param) { return; }
GL_APICALL void GL_APIENTRY glSamplerParameterf (GLuint sampler, GLenum pname, GLfloat param) { return; }
GL_APICALL void GL_APIENTRY glSamplerParameterfv (GLuint sampler, GLenum pname, const GLfloat *param) { return; }
GL_APICALL void GL_APIENTRY glGetSamplerParameteriv (GLuint sampler, GLenum pname, GLint *params) { return; }
GL_APICALL void GL_APIENTRY glGetSamplerParameterfv (GLuint sampler, GLenum pname, GLfloat *params) { return; }
GL_APICALL void GL_APIENTRY glVertexAttribDivisor (GLuint index, GLuint divisor) { return; }
GL_APICALL void GL_APIENTRY glBindTransformFeedback (GLenum target, GLuint id) { return; }
GL_APICALL void GL_APIENTRY glDeleteTransformFeedbacks (GLsizei n, const GLuint *ids) { return; }
GL_APICALL void GL_APIENTRY glGenTransformFeedbacks (GLsizei n, GLuint *ids) { return; }
GL_APICALL GLboolean GL_APIENTRY glIsTransformFeedback (GLuint id) { return; }
GL_APICALL void GL_APIENTRY glPauseTransformFeedback (void) { return; }
GL_APICALL void GL_APIENTRY glResumeTransformFeedback (void) { return; }
GL_APICALL void GL_APIENTRY glGetProgramBinary (GLuint program, GLsizei bufSize, GLsizei *length, GLenum *binaryFormat, void *binary) { return; }
GL_APICALL void GL_APIENTRY glProgramBinary (GLuint program, GLenum binaryFormat, const void *binary, GLsizei length) { return; }
GL_APICALL void GL_APIENTRY glProgramParameteri (GLuint program, GLenum pname, GLint value) { return; }
GL_APICALL void GL_APIENTRY glInvalidateFramebuffer (GLenum target, GLsizei numAttachments, const GLenum *attachments) { return; }
GL_APICALL void GL_APIENTRY glInvalidateSubFramebuffer (GLenum target, GLsizei numAttachments, const GLenum *attachments, GLint x, GLint y, GLsizei width, GLsizei height) { return; }
GL_APICALL void GL_APIENTRY glTexStorage2D (GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height) { return; }
GL_APICALL void GL_APIENTRY glTexStorage3D (GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth) { return; }
GL_APICALL void GL_APIENTRY glGetInternalformativ (GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLint *params) { return; }
#endif /* GL_ES_VERSION_3_0 */

